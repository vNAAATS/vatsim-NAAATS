import os, sys, tempfile, unittest
from datetime import datetime, timezone
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import rcl_auto as r
import commands as cm
import vnaaats_server as s


class FakeNet:
    def __init__(self):
        self.pos, self.pilots, self.offline, self.flights = {}, {}, {}, []
    def is_offline(self, cs): return self.offline.get(cs)
    def position(self, cs): return self.pos.get(cs)
    def lookup_pilot(self, cs): return self.pilots.get(cs)
    def all(self): return list(self.flights)


class FakeNat:
    def __init__(self): self.d = {}
    def by_callsign(self): return self.d


class Base(unittest.TestCase):
    def setUp(self):
        self.store = s.FlightStore(os.path.join(tempfile.mkdtemp(), "f.db"))
        self.q = cm.CommandQueue()
        self.net, self.nat = FakeNet(), FakeNat()
        self.t = [datetime(2026, 10, 23, 12, 0, tzinfo=timezone.utc).timestamp()]
        self.dist = [None]
        self.rcl = r.RclAuto(self.store, self.q, self.net, self.nat, lambda route, pos: self.dist[0], ("CZQO", "EGGX"), now=lambda: self.t[0], start=False)
        self.rcl.confirm = False                                    # these tests are about the automatic path; the confirm path has its own tests below
        self.rec = {"callsign": "BAW1", "route": "A B", "routeEtas": "1230 1300", "trackedBy": ""}
        self.net.flights = [self.rec]
        self.net.pos["BAW1"] = (51.0, -8.0)
        self.net.pilots["BAW1"] = {"groundspeed": 450}

    def poll(self, me="CZQO_CTR", settled=True):
        self.rcl.note_poll(me)
        if settled and self.rcl.sender_since is None and self.rcl.sender():
            self.rcl.sender_since = self.t[0] - r.START_DELAY_SECONDS - 1     # pretend the session has already settled


class RclTests(Base):
    def test_entry_minutes(self):
        self.assertEqual(r.entry_minutes_from_now("1230 1300", 12 * 60), 30)
        self.assertEqual(r.entry_minutes_from_now("0010 0100", 23 * 60 + 50), 20)    # across midnight
        self.assertEqual(r.entry_minutes_from_now("1150 1300", 12 * 60), -10)
        self.assertIsNone(r.entry_minutes_from_now("-- --", 0))

    def test_due_rules(self):
        self.assertTrue(self.rcl.due(self.rec))                         # 30 min to entry
        self.rec["routeEtas"] = "1330 1400"
        self.assertFalse(self.rcl.due(self.rec))                        # 90 min away, no distance known
        self.dist[0] = 150
        self.assertTrue(self.rcl.due(self.rec))                         # but within 200 nm
        self.dist[0] = 250
        self.assertFalse(self.rcl.due(self.rec))
        self.rec["routeEtas"] = "1140 1300"
        self.dist[0] = 100
        self.assertFalse(self.rcl.due(self.rec))                        # entry passed 20 min ago
        self.rec["routeEtas"] = "1158 1300"
        self.assertFalse(self.rcl.due(self.rec))                        # entry passed 2 min ago: already in oceanic
        self.rec["routeEtas"] = "1215 1300"
        self.assertFalse(self.rcl.due(self.rec))                        # 15 min to entry: natTrak would refuse the request
        self.rec["routeEtas"] = "1220 1300"
        self.assertTrue(self.rcl.due(self.rec))                         # exactly 20 min: still allowed
        self.rec["routeEtas"] = "-- --"
        self.dist[0] = 100
        self.net.pilots["BAW1"] = {"groundspeed": 450}
        self.assertFalse(self.rcl.due(self.rec))                        # no times: 100 nm at 450 kt is 13 min, too late
        self.dist[0] = 190
        self.assertTrue(self.rcl.due(self.rec))                         # 190 nm at 450 kt is 25 min
        self.rec["routeEtas"] = "1230 1300"
        self.rec["routeEtas"] = "1230 1300"
        self.net.pilots["BAW1"] = {"groundspeed": 5}
        self.assertFalse(self.rcl.due(self.rec))                        # parked
        self.net.pilots["BAW1"] = {"groundspeed": 450}
        self.nat.d["BAW1"] = {"status": "PENDING"}
        self.assertFalse(self.rcl.due(self.rec))                        # already on natTrak
        del self.nat.d["BAW1"]
        self.rec["trackedBy"] = "CZQO_CTR"
        self.assertFalse(self.rcl.due(self.rec))                        # an oceanic controller has it
        self.rec["trackedBy"] = ""
        self.rec["ntSent"] = {"by": "x"}
        self.assertFalse(self.rcl.due(self.rec))                        # already reminded by hand
        self.rec.pop("ntSent")
        self.net.offline["BAW1"] = True
        self.assertFalse(self.rcl.due(self.rec))

    def test_needs_an_oceanic_sender(self):
        self.assertIsNone(self.rcl.tick())                              # nobody online
        self.poll("EGLL_TWR")
        self.assertIsNone(self.rcl.tick())                              # not an oceanic position
        self.poll("CZQO_CTR")
        self.assertEqual(self.rcl.tick(), "BAW1")
        cmds = self.q.pending_for("CZQO_CTR")
        self.assertEqual(cmds[0]["type"], "natmsg")
        self.assertEqual(cmds[0]["by"], "AUTO")
        self.assertTrue(cmds[0]["args"]["text"].startswith(".msg BAW1 Hello"))

    def test_sent_once_and_marked(self):
        self.poll()
        self.assertEqual(self.rcl.tick(), "BAW1")
        self.assertIsNone(self.rcl.tick())                              # already in flight
        c = self.q.done(1, True, "ok")
        self.rcl.done(c, True)
        self.assertTrue(self.rcl.already_sent("BAW1"))
        self.assertTrue(self.store.get("BAW1")["ntSent"])
        self.t[0] += 3600
        self.poll()
        self.assertIsNone(self.rcl.tick())                              # never twice

    def test_failure_retries_then_gives_up(self):
        self.poll()
        for n in range(r.MAX_TRIES):
            self.assertEqual(self.rcl.tick(), "BAW1")
            c = self.q.done(n + 1, False, "autotype is off")
            self.rcl.done(c, False, "autotype is off")
            self.assertIsNone(self.rcl.tick())                          # waits before trying again
            self.t[0] += r.RETRY_SECONDS + 1
            self.poll()
        self.assertIsNone(self.rcl.tick())
        self.assertFalse(self.rcl.already_sent("BAW1"))

    def test_switch_off(self):
        self.poll()
        self.rcl.set_enabled(False)
        self.assertIsNone(self.rcl.tick())
        self.rcl.set_enabled(True)
        self.assertEqual(self.rcl.tick(), "BAW1")

    def test_inside_or_through_the_oca_is_never_due(self):
        self.assertTrue(self.rcl.due(self.rec))
        self.net.pos["BAW1"] = (55.0, -30.0)                            # mid-Atlantic
        self.assertFalse(self.rcl.due(self.rec))
        self.dist[0] = 50
        self.rec["routeEtas"] = "-- --"
        self.assertFalse(self.rcl.due(self.rec))                        # no times and close to a route fix, still inside
        self.net.pos["BAW1"] = (51.0, -5.0)                             # east of Shanwick
        self.rec["direction"] = True                                    # eastbound = through it
        self.assertFalse(self.rcl.due(self.rec))
        self.rec["direction"] = False                                   # westbound = about to enter
        self.dist[0] = 150
        self.assertTrue(self.rcl.due(self.rec))
        self.net.pos["BAW1"] = (48.0, -60.0)                            # west of Gander
        self.assertFalse(self.rcl.due(self.rec))                        # westbound and past the area
        self.rec["direction"] = True
        self.assertTrue(self.rcl.due(self.rec))                         # eastbound and not yet in it

    def test_waits_five_minutes_after_the_session_starts(self):
        self.poll(settled=False)
        self.assertIsNone(self.rcl.tick())                              # just came online
        self.t[0] += r.START_DELAY_SECONDS - 5
        self.poll(settled=False)
        self.assertIsNone(self.rcl.tick())
        self.t[0] += 10
        self.poll(settled=False)
        self.assertEqual(self.rcl.tick(), "BAW1")

    def test_baseline_skips_flights_already_in_the_oca(self):
        inside = {"callsign": "DLH2", "route": "A B", "routeEtas": "-- --", "trackedBy": ""}
        late = {"callsign": "AAL3", "route": "A B", "routeEtas": "1210 1300", "trackedBy": ""}
        self.net.flights = [inside, late, self.rec]
        for cs, pos in (("DLH2", (55.0, -35.0)), ("AAL3", (51.0, -9.0))):
            self.net.pos[cs] = pos
            self.net.pilots[cs] = {"groundspeed": 450}
        self.dist[0] = 100
        self.poll(settled=False)
        self.assertIsNone(self.rcl.tick())
        self.t[0] += r.START_DELAY_SECONDS + 1
        self.poll(settled=False)
        self.assertEqual(self.rcl.tick(), "BAW1")                       # only the flight that still has time
        self.assertEqual(self.rcl.baseline, {"DLH2", "AAL3"})
        self.net.pos["DLH2"] = (51.0, -9.0)                             # even if it later looks eligible it stays handled
        self.assertFalse(self.rcl.due(inside))

    def test_new_session_after_a_long_gap(self):
        self.poll(settled=False)
        self.rcl.tick()
        self.t[0] += r.START_DELAY_SECONDS + 1
        self.poll(settled=False)
        self.rcl.tick()
        self.t[0] += r.SENDER_GAP_RESET + 100                           # nobody online for a long while
        self.assertIsNone(self.rcl.tick())
        self.assertIsNone(self.rcl.sender_since)
        self.poll(settled=False)
        self.assertIsNone(self.rcl.tick())                              # the five minutes start again


if __name__ == "__main__":
    unittest.main()


class ConfirmTests(Base):
    """Same fixtures, with the controller asked first."""
    def setUp(self):
        Base.setUp(self)
        self.rcl.confirm = True

    def test_nothing_is_typed_until_released(self):
        self.poll()
        self.assertIsNone(self.rcl.tick())
        self.assertEqual(self.q.pending_for("CZQO_CTR"), [])
        st = self.rcl.status()
        self.assertEqual([w["callsign"] for w in st["waiting"]], ["BAW1"])
        self.assertEqual(st["waiting"][0]["minutes"], 30)
        self.assertEqual(st["waiting"][0]["fix"], "A")
        self.assertEqual(st["waiting"][0]["time"], "1230")

    def test_prompt_is_paced_and_needs_a_sender(self):
        self.assertFalse(self.rcl.status()["prompt"])                   # nobody online
        self.poll()
        self.rcl.tick()
        self.assertTrue(self.rcl.status()["prompt"])
        self.rcl.later()
        self.assertFalse(self.rcl.status()["prompt"])                   # asked again in 5 minutes
        self.t[0] += r.PROMPT_EVERY_SECONDS + 1
        self.poll()
        self.rcl.tick()
        self.assertTrue(self.rcl.status()["prompt"])

    def test_first_prompt_waits_for_the_start_delay(self):
        self.poll(settled=False)                                        # session just started
        self.rcl.tick()
        self.assertFalse(self.rcl.status()["prompt"])
        self.assertEqual(self.rcl.status()["waiting"], [])

    def test_send_now_types_released_flights_one_at_a_time(self):
        self.rec2 = {"callsign": "DLH2", "route": "A B", "routeEtas": "1235 1300", "trackedBy": ""}
        self.net.flights.append(self.rec2)
        self.net.pos["DLH2"] = (51.0, -9.0)
        self.net.pilots["DLH2"] = {"groundspeed": 450}
        self.poll()
        self.rcl.tick()
        self.rcl.send_now()
        self.assertFalse(self.rcl.status()["prompt"])
        self.assertEqual(self.rcl.status()["batch"]["total"], 2)
        self.assertEqual(self.rcl.tick(), "BAW1")
        self.assertIsNone(self.rcl.tick())                              # one in flight at a time per tick... the second waits for the next tick
        # the plugin reports the first as done
        c = self.q.done(1, True, "ok")
        self.rcl.done(c, True)
        self.assertEqual(self.rcl.tick(), "DLH2")
        c = self.q.done(2, True, "ok")
        self.rcl.done(c, True)
        b = self.rcl.status()["batch"]
        self.assertEqual((b["total"], b["done"], b["failed"]), (2, 2, []))

    def test_skip_never_reminds(self):
        self.poll()
        self.rcl.tick()
        self.rcl.skip()
        self.assertEqual(self.rcl.status()["waiting"], [])
        self.t[0] += 3600
        self.poll()
        self.assertIsNone(self.rcl.tick())
        self.assertEqual(self.rcl.status()["waiting"], [])

    def test_flight_that_stops_being_due_is_dropped_before_typing(self):
        self.poll()
        self.rcl.tick()
        self.rcl.send_now()
        self.rec["routeEtas"] = "1215 1300"                             # now only 15 minutes to entry: natTrak would refuse
        self.assertIsNone(self.rcl.tick())
        self.assertEqual(self.q.pending_for("CZQO_CTR"), [])

    def test_failure_is_listed_in_the_batch(self):
        self.poll()
        self.rcl.tick()
        self.rcl.send_now()
        self.assertEqual(self.rcl.tick(), "BAW1")
        c = self.q.done(1, False, "interrupted")
        self.rcl.done(c, False, "interrupted")
        self.assertEqual(self.rcl.status()["batch"]["failed"], ["BAW1"])

    def test_confirm_setting_is_remembered(self):
        self.rcl.set_confirm(False)
        again = r.RclAuto(self.store, self.q, self.net, self.nat, lambda route, pos: None, ("CZQO",), now=lambda: self.t[0], start=False)
        self.assertFalse(again.confirm)
        self.rcl.set_confirm(True)
        again = r.RclAuto(self.store, self.q, self.net, self.nat, lambda route, pos: None, ("CZQO",), now=lambda: self.t[0], start=False)
        self.assertTrue(again.confirm)

    def test_entry_info(self):
        self.assertEqual(r.entry_info("A B C", "1230 1300 1330", 12 * 60), (30, "A", "1230"))
        self.assertEqual(r.entry_info("A B C", "-- 1300 1330", 12 * 60), (60, "B", "1300"))
        self.assertEqual(r.entry_info("A B", "-- --", 0), (None, "", ""))


class StuckTests(Base):
    """The 'Sending reminder' box must never stay up once nothing is being typed."""
    def setUp(self):
        Base.setUp(self)
        self.rcl.confirm = True

    def release(self):
        self.poll()
        self.rcl.tick()
        self.rcl.send_now()
        self.assertGreater(self.rcl.status()["sending"], 0)

    def test_switching_off_clears_sending(self):
        self.release()
        self.rcl.set_enabled(False)
        st = self.rcl.status()
        self.assertEqual((st["sending"], st["inflight"], st["batch"]["total"]), (0, 0, 0))

    def test_sender_leaving_clears_sending(self):
        self.release()
        self.t[0] += r.POLLER_FRESH_SECONDS + 5                         # plugin stopped polling
        self.assertEqual(self.rcl.status()["sending"], 0)

    def test_stalled_batch_is_abandoned(self):
        self.release()
        self.t[0] += r.STALL_SECONDS + 1
        self.poll()
        self.assertEqual(self.rcl.status()["sending"], 0)

    def test_unanswered_command_expires_and_is_marked_failed(self):
        self.release()
        self.poll()
        self.assertEqual(self.rcl.tick(), "BAW1")
        self.assertEqual(self.rcl.status()["inflight"], 1)
        self.t[0] += r.INFLIGHT_SECONDS + 1
        self.poll()
        st = self.rcl.status()
        self.assertEqual((st["inflight"], st["sending"]), (0, 0))
        self.assertEqual(st["batch"]["failed"], ["BAW1"])

    def test_stop_cancels_an_uncollected_command(self):
        self.release()
        self.poll()
        self.rcl.tick()
        self.rcl.stop()
        self.assertEqual(self.q.pending_for("CZQO_CTR"), [])           # never handed to the plugin
        self.assertEqual(self.rcl.status()["sending"], 0)

