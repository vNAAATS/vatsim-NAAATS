"""
Automatic RCL REQ reminder.

A flight that is about to enter oceanic airspace and has not asked for clearance on natTrak is sent the standard
"please request oceanic clearance" message.  The message is typed into EuroScope by a controller's plugin (the same
"natmsg" command the FDD's RCL REQ > Send button uses), so one oceanic controller who is online with the plugin
(autotype=1) is picked as the sender.

A flight is due when ALL of these hold:
  * it is on VATSIM, moving (not parked), and not already handled by natTrak or tracked by an oceanic controller
  * no reminder has been sent to it yet (by hand or automatically)
  * and either its oceanic entry estimate is between MIN_LEAD_MINUTES and DUE_MINUTES away,
    or (no usable times) it is within DUE_NM of a fix on its route
  and it is NOT already inside, or through, oceanic airspace, and NOT closer than MIN_LEAD_MINUTES to its entry point: natTrak
  refuses a new request that late, and a flight past its entry point does not need one.

By default (confirm on) nothing is typed until the sending controller agrees.  Flights that are due wait in a list; at most once
every PROMPT_EVERY_SECONDS the FDD asks that controller "send the reminders now?" (they must keep EuroScope in front while they go),
and only the flights they release are typed, one at a time.  With confirm off the reminders go out by themselves as before.
"""
import logging
import re
import threading
import time
from datetime import datetime, timezone

import commands as commands_mod

log = logging.getLogger("vnaaats")

DUE_MINUTES = 45
MIN_LEAD_MINUTES = 20          # natTrak will not accept a new request closer than this to the oceanic entry point
DUE_NM = 200
START_DELAY_SECONDS = 300      # nothing is sent until an oceanic controller has been online this long (session start settling time)
SENDER_GAP_RESET = 600         # no oceanic controller online for this long = a new session: the wait and the baseline start again
OCA_WEST_LON = -53.0           # Gander OCA western edge / Shanwick OCA eastern edge, rounded outwards for a safe "inside" test
OCA_EAST_LON = -15.0
OCA_MIN_LAT = 43.0
OCA_MAX_LAT = 72.0
MIN_AIRBORNE_ALT = 10000       # feet: below this the flight is on the ground, taking off or still climbing out
MIN_GROUNDSPEED = 100          # knots: a parked or taxiing aircraft is not due
TICK_SECONDS = 10
POLLER_FRESH_SECONDS = 15      # a plugin that asked for commands this recently is "online"
RETRY_SECONDS = 300            # after a reminder was typed and the plugin said it worked, no second one is tried for this long
RETRY_FAIL_SECONDS = 60        # after a failure (EuroScope not in front, no answer) the next try comes this soon
MAX_TRIES = 5
TELEX_WAIT_SECONDS = 180       # a telex was sent: if the pilot has not asked for clearance by then, the EuroScope message goes as well
TELEX_REPING_SECONDS = 120     # a flight that was not online on Hoppie is asked about again after this long
TELEX_PER_TICK = 5
INFLIGHT_SECONDS = 90          # the command is given this long to come back before we may queue it again
PROMPT_EVERY_SECONDS = 300    # the FDD asks about the waiting flights at most this often
STALL_SECONDS = 60            # released flights with nothing in flight and no progress for this long: the batch is abandoned
BY = "AUTO"
TELEX_BY = "Auto RCL telex"      # the red star set by a telex; the EuroScope message may still follow it


def entry_minutes_from_now(route_etas, now_utc_minute):
    """Minutes until the earliest estimate on the route (negative if it has passed). None if there are no times."""
    times = [t for t in str(route_etas or "").split() if re.fullmatch(r"\d{4}", t)]
    if not times:
        return None
    mins, prev, day = [], None, 0
    for t in times:                                        # unwrap midnight so the times form one run
        m = int(t[:2]) * 60 + int(t[2:])
        if prev is not None and m + day * 1440 < prev - 720:
            day += 1
        elif prev is not None and m + day * 1440 > prev + 720:
            day -= 1
        prev = m + day * 1440
        mins.append(prev)
    entry = min(mins) % 1440
    d = entry - now_utc_minute
    if d > 720:
        d -= 1440
    elif d < -720:
        d += 1440
    return d


def entry_info(route, etas, now_utc_minute):
    """(minutes until the oceanic entry estimate, the fix it is for, the time as HHMM) from a route and its fix times; Nones if no times."""
    fixes = str(route or "").split()
    toks = str(etas or "").split()
    best = None
    for i, t in enumerate(toks):
        if re.fullmatch(r"\d{4}", t):
            m = entry_minutes_from_now(t, now_utc_minute)
            if m is not None and (best is None or m < best[0]) and i < len(fixes):
                best = (m, fixes[i], t)
    if best is None:
        m = entry_minutes_from_now(etas, now_utc_minute)
        return (m, "", "") if m is not None else (None, "", "")
    return best


def oca_state(pos, eastbound):
    """'inside' (in the oceanic area between 53W and 15W), 'past' (through it, direction known) or 'before'. Unknown position = 'before'."""
    try:
        lat, lon = float(pos[0]), float(pos[1])
    except (TypeError, ValueError, IndexError):
        return "before"
    if lat < OCA_MIN_LAT or lat > OCA_MAX_LAT:
        return "before"
    if OCA_WEST_LON <= lon <= OCA_EAST_LON:
        return "inside"
    if eastbound is True and lon > OCA_EAST_LON:
        return "past"
    if eastbound is False and lon < OCA_WEST_LON:
        return "past"
    return "before"


class RclAuto:
    def __init__(self, store, queue, network, natrak, distance_fn, oceanic_prefixes, now=time.time, start=True, hoppie=None):
        self.store, self.queue, self.network, self.natrak = store, queue, network, natrak
        self.hoppie = hoppie       # (sender position) -> a signed-in CPDLC station that can ping Hoppie and send a telex, or None
        self.telex_sent = {}       # flight callsign -> time the telex went out
        self.answered_logged = set()   # flights whose "answered" line is already in the history
        self.noreply_logged = set()
        self.telex_off = {}        # flight callsign -> time it was found not online on Hoppie
        self.distance_fn = distance_fn          # (route, (lat, lon)) -> nm to the nearest fix on the route, or None
        self.prefixes = tuple(oceanic_prefixes)
        self.now = now
        self.lock = threading.Lock()
        self.pollers = {}          # position callsign -> time its plugin last asked for commands
        self.inflight = {}         # flight callsign -> {"cid", "ts", "base", "sender"}
        self.tries = {}            # flight callsign -> {"n": attempts, "next": earliest next try}
        self.enabled = True
        self.last_error = ""
        self.sender_since = None   # when an oceanic controller first came online in this session
        self.sender_last = 0.0     # last time one was seen online
        self.baselined = False
        self.baseline = set()      # flights already in/at the oceanic area when the session settled: never reminded
        self.confirm = True        # ask the sending controller before typing (switchable on the display)
        self.pending = {}          # callsign -> {"callsign", "minutes", "fix", "time"}: due, waiting for the controller
        self.approved = set()      # released by the controller, typed one per tick
        self.skipped = set()       # the controller chose not to remind these (this session)
        self.next_prompt = 0.0     # the FDD may ask again from this time
        self.batch = {"total": 0, "done": 0, "failed": []}
        self.activity = 0.0        # last time a batch started, a reminder was queued, or the plugin reported back
        with self.store.lock:
            self.store.db.execute("CREATE TABLE IF NOT EXISTS rcl_confirm (id INTEGER PRIMARY KEY CHECK (id = 1), v INTEGER NOT NULL)")
            crow = self.store.db.execute("SELECT v FROM rcl_confirm WHERE id=1").fetchone()
            self.confirm = True if crow is None else bool(crow[0])
            self.store.db.commit()
        with self.store.lock:
            self.store.db.execute("CREATE TABLE IF NOT EXISTS rcl_auto (callsign TEXT PRIMARY KEY, ts REAL NOT NULL)")
            self.store.db.execute("CREATE TABLE IF NOT EXISTS rcl_telex (callsign TEXT PRIMARY KEY, ts REAL NOT NULL)")
            self.store.db.execute("CREATE TABLE IF NOT EXISTS rcl_history (id INTEGER PRIMARY KEY AUTOINCREMENT, ts REAL NOT NULL, callsign TEXT NOT NULL, kind TEXT NOT NULL, detail TEXT NOT NULL DEFAULT '')")
            self.store.db.execute("CREATE TABLE IF NOT EXISTS rcl_setting (id INTEGER PRIMARY KEY CHECK (id = 1), on_ INTEGER NOT NULL)")
            row = self.store.db.execute("SELECT on_ FROM rcl_setting WHERE id=1").fetchone()
            self.store.db.commit()
        self.enabled = True if row is None else bool(row[0])
        if start:
            threading.Thread(target=self.loop, daemon=True).start()

    # --- history (what happened to which aircraft, newest last; shown in the Auto RCL panel)
    def hist(self, cs, kind, detail=""):
        """kind: telex, telex-failed, answered, no-reply, message, sent, failed."""
        try:
            with self.store.lock:
                self.store.db.execute("INSERT INTO rcl_history (ts, callsign, kind, detail) VALUES (?, ?, ?, ?)", (self.now(), cs, kind, str(detail)[:160]))
                self.store.db.commit()
        except Exception:
            log.exception("could not record the auto RCL history")

    def history(self, limit=60):
        with self.store.lock:
            rows = self.store.db.execute("SELECT ts, callsign, kind, detail FROM rcl_history ORDER BY id DESC LIMIT ?", (limit,)).fetchall()
        return [{"ts": r[0], "callsign": r[1], "kind": r[2], "detail": r[3]} for r in rows]

    # --- settings
    def set_enabled(self, on):
        self.enabled = bool(on)
        if not on:
            self.stop()
        with self.store.lock:
            self.store.db.execute("INSERT OR REPLACE INTO rcl_setting VALUES (1, ?)", (1 if on else 0,))
            self.store.db.commit()

    def set_confirm(self, on):
        self.confirm = bool(on)
        with self.store.lock:
            self.store.db.execute("INSERT OR REPLACE INTO rcl_confirm VALUES (1, ?)", (1 if on else 0,))
            self.store.db.commit()

    # --- the controller's answer to "send the reminders now?"
    def send_now(self):
        with self.lock:
            self.approved |= set(self.pending)
            self.batch = {"total": len(self.approved), "done": 0, "failed": []}
            self.pending = {}
        self.activity = self.now()
        self.next_prompt = self.now() + PROMPT_EVERY_SECONDS
        log.info("Auto RCL: controller released %d reminders", self.batch["total"])

    def stop(self):
        """Abandon whatever is being sent: nothing more is typed, and a command the plugin has not collected is cancelled."""
        with self.lock:
            cids = [v["cid"] for v in self.inflight.values()]
            self.inflight, self.approved, self.pending = {}, set(), {}
            self.batch = {"total": 0, "done": 0, "failed": []}
        for cid in cids:
            try:
                self.queue.done(cid, False, "cancelled")
            except Exception:
                log.exception("could not cancel RCL command %s", cid)
        self.next_prompt = self.now() + PROMPT_EVERY_SECONDS
        log.info("Auto RCL: sending stopped (%d in flight cancelled)", len(cids))

    def housekeeping(self):
        """Runs every tick and before every status, whatever the settings: nothing may stay 'sending' for ever."""
        t = self.now()
        no_sender = self.sender() is None                   # (takes the lock itself)
        with self.lock:
            for cs in [c for c, v in self.inflight.items() if t - v["ts"] > INFLIGHT_SECONDS]:
                del self.inflight[cs]                       # never came back; it may be tried again later
                self.approved.discard(cs)
                self.batch["failed"].append(cs)
                self.last_error = "%s: no answer from EuroScope" % cs
                if cs in self.tries:
                    self.tries[cs]["next"] = t + RETRY_FAIL_SECONDS
            gone = (not self.enabled) or (not self.confirm) or no_sender
            if self.approved and not self.inflight and (gone or t - self.activity > STALL_SECONDS):
                self.approved = set()                       # released flights that will never be typed (sender left, switched off, stalled)
            if not self.approved and not self.inflight and self.batch["total"] and t - self.activity > 600:
                self.batch = {"total": 0, "done": 0, "failed": []}

    def later(self):
        self.next_prompt = self.now() + PROMPT_EVERY_SECONDS

    def skip(self):
        with self.lock:
            self.skipped |= set(self.pending)
            self.pending = {}
        self.next_prompt = self.now() + PROMPT_EVERY_SECONDS

    # --- who can send
    def note_poll(self, me):
        me = (me or "").strip().upper()
        if me:
            with self.lock:
                self.pollers[me] = self.now()

    def sender(self):
        """The oceanic controller whose plugin will type the message, or None."""
        t = self.now()
        with self.lock:
            live = sorted(c for c, ts in self.pollers.items() if t - ts <= POLLER_FRESH_SECONDS and c.startswith(self.prefixes))
        return live[0] if live else None

    def status(self):
        self.housekeeping()
        with self.lock:
            n = len(self.inflight)
            waiting = sorted(self.pending.values(), key=lambda w: (w["minutes"] is None, w["minutes"] if w["minutes"] is not None else 0, w["callsign"]))
            batch = dict(self.batch)
            sending = len(self.approved) + n
        wait = 0
        if self.sender_since is not None:
            wait = max(0, int(self.sender_since + START_DELAY_SECONDS - self.now()))
        sender = self.sender() or ""
        prompt = bool(self.enabled and self.confirm and waiting and sender and self.now() >= self.next_prompt)
        return {"on": self.enabled, "sender": sender, "inflight": n, "error": self.last_error, "startsInSeconds": wait,
                "confirm": self.confirm, "waiting": waiting, "prompt": prompt, "sending": sending, "batch": batch,
                "telex": len(self.telex_sent), "telexOn": bool(self.hoppie), "history": self.history()}

    # --- the decision
    def already_sent(self, cs):
        with self.store.lock:
            return self.store.db.execute("SELECT 1 FROM rcl_auto WHERE callsign=?", (cs,)).fetchone() is not None

    def due(self, rec, now_dt=None):
        """True when this flight should get the reminder now."""
        cs = str(rec.get("callsign", "")).upper()
        sent = rec.get("ntSent")
        if not cs or (sent and (not isinstance(sent, dict) or sent.get("by") != TELEX_BY)):
            return False                                    # reminded already (a telex alone does not count: telex_stage decides about that)
        if self.natrak and cs in self.natrak.by_callsign():
            return False
        by = str(rec.get("trackedBy", "")).upper()
        if by and by.startswith(self.prefixes + ("NATTRAK",)):
            return False
        if self.network:
            if self.network.is_offline(cs):
                return False
            pos = self.network.position(cs)
            pilot = self.network.lookup_pilot(cs) or {}
        else:
            return False
        if not pos:
            return False                                    # not on the network feed: we cannot tell where it is
        gs = pilot.get("groundspeed")
        if isinstance(gs, (int, float)) and gs < MIN_GROUNDSPEED:
            return False
        alt = pilot.get("altitude")
        if isinstance(alt, (int, float)) and 0 < alt < MIN_AIRBORNE_ALT:
            return False                                    # still on the ground (the take-off run is faster than MIN_GROUNDSPEED) or just lifting off
        if cs in self.baseline:
            return False
        if oca_state(pos, rec.get("direction") if isinstance(rec.get("direction"), bool) else None) != "before":
            return False                                    # already inside, or through, the oceanic area: nothing to request
        now_dt = now_dt or datetime.fromtimestamp(self.now(), timezone.utc)
        m = entry_minutes_from_now(rec.get("routeEtas", ""), now_dt.hour * 60 + now_dt.minute)
        if m is not None:
            if m < MIN_LEAD_MINUTES:
                return False        # natTrak takes no new request inside 20 minutes of entry (and one already past entry, or through, needs none)
            if m <= DUE_MINUTES:
                return True
        d = self.distance_fn(rec.get("route", ""), pos)
        if d is None or d > DUE_NM:
            return False
        if m is None and isinstance(gs, (int, float)) and gs > 0 and d / gs * 60 < MIN_LEAD_MINUTES:
            return False            # no times on the flight: judge the 20 minutes from the distance and speed instead
        return True

    # --- telex first: no EuroScope typing at all when the pilot is on the Hoppie network
    def telex_time(self, cs):
        t = self.telex_sent.get(cs)
        if t is None:
            with self.store.lock:
                row = self.store.db.execute("SELECT ts FROM rcl_telex WHERE callsign=?", (cs,)).fetchone()
            if row:
                t = self.telex_sent[cs] = row[0]
        return t

    def telex_stage(self, sender, due_list, t):
        """Takes the flights that are due and returns the ones that still need the EuroScope message. A flight that is online on Hoppie
        gets the reminder as a telex (no logon needed) and is left alone for TELEX_WAIT_SECONDS; if it has not asked for clearance by
        then it comes back here and is typed as before."""
        try:
            st = self.hoppie(sender)
        except Exception:
            log.exception("auto RCL: could not get a CPDLC station")
            st = None
        if not st:
            return due_list
        rest, fresh = [], []
        for item in due_list:
            cs = item[0]
            sent = self.telex_time(cs)
            if sent is not None:
                try:
                    if st.asked_rcl(cs):
                        if cs not in self.answered_logged:
                            self.answered_logged.add(cs)
                            self.hist(cs, "answered", "Pilot asked for clearance after the telex")
                        continue                                    # the pilot answered by CPDLC or telex: nothing more to do
                except Exception:
                    pass
                if t - sent < TELEX_WAIT_SECONDS:
                    continue                                        # give the telex time to be read
                if cs not in self.noreply_logged:
                    self.noreply_logged.add(cs)
                    self.hist(cs, "no-reply", "No reply to the telex; EuroScope message follows")
                rest.append(item)                                   # no answer: the EuroScope message goes too
            elif t - self.telex_off.get(cs, -1e9) < TELEX_REPING_SECONDS:
                rest.append(item)
            else:
                fresh.append(item)
        if fresh:
            try:
                online = st.ping_online([i[0] for i in fresh])
            except Exception:
                log.exception("auto RCL: Hoppie ping failed")
                online = None
            if online is None:
                return rest + fresh                                 # Hoppie could not be asked: EuroScope as before
            sent_now = 0
            for item in fresh:
                cs = item[0]
                if cs in online and sent_now < TELEX_PER_TICK:
                    try:
                        st.send_telex(cs, commands_mod.NATMSG, by=BY)
                    except Exception as e:
                        log.warning("Auto RCL telex to %s failed: %s", cs, e)
                        self.hist(cs, "telex-failed", e)
                        rest.append(item)
                        continue
                    sent_now += 1
                    self.telex_sent[cs] = t
                    self.hist(cs, "telex", "Reminder sent as a Hoppie telex")
                    with self.store.lock:
                        self.store.db.execute("INSERT OR REPLACE INTO rcl_telex VALUES (?, ?)", (cs, t))
                        self.store.db.commit()
                    try:
                        self.store.mark_nt(cs, TELEX_BY, True, item[1])             # the red star on the strip
                    except Exception:
                        log.exception("could not mark the telex on %s", cs)
                    log.info("Auto RCL REQ telex sent to %s (online on Hoppie); EuroScope message follows after %d s if no request", cs, TELEX_WAIT_SECONDS)
                elif cs in online:
                    continue                                        # more than TELEX_PER_TICK this tick: the rest next time
                else:
                    self.telex_off[cs] = t                          # not on Hoppie: the EuroScope message goes now
                    rest.append(item)
        return rest

    # --- the loop
    def candidates(self):
        seen, out = set(), []
        for rec in self.store.all_flights() + (self.network.all() if self.network else []):
            cs = str(rec.get("callsign", "")).upper()
            if cs and cs not in seen:
                seen.add(cs)
                out.append(rec)
        return out

    def take_baseline(self):
        """Flights already in the oceanic area, or inside the 20-minute no-new-request window, when the wait ends are marked
        as handled for this session so they are never reminded."""
        now_dt = datetime.fromtimestamp(self.now(), timezone.utc)
        for rec in self.candidates():
            cs = str(rec.get("callsign", "")).upper()
            pos = self.network.position(cs) if self.network and cs else None
            if not pos:
                continue
            direction = rec.get("direction") if isinstance(rec.get("direction"), bool) else None
            m = entry_minutes_from_now(rec.get("routeEtas", ""), now_dt.hour * 60 + now_dt.minute)
            if oca_state(pos, direction) != "before" or (m is not None and m < MIN_LEAD_MINUTES):
                self.baseline.add(cs)
        self.baselined = True
        log.info("Auto RCL baseline taken: %d flights already at/in the oceanic area will not be reminded", len(self.baseline))

    def tick(self):
        """Queue at most one reminder. Returns the callsign queued, or None."""
        self.housekeeping()
        if not self.enabled:
            return None
        sender = self.sender()
        t = self.now()
        if not sender:
            if self.sender_since is not None and t - self.sender_last > SENDER_GAP_RESET:
                self.sender_since, self.baselined = None, False      # the session ended: start fresh next time
            return None
        self.sender_last = t
        if self.sender_since is None:
            self.sender_since, self.baselined, self.baseline = t, False, set()
        if t - self.sender_since < START_DELAY_SECONDS:
            return None                                              # let the session settle first
        if not self.baselined:
            self.take_baseline()
        hidden = self.store.dismissed_set()
        now_min = datetime.fromtimestamp(t, timezone.utc)
        now_min = now_min.hour * 60 + now_min.minute
        due_list = []
        for rec in self.candidates():
            cs = str(rec.get("callsign", "")).upper()
            if cs in hidden or cs in self.skipped or not re.fullmatch(r"[A-Z0-9]{2,8}", cs):
                continue
            with self.lock:
                if cs in self.inflight:
                    continue
                tr = self.tries.get(cs, {"n": 0, "next": 0})
                if tr["n"] >= MAX_TRIES or t < tr["next"]:
                    continue
            if self.already_sent(cs) or not self.due(rec):
                continue
            due_list.append((cs, rec, tr))
        if self.hoppie and due_list:
            due_list = self.telex_stage(sender, due_list, t)
        if self.confirm:
            mins = {}
            for cs, rec, _ in due_list:
                mins[cs] = entry_info(rec.get("route", ""), rec.get("routeEtas", ""), now_min)
            with self.lock:
                due_cs = {cs for cs, _, _ in due_list}
                self.approved &= due_cs | set(self.inflight)            # one that stopped being due is no longer sent
                self.pending = {cs: {"callsign": cs, "minutes": mins[cs][0], "fix": mins[cs][1], "time": mins[cs][2]}
                                for cs, _, _ in due_list if cs not in self.approved}
            if self.sender_since is not None and self.next_prompt < self.sender_since + START_DELAY_SECONDS:
                self.next_prompt = self.sender_since + START_DELAY_SECONDS
            with self.lock:
                busy = bool(self.inflight)                                  # the next one is typed only after the plugin reports the last
            to_send = [] if busy else [(cs, rec, tr) for cs, rec, tr in due_list if cs in self.approved]
        else:
            with self.lock:
                self.pending, self.approved = {}, set()
            to_send = due_list
        for cs, rec, tr in to_send:
            typ, _, args, err = commands_mod.clean_command({"type": "natmsg", "callsign": cs})
            if err:
                continue
            cid = self.queue.add(sender, typ, cs, args, by=BY)
            self.activity = t
            with self.lock:
                self.inflight[cs] = {"cid": cid, "ts": t, "base": rec, "sender": sender}
                self.tries[cs] = {"n": tr["n"] + 1, "next": t + RETRY_SECONDS}
            self.hist(cs, "message", "EuroScope message queued on %s" % sender)
            log.info("Auto RCL REQ queued for %s via %s", cs, sender)
            return cs
        return None

    def done(self, cmd, ok, msg=""):
        """Called when the plugin reports back on one of our commands (cmd is the finished queue item)."""
        cs = str(cmd.get("callsign", "")).upper()
        self.activity = self.now()
        with self.lock:
            info = self.inflight.pop(cs, None)
            self.approved.discard(cs)
            if info:
                if ok:
                    self.batch["done"] += 1
                else:
                    self.batch["failed"].append(cs)
        if not info:
            return
        if ok:
            with self.store.lock:
                self.store.db.execute("INSERT OR REPLACE INTO rcl_auto VALUES (?, ?)", (cs, self.now()))
                self.store.db.commit()
            self.store.mark_nt(cs, "Auto RCL REQ", True, info.get("base"))
            self.last_error = ""
            self.hist(cs, "sent", "RCL REQ message typed in EuroScope")
            log.info("Auto RCL REQ sent to %s", cs)
        else:
            self.last_error = "%s: %s" % (cs, (msg or "failed")[:120])
            with self.lock:
                if cs in self.tries:
                    self.tries[cs]["next"] = self.now() + RETRY_FAIL_SECONDS      # try again soon, not in five minutes
            self.hist(cs, "failed", (msg or "failed")[:120])
            log.warning("Auto RCL REQ for %s failed: %s", cs, msg)

    def purge(self):
        with self.store.lock:
            self.store.db.execute("DELETE FROM rcl_auto WHERE ts < ?", (self.now() - 3 * 86400,))
            self.store.db.execute("DELETE FROM rcl_history WHERE ts < ?", (self.now() - 2 * 86400,))
            self.store.db.commit()

    def loop(self):
        n = 0
        while True:
            try:
                self.tick()
                n += 1
                if n % 360 == 0:
                    self.purge()
            except Exception:
                log.exception("auto RCL check failed")
            time.sleep(TICK_SECONDS)
