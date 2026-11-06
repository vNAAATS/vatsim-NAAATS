#pragma once
#include "pch.h"
#include "RadarDisplay.h"
#include <set>
#include "AcTargets.h"
#include "InboundList.h"
#include "Constants.h"
#include "DataHandler.h"
#include "Utils.h"
#include "ConflictDetection.h"
#include "DataHandler.h"
#include "RoutesHelper.h"
#include <thread>
#include <gdiplus.h>
#include <ctype.h>
#include <iostream>
#include <functional>
#include <json.hpp>

using namespace Gdiplus;
using namespace std;
using namespace EuroScopePlugIn;

CRadarDisplay::CRadarDisplay()
{
	//COverlays::ShowHideGridReference(this, false);
	inboundList = new CInboundList({ CUtils::InboundX, CUtils::InboundY });
	otherList = new COtherList({ CUtils::OthersX, CUtils::OthersY });
	//rclList = new CRCLList({ CUtils::RCLX, CUtils::RCLY }); // TODO: settings save
	conflictList = new CConflictList({ CUtils::ConflictX, CUtils::ConflictY }); // TODO: settings save
	trackWindow = new CTrackInfoWindow({ CUtils::TrackWindowX, CUtils::TrackWindowY });
	fltPlnWindow = new CFlightPlanWindow({ 1000, 300 }); // TODO: settings save
	msgWindow = new CMessageWindow({ 500, 500 }); // TODO: settings save
	npWindow = new CNotePad({ 300, 300 }, { 800, 200 }); // TODO: save settings
	menuBar = new CMenuBar();
	asel = GetPlugIn()->FlightPlanSelectASEL().GetCallsign();
	fiveSecondTimer = clock();
	tenSecondTimer = clock();
	thirtySecondTimer = clock();

	// Clogger
	CLogger::Log(CLogType::NORM, "Finished initialisation.", "CRadarDisplay");
}

CRadarDisplay::~CRadarDisplay()
{
	// Clean up
	appCursor->isESClosed;
	delete appCursor;
	delete trackWindow;
	delete fltPlnWindow;
	delete trackWindow;
	delete msgWindow;
	delete npWindow;
	delete inboundList;
	delete otherList;
	delete this;
}

void CRadarDisplay::PopulateProgramData() {
	// Lists
	inboundList->MoveList({ CUtils::InboundX, CUtils::InboundY });
	otherList->MoveList({ CUtils::OthersX, CUtils::OthersY });

	CLogger::Log(CLogType::NORM, "Step: lists moved", "CRadarDisplay::PopulateProgramData");
	// Dropdown values
	menuBar->SetDropDownValue(CMenuBar::DRP_AREASEL, CUtils::AreaSelection);
	menuBar->SetDropDownValue(CMenuBar::DRP_OVERLAYS, CUtils::SelectedOverlay);
	menuBar->SetDropDownValue(CMenuBar::DRP_TYPESEL, CUtils::PosType);

	CLogger::Log(CLogType::NORM, "Step: dropdown values set", "CRadarDisplay::PopulateProgramData");
	// Buttons
	if (CUtils::TagsEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_TAGS)) {
		menuBar->SetButtonState(CMenuBar::BTN_TAGS, CInputState::ACTIVE);
	}
	if (CUtils::GridEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_GRID)) {
		menuBar->SetButtonState(CMenuBar::BTN_GRID, CInputState::ACTIVE);
	}
	if (CUtils::OverlayEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_OVERLAYS)) {
		menuBar->SetButtonState(CMenuBar::BTN_OVERLAYS, CInputState::ACTIVE);
	}
	if (CUtils::QckLookEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_QCKLOOK)) {
		menuBar->SetButtonState(CMenuBar::BTN_QCKLOOK, CInputState::ACTIVE);
	}

	CLogger::Log(CLogType::NORM, "Step: buttons set", "CRadarDisplay::PopulateProgramData");
	// Parse track ID
	if (CUtils::SelectedOverlay == DRP_OVL_ALL) {
		COverlays::CurrentType = COverlayType::TCKS_ALL;
	}
	else if (CUtils::SelectedOverlay == DRP_OVL_EAST) {
		COverlays::CurrentType = COverlayType::TCKS_EAST;
	}
	else if (CUtils::SelectedOverlay == DRP_OVL_WEST) {
		COverlays::CurrentType = COverlayType::TCKS_WEST;
	}
	else {
		COverlays::CurrentType = COverlayType::TCKS_SEL;
	}

	CLogger::Log(CLogType::NORM, "Step: overlay chosen", "CRadarDisplay::PopulateProgramData");
	// Load server address / key from vnaaats.cfg
	CDataHandler::LoadSettings();

	CLogger::Log(CLogType::NORM, "Step: cfg read", "CRadarDisplay::PopulateProgramData");
	// Download the tracks
	CDataHandler::PopulateLatestTrackData(GetPlugIn());

	CLogger::Log(CLogType::NORM, "Step: tracks downloaded", "CRadarDisplay::PopulateProgramData");
	// Set tracks in menu bar
	menuBar->MakeDropDownItems(menuBar->DRP_TCKCTRL);

	CLogger::Log(CLogType::NORM, "Step: track list built", "CRadarDisplay::PopulateProgramData");
	// Initialise fonts
	FontSelector::InitialiseFonts();

	CLogger::Log(CLogType::NORM, "Step: fonts ready", "CRadarDisplay::PopulateProgramData");
	// Start cursor update loop
	appCursor->screen = this;
	_beginthread(CursorStateUpdater, 0, (void*)appCursor);
	CLogger::Log(CLogType::NORM, "Firing cursor update sequence thread.", "CRadarDisplay::PopulateProgramData");

	CLogger::Log(CLogType::NORM, "Step: cursor thread started", "CRadarDisplay::PopulateProgramData");
	// Check plugin version
	CDataHandler::CheckPluginVersion(GetPlugIn());
	CLogger::Log(CLogType::NORM, "Start-up finished.", "CRadarDisplay::PopulateProgramData");
}

// On radar screen refresh (modified to occur 4 times a second)
// Find EuroScope's main window (the visible top-level window of this process)
static BOOL CALLBACK FindEsWindow(HWND hwnd, LPARAM lParam) {
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == GetCurrentProcessId() && IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == NULL && GetWindowTextLengthA(hwnd) > 0) {
		*(HWND*)lParam = hwnd;
		return FALSE;
	}
	return TRUE;
}

// Bring EuroScope to the front. Windows often refuses this for a background program, so it is tried up to four times, each time a little
// harder: an Alt tap, then joining the input queue of the window that is in front (which lets SetForegroundWindow through).
static bool BringEsToFront(HWND es) {
	for (int attempt = 0; attempt < 4; attempt++) {
		DWORD pid = 0;
		HWND fg = GetForegroundWindow();
		DWORD fgThread = fg ? GetWindowThreadProcessId(fg, &pid) : 0;
		if (pid == GetCurrentProcessId()) return true;
		if (IsIconic(es)) ShowWindow(es, SW_RESTORE);
		DWORD me = GetCurrentThreadId();
		bool attached = fgThread && fgThread != me && AttachThreadInput(me, fgThread, TRUE) != FALSE;
		keybd_event(VK_MENU, 0, 0, 0);
		keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
		BringWindowToTop(es);
		SetForegroundWindow(es);
		if (attached) AttachThreadInput(me, fgThread, FALSE);
		Sleep(150 + attempt * 100);
		fg = GetForegroundWindow();
		pid = 0;
		if (fg) GetWindowThreadProcessId(fg, &pid);
		if (pid == GetCurrentProcessId()) return true;
	}
	return false;
}

// Experimental (backgroundtype=1 in vnaaats.cfg): type the line by posting key messages straight to EuroScope's window, so EuroScope does not
// have to be in front and nothing is interrupted if you switch away. Windows delivers these whichever program is in front.
// There is no way to see whether EuroScope ran the line, so this reports success once the keys have been posted.
static bool TypeIntoEuroScopeBackground(const string& text, string& why) {
	HWND es = NULL;
	EnumWindows(FindEsWindow, (LPARAM)&es);
	if (!es) { why = "EuroScope's window was not found"; return false; }
	DWORD tid = GetWindowThreadProcessId(es, NULL);
	// the window inside EuroScope that has the keyboard focus (it changes after Home focuses the command line)
	auto target = [&]() -> HWND {
		GUITHREADINFO gi = {};
		gi.cbSize = sizeof(gi);
		if (GetGUIThreadInfo(tid, &gi) && gi.hwndFocus) return gi.hwndFocus;
		return es;
	};
	auto key = [&](WORD vk) {
		HWND t = target();
		LPARAM scan = (LPARAM)MapVirtualKey(vk, MAPVK_VK_TO_VSC);
		PostMessage(t, WM_KEYDOWN, vk, 1 | (scan << 16));
		PostMessage(t, WM_KEYUP, vk, 1 | (scan << 16) | (1 << 30) | ((LPARAM)1 << 31));
	};
	key(VK_HOME);                                  // Home focuses the command line
	Sleep(300);
	key(VK_END);                                   // then empty it
	for (int n = 0; n < 300; n++) key(VK_BACK);
	Sleep(400);
	HWND t = target();
	for (unsigned char ch : text) {
		PostMessage(t, WM_CHAR, (WPARAM)ch, 1);
		Sleep(ch == ' ' ? 40 : 8);
	}
	Sleep(300);
	key(VK_RETURN);
	return true;
}

// Type a line into EuroScope's command line: Home (focuses it), the text, Enter.
// EuroScope is brought to the front first (the controller has just clicked in the browser). Fails if Windows will not allow that.
static bool TypeIntoEuroScope(const string& text, string& why) {
	HWND es = NULL;
	EnumWindows(FindEsWindow, (LPARAM)&es);
	if (!es) return false;
	if (!BringEsToFront(es)) { why = "Windows would not let the plugin bring EuroScope to the front - click on EuroScope and try again, or use Copy and paste"; return false; }
	// True while EuroScope is still the window in front. Typing goes to whichever window is in front, so this is checked before every
	// key press: if the controller clicks into another program (Discord, a browser...) the typing stops at once instead of carrying on there.
	auto esInFront = [&]() -> bool {
		HWND f = GetForegroundWindow();
		DWORD p2 = 0;
		if (f) GetWindowThreadProcessId(f, &p2);
		return p2 == GetCurrentProcessId();
	};
	const string interrupted = "interrupted - you switched away from EuroScope, so nothing more was typed and the message was NOT sent";
	// This runs on its own thread with short pauses, so EuroScope's own thread can finish each step (move focus to the
	// command line, clear it, take the text, run the line) before the next keys arrive. Sending everything in one burst
	// left old text in the box and glued commands together.
	auto send = [&](vector<INPUT>& in) { if (!in.empty()) SendInput((UINT)in.size(), in.data(), sizeof(INPUT)); in.clear(); };
	vector<INPUT> in;
	auto key = [&](WORD vk) {
		INPUT d = {}; d.type = INPUT_KEYBOARD; d.ki.wVk = vk; in.push_back(d);
		INPUT u = d; u.ki.dwFlags = KEYEVENTF_KEYUP; in.push_back(u);
	};
	if (!esInFront()) { why = interrupted; return false; }
	key(VK_HOME); send(in);                       // Home focuses the command line
	Sleep(300);
	if (!esInFront()) { why = interrupted; return false; }
	key(VK_END);                                  // then empty it: End and plenty of Backspaces, whatever was left in it
	for (int n = 0; n < 300; n++) key(VK_BACK);
	send(in);
	Sleep(300);
	if (!esInFront()) { why = interrupted; return false; }
	// One character at a time with a short pause: EuroScope shows a dot-command hint while ".msg" is being typed and could
	// swallow the space that follows it (".msgBAW123 Hello"). Spaces are sent as a real Space key press.
	for (unsigned char ch : text) {
		if (!esInFront()) {                       // switched away part-way: stop (the next message clears the command line first)
			why = interrupted;
			return false;
		}
		if (ch == ' ') {
			key(VK_SPACE);
		}
		else {
			INPUT d = {}; d.type = INPUT_KEYBOARD; d.ki.wScan = ch; d.ki.dwFlags = KEYEVENTF_UNICODE; in.push_back(d);
			INPUT u = d; u.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP; in.push_back(u);
		}
		send(in);
		Sleep(ch == ' ' ? 60 : 10);
	}
	Sleep(300);
	if (!esInFront()) { why = interrupted; return false; }
	key(VK_RETURN); send(in);
	return true;
}

static HWND g_radarWnd = NULL;            // the radar window the cursor was last seen over (set by the cursor thread)
static volatile clock_t g_ignoreClicksUntil = 0; // our own simulated click must not trigger vNAAATS actions

// Click once at a point of the radar window (client pixels), then put the mouse back where it was.
static bool ClickRadarPoint(POINT clientPt) {
	HWND w = g_radarWnd;
	if (!w || !IsWindow(w)) return false;
	POINT sp = clientPt;
	if (!ClientToScreen(w, &sp)) return false;
	POINT back; GetCursorPos(&back);
	g_ignoreClicksUntil = clock() + 1500;
	// a small move first so EuroScope sees the pointer arrive over the aircraft, then the click with the button set in cfg (contactbutton)
	SetCursorPos(sp.x - 2, sp.y - 2);
	Sleep(80);
	SetCursorPos(sp.x, sp.y);
	Sleep(200);
	DWORD down = MOUSEEVENTF_LEFTDOWN, up = MOUSEEVENTF_LEFTUP;
	if (CDataHandler::ContactButton == 1) { down = MOUSEEVENTF_MIDDLEDOWN; up = MOUSEEVENTF_MIDDLEUP; }
	else if (CDataHandler::ContactButton == 2) { down = MOUSEEVENTF_RIGHTDOWN; up = MOUSEEVENTF_RIGHTUP; }
	mouse_event(down, 0, 0, 0, 0);
	Sleep(60);
	mouse_event(up, 0, 0, 0, 0);
	Sleep(250);
	SetCursorPos(back.x, back.y);
	return true;
}

struct CTypeJob { string Text; int Id; bool Click = false; POINT Px = { 0, 0 }; };
static volatile LONG TypingBusy = 0;
static void TypeThread(void* arg) {
	CTypeJob* job = (CTypeJob*)arg;
	try {
		string why = "Windows would not let the plugin bring EuroScope to the front - click on EuroScope and try again, or use Copy and paste";
		bool typedOk = CDataHandler::BackgroundType ? TypeIntoEuroScopeBackground(job->Text, why) : TypeIntoEuroScope(job->Text, why);
		if (typedOk) {
			string note = string(CDataHandler::BackgroundType ? "Sent to EuroScope (background): " : "Sent to EuroScope: ") + job->Text.substr(0, 40);
			if (job->Click) {
				Sleep(500);                       // EuroScope now waits for a click on the aircraft
				if (ClickRadarPoint(job->Px)) note += " (aircraft clicked for you)";
				else note += " - click the aircraft to send it";
			}
			CDataHandler::ReportRemoteResult(job->Id, true, note);
		}
		else CDataHandler::ReportRemoteResult(job->Id, false, why);
	}
	catch (...) { CDataHandler::ReportRemoteResult(job->Id, false, "plugin error while typing"); }
	Sleep(1200);                                    // leave EuroScope time to run the line before the next one
	InterlockedExchange(&TypingBusy, 0);
	delete job;
}

// Routes drawn with a right click on a tag (CRoutesHelper::ActiveRoutes) and when each one is to be erased (routeseconds in vnaaats.cfg)
static std::map<string, clock_t> g_routeExpiry;

static int RetrackPhase = 0; // see RunRemoteCommands: 0 = not connected

// Aircraft I was tracking whose pilot disconnected: if the pilot is back within 5 minutes and nobody else has the aircraft,
// track it again with the Flight Level, Mach and SELCAL it had. (Runs about every 10 seconds, on the EuroScope thread.)
static void RunTrackMemos(CPlugIn* plugin, const std::set<string>& mineNow) {
	const double WINDOW_SECONDS = 300.0;
	time_t now = time(NULL);
	for (auto& kv : CDataHandler::MemoSnapshot()) {
		const string& cs = kv.first;
		const CDataHandler::CTrackMemo& m = kv.second;
		try {
			CFlightPlan es = plugin->FlightPlanSelect(cs.c_str());
			CRadarTarget rt = plugin->RadarTargetSelect(cs.c_str());
			if (m.DisconnectedAt == 0) {
				if (mineNow.count(cs)) continue;                                  // still tracking it
				// no longer mine: pilot gone (remember it) or I released / handed it off (forget it)
				if (!es.IsValid() || !rt.IsValid()) CDataHandler::MemoDisconnected(cs);
				else CDataHandler::MemoForget(cs);
				continue;
			}
			if (difftime(now, m.DisconnectedAt) > WINDOW_SECONDS) { CDataHandler::MemoForget(cs); continue; }   // too late
			if (difftime(now, m.DisconnectedAt) < 10.0) continue;                 // let EuroScope drop the old one first
			if (!es.IsValid() || !rt.IsValid()) continue;                         // not back yet
			if (rt.GetPosition().GetReceivedTime() > 30) continue;                // flight plan still there but no fresh position
			if (es.GetTrackingControllerIsMe() || !string(es.GetTrackingControllerCallsign()).empty()) { CDataHandler::MemoForget(cs); continue; }   // someone has it
			CDataHandler::CRemoteCommand cmd;
			cmd.Id = 0; cmd.Type = "track"; cmd.Callsign = cs; cmd.Level = m.Level; cmd.Mach = m.Mach;
			cmd.Selcal = (m.Selcal == "N/A") ? "" : m.Selcal;
			CDataHandler::QueueLocalCommand(cmd);
			CLogger::Log(CLogType::NORM, cs + " is back within 5 minutes: tracking it again (FL" + m.Level + " M" + m.Mach + (cmd.Selcal.empty() ? "" : " SELCAL " + cmd.Selcal) + ").", "RunTrackMemos");
			CDataHandler::MemoForget(cs);
		}
		catch (...) { CDataHandler::MemoForget(cs); }
	}
}

void CRadarDisplay::RunRemoteCommands()
{
	CPlugIn* plugin = GetPlugIn();
	// Flight data that EuroScope reported as out of scope is erased here, on the draw thread, after anything pointing at it is cleared
	for (const string& cs : CDataHandler::TakeQueuedDeletes()) {
		if (cs == asel) {
			asel = "";
			menuBar->SetButtonState(CMenuBar::BTN_FLIGHTPLAN, CInputState::INACTIVE);
		}
		for (auto it = CRoutesHelper::ActiveRoutes.begin(); it != CRoutesHelper::ActiveRoutes.end();) {
			if (*it == cs) it = CRoutesHelper::ActiveRoutes.erase(it); else ++it;
		}
		CDataHandler::DeleteFlightData(cs);
	}
	// Settings chosen on the FDD: check every 2 minutes; changed ones are written to vnaaats.cfg and apply after a restart of EuroScope
	{
		static clock_t lastSettings = 0;
		if (CDataHandler::SettingsBusy == 0 && (lastSettings == 0 || (double)(clock() - lastSettings) / (double)CLOCKS_PER_SEC >= 120.0)) {
			lastSettings = clock();
			InterlockedExchange(&CDataHandler::SettingsBusy, 1);
			_beginthread(CDataHandler::FetchSettings, 0, NULL);
		}
		if (InterlockedExchange(&CDataHandler::SettingsMessagePending, 0))
			plugin->DisplayUserMessage("vNAAATS", "Settings", "Your vNAAATS settings were changed on the FDD and saved in vnaaats.cfg. Close EuroScope and open it again for them to take effect.", true, true, true, true, true);
	}
	CDataHandler::ApplyPendingNetwork(this); // changes fetched from the server (e.g. edits made on the web FDD), applied on this thread
	if (!plugin->ControllerMyself().IsValid() || !plugin->ControllerMyself().IsController()) { RetrackPhase = 0; return; }   // disconnected: start the recovery again next time
	CDataHandler::ControllerCallsign = plugin->ControllerMyself().GetCallsign();

	// Crash / disconnection recovery: when I (re)connect as a controller, track again the aircraft I had before.
	//  phase 0 = not connected, 1 = settling (EuroScope needs time to load the flight plans), 2 = fetching, 3 = re-tracking, 4 = normal
	{
		int& phase = RetrackPhase;
		static clock_t phaseAt = 0, lastTrack = 0;
		static vector<string> todo;
		static string lastReported = "";
		static clock_t lastReport = 0;
		if (!CDataHandler::Retrack) phase = 4;
		else if (phase == 0) { phase = 1; phaseAt = clock(); }
		double sinceAt = (double)(clock() - phaseAt) / (double)CLOCKS_PER_SEC;
		if (phase == 1 && sinceAt >= 20.0) {
			InterlockedExchange(&CDataHandler::RetrackFetched, 0);
			phase = 2; phaseAt = clock();
			_beginthread(CDataHandler::FetchRetrack, 0, nullptr);
		}
		else if (phase == 2 && CDataHandler::RetrackFetched) { todo = CDataHandler::TakeRetrack(); phase = todo.empty() ? 4 : 3; phaseAt = clock(); }
		else if (phase == 2 && sinceAt > 30.0) phase = 4;                                     // server did not answer: carry on
		else if (phase == 3 && (double)(clock() - lastTrack) / (double)CLOCKS_PER_SEC >= 1.0) {
			lastTrack = clock();
			if (todo.empty() || sinceAt > 120.0) { phase = 4; lastReported = "\x01"; }
			else {
				string cs = todo.back(); todo.pop_back();
				CFlightPlan es = plugin->FlightPlanSelect(cs.c_str());
				// only aircraft nobody is tracking: never take one from another controller
				if (es.IsValid() && !es.GetTrackingControllerIsMe() && string(es.GetTrackingControllerCallsign()).empty()) {
					if (es.StartTracking()) CLogger::Log(CLogType::NORM, "Tracking " + cs + " again after reconnecting.", "CRadarDisplay::RunRemoteCommands");
				}
			}
		}
		// Tell the server what I am tracking (when it changes, checked every 10 s) - only once recovery is finished
		if (phase == 4 && (CDataHandler::Retrack || CDataHandler::SquawkOnClearance) && (double)(clock() - lastReport) / (double)CLOCKS_PER_SEC >= 10.0) {
			lastReport = clock();
			string list;
			std::set<string> mineNow;
			for (CFlightPlan f = plugin->FlightPlanSelectFirst(); f.IsValid(); f = plugin->FlightPlanSelectNext(f)) {
				if (f.GetTrackingControllerIsMe()) {
					list += (list.empty() ? "" : ",") + string(f.GetCallsign());
					string csNow = f.GetCallsign();
					mineNow.insert(csNow);
					// Remember what I set on it, in case its pilot disconnects and comes back
					CAircraftFlightPlan* mfp = CDataHandler::GetFlightData(csNow);
					if (mfp->IsValid && !mfp->FlightLevel.empty() && !mfp->Mach.empty() && atoi(mfp->FlightLevel.c_str()) > 0 && atoi(mfp->Mach.c_str()) > 0)
						CDataHandler::MemoTracked(csNow, mfp->FlightLevel, mfp->Mach, mfp->SELCAL);
				}
			}
			// Squawk 2000 as soon as I start tracking an aircraft (domestic reassign it when it is back in radar service).
			// Aircraft I was already tracking when this display started are left alone.
			static std::set<string> prevMine;
			static bool prevMineKnown = false;
			if (CDataHandler::SquawkOnClearance && prevMineKnown) {
				for (auto& c : mineNow) if (prevMine.find(c) == prevMine.end()) CUtils::AssignOceanicSquawk(plugin, c);
			}
			prevMine = mineNow;
			prevMineKnown = true;
			if (CDataHandler::Retrack) RunTrackMemos(plugin, mineNow);
			static clock_t lastSent = 0;
			// also sent once a minute when nothing changed: this is how the server knows I was still here before a crash
			if (list != lastReported || (double)(clock() - lastSent) / (double)CLOCKS_PER_SEC >= 60.0) {
				lastReported = list;
				lastSent = clock();
				_beginthread(CDataHandler::ReportTracked, 0, (void*)new CTrackedPush{ CDataHandler::ControllerCallsign, list });
			}
		}
	}

	// Send any waiting log lines to the admin log (background thread, every 5 s at most)
	CLogger::MaybeFlushRemote();

	// Ask the server for new commands about every 2 seconds
	if (!CDataHandler::RemotePollBusy && (double)(clock() - CDataHandler::RemoteLastPoll) / (double)CLOCKS_PER_SEC >= 1.0) {
		CDataHandler::RemoteLastPoll = clock();
		CDataHandler::RemotePollBusy = true;
		_beginthread(CDataHandler::PollRemoteCommands, 0, nullptr);
	}

	vector<CDataHandler::CRemoteCommand> cmds = CDataHandler::TakeRemoteCommands();
	// Typed commands (.selcal / .contactme / .msg) go one at a time, at least 1.5 s apart: EuroScope needs time to run the
	// previous line, otherwise two commands end up joined on one line in the command box.
	static vector<CDataHandler::CRemoteCommand> typeBacklog;
	static clock_t lastTyped = 0;
	vector<CDataHandler::CRemoteCommand> now;
	for (auto& c : cmds) {
		if (c.Type == "selcal" || c.Type == "contactme" || c.Type == "natmsg") typeBacklog.push_back(c);
		else now.push_back(c);
	}
	if (!typeBacklog.empty() && TypingBusy == 0 && (lastTyped == 0 || (double)(clock() - lastTyped) / (double)CLOCKS_PER_SEC >= 1.5)) {
		now.push_back(typeBacklog.front());
		typeBacklog.erase(typeBacklog.begin());
		lastTyped = clock();
	}
	cmds = now;
	for (auto& cmd : cmds) {
		try {
			CFlightPlan es = plugin->FlightPlanSelect(cmd.Callsign.c_str());
			if (cmd.Type == "highlight") {
				// the circle only needs the aircraft's radar target (not a flight plan)
				if (!CDataHandler::Highlight) { CDataHandler::ReportRemoteResult(cmd.Id, true, "circle is switched off (highlight=0 in vnaaats.cfg)"); continue; }
				if (!plugin->RadarTargetSelect(cmd.Callsign.c_str()).IsValid()) { CDataHandler::ReportRemoteResult(cmd.Id, false, cmd.Callsign + " is not on your EuroScope radar"); continue; }
				hlCallsign = cmd.Callsign;
				hlUntil = clock() + 3000;
				CDataHandler::ReportRemoteResult(cmd.Id, true, "Circled " + cmd.Callsign);
				continue;
			}
			if (!es.IsValid()) { CDataHandler::ReportRemoteResult(cmd.Id, false, "EuroScope has no flight plan for " + cmd.Callsign); continue; }
			bool mine = es.GetTrackingControllerIsMe();
			string trackedBy = es.GetTrackingControllerCallsign();

			if (cmd.Type == "track") {
				// Another controller has offered it to me (a handoff): tracking it accepts the handoff
				bool handoffToMe = !mine && string(es.GetHandoffTargetControllerCallsign()) == string(plugin->ControllerMyself().GetCallsign());
				if (!mine && !trackedBy.empty() && !handoffToMe) { CDataHandler::ReportRemoteResult(cmd.Id, false, cmd.Callsign + " is already tracked by " + trackedBy); continue; }
				CAircraftFlightPlan* fp = CDataHandler::GetFlightData(cmd.Callsign);
				if (!fp->IsValid) {
					// The aircraft is not on this radar's list yet (outside the filters): load it now rather than make the controller wait
					CDataHandler::CreateFlightData(this, cmd.Callsign);
					fp = CDataHandler::GetFlightData(cmd.Callsign);
				}
				if (!fp->IsValid) { CDataHandler::ReportRemoteResult(cmd.Id, false, "vNAAATS has not loaded " + cmd.Callsign + " yet - try again in a few seconds"); continue; }
				// StartTracking() only works on an untracked aircraft; a handoff offered to me has to be accepted instead
				bool started = true;
				if (handoffToMe) es.AcceptHandoff();
				else if (!mine) started = es.StartTracking();
				if (!started) {
					string why = "tracked by " + (trackedBy.empty() ? string("nobody") : trackedBy) + ", handoff offered to " + (string(es.GetHandoffTargetControllerCallsign()).empty() ? string("nobody") : string(es.GetHandoffTargetControllerCallsign())) + ", you are " + string(plugin->ControllerMyself().GetCallsign());
					CLogger::Log(CLogType::WARN, "StartTracking refused for " + cmd.Callsign + ": " + why + ".", "CRadarDisplay::RunRemoteCommands");
					CDataHandler::ReportRemoteResult(cmd.Id, false, "EuroScope would not let you track " + cmd.Callsign + " (" + why + ")");
					continue;
				}
				// Level and Mach are optional: when the controller leaves them blank keep what is there, else take EuroScope's own figures
				if (!cmd.Level.empty()) fp->FlightLevel = cmd.Level;
				else if (fp->FlightLevel.empty() || fp->FlightLevel == "0") {
					int fl = es.GetControllerAssignedData().GetFinalAltitude() / 100;
					if (fl <= 0) fl = es.GetFlightPlanData().GetFinalAltitude() / 100;
					fp->FlightLevel = to_string(fl > 0 ? fl : 0);
				}
				if (!cmd.Mach.empty()) fp->Mach = cmd.Mach;
				else if (fp->Mach.empty() || fp->Mach == "0") {
					int mc = es.GetControllerAssignedData().GetAssignedMach() / 10;
					fp->Mach = to_string(mc > 0 ? mc : 0);
				}
				if (!cmd.Selcal.empty()) { fp->SELCAL = cmd.Selcal; fp->SelcalManual = true; }
				// EuroScope needs a moment to report the new tracker after StartTracking(): if it still says nobody, it is me
				string myCallsign = plugin->ControllerMyself().IsValid() ? string(plugin->ControllerMyself().GetCallsign()) : "";
				string myId = plugin->ControllerMyself().IsValid() ? string(plugin->ControllerMyself().GetPositionId()) : "";
				string trkCallsign = plugin->FlightPlanSelect(cmd.Callsign.c_str()).GetTrackingControllerCallsign();
				string trkId = plugin->FlightPlanSelect(cmd.Callsign.c_str()).GetTrackingControllerId();
				if (trkCallsign.empty() || handoffToMe) { trkCallsign = myCallsign; trkId = myId; }
				fp->Sector = trkId;
				fp->IsCleared = true;
				if (CDataHandler::SquawkOnClearance) CUtils::AssignOceanicSquawk(plugin, cmd.Callsign);

				CNetworkFlightPlan* netFP = new CNetworkFlightPlan();
				netFP->Callsign = fp->Callsign;
				netFP->Type = fp->Type;
				netFP->AssignedLevel = atoi(fp->FlightLevel.c_str());
				netFP->AssignedMach = atoi(fp->Mach.c_str());
				netFP->Track = fp->Track;
				netFP->Departure = fp->Depart;
				netFP->Arrival = fp->Dest;
				netFP->Direction = CUtils::GetAircraftDirection(plugin->RadarTargetSelect(cmd.Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth());
				netFP->Selcal = fp->SELCAL == "N/A" ? "" : fp->SELCAL;
				netFP->DatalinkConnected = fp->DLStatus == "true";
				netFP->IsEquipped = fp->IsEquipped;
				netFP->State = fp->State;
				netFP->Etd = fp->Etd;
				netFP->Relevant = true;
				netFP->TargetMode = CUtils::GetTargetMode(plugin->RadarTargetSelect(cmd.Callsign.c_str()).GetPosition().GetRadarFlags());
				netFP->TrackedBy = trkCallsign;
				netFP->TrackedById = trkId;
				netFP->Route = "";
				netFP->RouteEtas = "";
				vector<CRoutePosition> rte;
				CRoutesHelper::GetRoute(this, &rte, cmd.Callsign);
				for (size_t i = 0; i < rte.size(); i++) {
					netFP->Route += rte[i].Fix + (i + 1 < rte.size() ? " " : "");
					netFP->RouteEtas += rte[i].Estimate + (i + 1 < rte.size() ? " " : "");
				}
				CUtils::CNetworkAsyncData* data = new CUtils::CNetworkAsyncData();
				data->Screen = this;
				data->Callsign = fp->Callsign;
				data->FP = netFP;
				_beginthread(CDataHandler::PostNetworkAircraft, 0, (void*)data);
				CDataHandler::ReportRemoteResult(cmd.Id, true, handoffToMe ? "Accepted the handoff and tracking " + cmd.Callsign : "Tracking " + cmd.Callsign);
			}
			else if (cmd.Type == "untrack") {
				if (!mine) { CDataHandler::ReportRemoteResult(cmd.Id, false, cmd.Callsign + " is not tracked by you"); continue; }
				es.EndTracking();
				CAircraftFlightPlan* fp = CDataHandler::GetFlightData(cmd.Callsign);
				if (fp->IsValid) fp->Sector = "-1";
				CDataHandler::ReportRemoteResult(cmd.Id, true, "Released " + cmd.Callsign);
			}
			else if (cmd.Type == "takeover") {
				// Relief: the controller before me has handed me this aircraft (or has gone and left it untracked)
				if (mine) { CDataHandler::ReportRemoteResult(cmd.Id, true, "Already tracking " + cmd.Callsign); continue; }
				bool handoffToMe = string(es.GetHandoffTargetControllerCallsign()) == string(plugin->ControllerMyself().GetCallsign());
				if (!trackedBy.empty() && !handoffToMe) { CDataHandler::ReportRemoteResult(cmd.Id, false, cmd.Callsign + " is tracked by " + trackedBy + ", not offered to you"); continue; }
				bool started = true;
				if (handoffToMe) es.AcceptHandoff(); else started = es.StartTracking();
				if (!started) { CDataHandler::ReportRemoteResult(cmd.Id, false, "EuroScope would not let you track " + cmd.Callsign); continue; }
				CAircraftFlightPlan* fp = CDataHandler::GetFlightData(cmd.Callsign);
				if (fp->IsValid) {
					string myId = plugin->ControllerMyself().IsValid() ? string(plugin->ControllerMyself().GetPositionId()) : "";
					fp->Sector = myId;
				}
				CDataHandler::ReportRemoteResult(cmd.Id, true, "Took over " + cmd.Callsign);
			}
			else if (cmd.Type == "transfer") {
				if (!mine) { CDataHandler::ReportRemoteResult(cmd.Id, false, "Track " + cmd.Callsign + " first - you can only transfer your own aircraft"); continue; }
				CController target = plugin->ControllerSelect(cmd.Target.c_str());
				if (!target.IsValid()) { CDataHandler::ReportRemoteResult(cmd.Id, false, cmd.Target + " is not online in EuroScope"); continue; }
				if (!es.InitiateHandoff(cmd.Target.c_str())) { CDataHandler::ReportRemoteResult(cmd.Id, false, "EuroScope would not start the handoff to " + cmd.Target); continue; }
				CDataHandler::ReportRemoteResult(cmd.Id, true, "Handoff to " + cmd.Target + " started");
			}
			else if (cmd.Type == "selcal" || cmd.Type == "contactme" || cmd.Type == "natmsg") {
				if (!CDataHandler::AutoType) { CDataHandler::ReportRemoteResult(cmd.Id, false, "auto-typing is off (add autotype=1 to vnaaats.cfg) - use Copy and paste instead"); continue; }
				if (!(cmd.Text.rfind(".selcal ", 0) == 0 || cmd.Text == ".contactme" || cmd.Text.rfind(".contactme ", 0) == 0 || cmd.Text.rfind(".msg ", 0) == 0)) { CDataHandler::ReportRemoteResult(cmd.Id, false, "refused: not an allowed command"); continue; }
				// .selcal and .contactme act on the selected aircraft (they take no callsign), so select it first
				if (cmd.Type == "selcal" || cmd.Type == "contactme") plugin->SetASELAircraft(es);
				InterlockedExchange(&TypingBusy, 1);
				CTypeJob* job = new CTypeJob{ cmd.Text, cmd.Id };
				if (cmd.Type == "contactme" && CDataHandler::AutoClick) {
					// where the aircraft is on this radar (worked out here, on EuroScope's own thread)
					// EuroScope waits for a click on the callsign of the tag, so click that when the tag was drawn just now
					CRect area0(GetRadarArea());
					auto cp = CAcTargets::CallsignPoints.find(cmd.Callsign);
					if (cp != CAcTargets::CallsignPoints.end() && clock() - cp->second.second < 3000 && area0.PtInRect(cp->second.first)) { job->Click = true; job->Px = cp->second.first; }
					CRadarTarget tgt = plugin->RadarTargetSelect(cmd.Callsign.c_str());
					if (!job->Click && tgt.IsValid()) {
						POINT px = ConvertCoordFromPositionToPixel(tgt.GetPosition().GetPosition());
						CRect area(GetRadarArea());
						if (area.PtInRect(px)) { job->Click = true; job->Px = px; }
					}
				}
				_beginthread(TypeThread, 0, (void*)job);   // the thread reports the result
			}
			else if (cmd.Type == "highlight") {
				if (!CDataHandler::Highlight) { CDataHandler::ReportRemoteResult(cmd.Id, true, "circle is switched off (highlight=0 in vnaaats.cfg)"); continue; }
				hlCallsign = cmd.Callsign;
				hlUntil = clock() + 3000;
				CDataHandler::ReportRemoteResult(cmd.Id, true, "Circled " + cmd.Callsign);
			}
			else if (cmd.Type == "planget") {
				// Hand the whole flight plan to the FDD edit window (as JSON in the result message)
				CFlightPlanData fd = es.GetFlightPlanData();
				auto S = [](const char* v) { return string(v ? v : ""); };
				nlohmann::json j;
				j["callsign"] = cmd.Callsign;
				j["planType"] = S(fd.GetPlanType());
				j["acInfo"] = S(fd.GetAircraftInfo());
				j["origin"] = S(fd.GetOrigin());
				j["dest"] = S(fd.GetDestination());
				j["alternate"] = S(fd.GetAlternate());
				j["tas"] = fd.GetTrueAirspeed();
				j["alt"] = fd.GetFinalAltitude();
				j["etd"] = S(fd.GetEstimatedDepartureTime());
				j["enrouteH"] = S(fd.GetEnrouteHours());
				j["enrouteM"] = S(fd.GetEnrouteMinutes());
				j["fuelH"] = S(fd.GetFuelHours());
				j["fuelM"] = S(fd.GetFuelMinutes());
				j["route"] = S(fd.GetRoute());
				j["remarks"] = S(fd.GetRemarks());
				j["mine"] = mine;
				j["trackedBy"] = trackedBy;
				CDataHandler::ReportRemoteResult(cmd.Id, true, j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
			}
			else if (cmd.Type == "planset") {
				// Only the controller tracking the aircraft may change its flight plan
				if (!mine) { CDataHandler::ReportRemoteResult(cmd.Id, false, "Track " + cmd.Callsign + " first - only the tracking controller can change its flight plan"); continue; }
				nlohmann::json p = nlohmann::json::parse(cmd.Plan, nullptr, false);
				if (p.is_discarded() || !p.is_object()) { CDataHandler::ReportRemoteResult(cmd.Id, false, "no flight plan received"); continue; }
				CFlightPlanData fd = es.GetFlightPlanData();
				auto S = [](const char* v) { return string(v ? v : ""); };
				string failed;
				// Change only what differs, so a "no change" answer from EuroScope is never taken as a refusal
				auto setStr = [&](const char* name, const string& want, const string& have, const std::function<bool(const char*)>& setter) {
					if (want != have && !setter(want.c_str())) failed += string(failed.empty() ? "" : ", ") + name;
				};
				setStr("flight rules", p.value("planType", "I"), S(fd.GetPlanType()), [&](const char* v) { return fd.SetPlanType(v); });
				setStr("aircraft data", p.value("acInfo", ""), S(fd.GetAircraftInfo()), [&](const char* v) { return fd.SetAircraftInfo(v); });
				setStr("departure", p.value("origin", ""), S(fd.GetOrigin()), [&](const char* v) { return fd.SetOrigin(v); });
				setStr("destination", p.value("dest", ""), S(fd.GetDestination()), [&](const char* v) { return fd.SetDestination(v); });
				setStr("alternate", p.value("alternate", ""), S(fd.GetAlternate()), [&](const char* v) { return fd.SetAlternate(v); });
				setStr("departure time", p.value("etd", ""), S(fd.GetEstimatedDepartureTime()), [&](const char* v) { return fd.SetEstimatedDepartureTime(v); });
				setStr("enroute hours", to_string(p.value("enrouteH", 0)), to_string(atoi(fd.GetEnrouteHours())), [&](const char* v) { return fd.SetEnrouteHours(v); });
				setStr("enroute minutes", to_string(p.value("enrouteM", 0)), to_string(atoi(fd.GetEnrouteMinutes())), [&](const char* v) { return fd.SetEnrouteMinutes(v); });
				setStr("fuel hours", to_string(p.value("fuelH", 0)), to_string(atoi(fd.GetFuelHours())), [&](const char* v) { return fd.SetFuelHours(v); });
				setStr("fuel minutes", to_string(p.value("fuelM", 0)), to_string(atoi(fd.GetFuelMinutes())), [&](const char* v) { return fd.SetFuelMinutes(v); });
				setStr("route", p.value("route", ""), S(fd.GetRoute()), [&](const char* v) { return fd.SetRoute(v); });
				setStr("remarks", p.value("remarks", ""), S(fd.GetRemarks()), [&](const char* v) { return fd.SetRemarks(v); });
				int tas = p.value("tas", 0), alt = p.value("alt", 0);
				if (tas != fd.GetTrueAirspeed() && !fd.SetTrueAirspeed(tas)) failed += string(failed.empty() ? "" : ", ") + "TAS";
				if (alt != fd.GetFinalAltitude() && !fd.SetFinalAltitude(alt)) failed += string(failed.empty() ? "" : ", ") + "altitude";
				if (!failed.empty()) { CLogger::Log(CLogType::WARN, "Flight plan change for " + cmd.Callsign + " refused by EuroScope: " + failed, "CRadarDisplay::RunRemoteCommands"); CDataHandler::ReportRemoteResult(cmd.Id, false, "EuroScope would not change: " + failed); continue; }
				if (!fd.AmendFlightPlan()) { CDataHandler::ReportRemoteResult(cmd.Id, false, "EuroScope did not send the amended flight plan"); continue; }
				// Keep vNAAATS' own copy in step
				CAircraftFlightPlan* fp = CDataHandler::GetFlightData(cmd.Callsign);
				if (fp->IsValid) {
					fp->Type = S(fd.GetAircraftFPType());
					fp->Depart = S(fd.GetOrigin());
					fp->Dest = S(fd.GetDestination());
					fp->Etd = CUtils::ParseZuluTime(false, atoi(fd.GetEstimatedDepartureTime()));
					fp->IsEquipped = CUtils::IsAircraftEquipped(S(fd.GetRemarks()), S(fd.GetAircraftInfo()), fd.GetCapibilities());
					fp->WindowRefresh = true;
				}
				CLogger::Log(CLogType::NORM, "Flight plan for " + cmd.Callsign + " amended from the FDD.", "CRadarDisplay::RunRemoteCommands");
				CDataHandler::ReportRemoteResult(cmd.Id, true, "Flight plan for " + cmd.Callsign + " amended");
			}
			else {
				CDataHandler::ReportRemoteResult(cmd.Id, false, "unknown command");
			}
		}
		catch (exception & e) {
			CLogger::Log(CLogType::EXC, "FDD command failed: " + string(e.what()), "CRadarDisplay::RunRemoteCommands");
			CDataHandler::ReportRemoteResult(cmd.Id, false, "plugin error");
		}
	}
}

void CRadarDisplay::OnRefresh(HDC hDC, int Phase)
{
	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
		RunRemoteCommands();

		// Sensible traffic filters for the position you log on as (once per position)
		static string defaultsFor = "";
		CController me = GetPlugIn()->ControllerMyself();
		// The web display sends commands to the callsign you are logged on as: set it on every refresh, not only when an aircraft updates
		if (me.IsValid()) CDataHandler::ControllerCallsign = me.GetCallsign();
		if (me.IsValid() && me.IsController()) {
			string myCs = me.GetCallsign();
			if (myCs != defaultsFor) {
				defaultsFor = myCs;
				string up = myCs;
				for (auto& ch : up) ch = (char)toupper((unsigned char)ch);
				if (up.rfind("NAT", 0) == 0) { // NAT_FSS bandbox: the whole area, all traffic
					CUtils::AreaSelection = 800;
					CUtils::PosType = 801;
					menuBar->SetDropDownValue(CMenuBar::DRP_AREASEL, CUtils::AreaSelection);
					menuBar->SetDropDownValue(CMenuBar::DRP_TYPESEL, CUtils::PosType);
					CLogger::Log(CLogType::NORM, "Logged on as " + myCs + ": area OCA and Multi-role selected.", "CRadarDisplay::OnRefresh");
				}
			}
		}
	}

	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Graphics object
	Graphics g(hDC);

	// 5 second timer
	double fiveSecT = (double)(clock() - fiveSecondTimer) / ((double)CLOCKS_PER_SEC);
	// 10 second timer
	double tenSecT = (double)(clock() - tenSecondTimer) / ((double)CLOCKS_PER_SEC);

	// Clear lists if not empty and time is greater than 1 second
	if (fiveSecT >= 5 && !inboundList->AircraftList.empty()) {
		inboundList->AircraftList.clear();
		CLogger::Log(CLogType::NORM, "Refreshing Inbound list.", "CRadarDisplay::OnRefresh");
	}
	if (fiveSecT >= 5 && !otherList->AircraftList.empty()) {
		otherList->AircraftList.clear();
		CLogger::Log(CLogType::NORM, "Refreshing Other list.", "CRadarDisplay::OnRefresh");
	}

	// Online controllers
	if (fiveSecT >= 5) {
		// Clear online controllers first
		if (!fltPlnWindow->onlineControllers.empty())
			fltPlnWindow->onlineControllers.clear();
		// Get controllers
		CController controller;
		for (controller = GetPlugIn()->ControllerSelectFirst(); controller.IsValid(); controller = GetPlugIn()->ControllerSelectNext(controller)) {
			if (string(controller.GetCallsign()).find("CTR") != string::npos || string(controller.GetCallsign()).find("FSS") != string::npos)
			fltPlnWindow->onlineControllers[controller.GetCallsign()] = controller;
		}
		CLogger::Log(CLogType::NORM, "Refreshing controller list.", "CRadarDisplay::OnRefresh");

		// Log cursor position every 5 seconds
		CLogger::Log(CLogType::NORM, "Current cursor position: (" + to_string(appCursor->position.x) + ", " + to_string(appCursor->position.y) + ")", "CRadarDisplay::OnRefresh");
	}

	// Set the ASEL if it is different to the current one in program
	string cs = GetPlugIn()->FlightPlanSelectASEL().GetCallsign();
	if (GetPlugIn()->FlightPlanSelectASEL().GetCallsign() != asel) {
		asel = GetPlugIn()->FlightPlanSelectASEL().GetCallsign();
		CLogger::Log(CLogType::NORM, "Selected aircraft changed to " + asel + ".", "CRadarDisplay::OnRefresh");
		// Tell the server, so the FDD can outline this aircraft's strip
		if (!CDataHandler::ControllerCallsign.empty()) _beginthread(CDataHandler::ReportAsel, 0, (void*)new CAselPush{ CDataHandler::ControllerCallsign, asel });
	}

	// Set the flight plan button state
	if (aircraftOnScreen.empty() || asel == "" || !CDataHandler::GetFlightData(asel)->IsValid || !CDataHandler::GetFlightData(asel)->IsFirstUpdate || !GetPlugIn()->ControllerMyself().IsController()) {
		if (menuBar->GetButtonState(CMenuBar::BTN_FLIGHTPLAN) != CInputState::DISABLED)
			menuBar->SetButtonState(CMenuBar::BTN_FLIGHTPLAN, CInputState::DISABLED);
	}
	else {
		if (menuBar->GetButtonState(CMenuBar::BTN_FLIGHTPLAN) == CInputState::DISABLED && menuBar->GetButtonState(CMenuBar::BTN_FLIGHTPLAN) != CInputState::ACTIVE)
			menuBar->SetButtonState(CMenuBar::BTN_FLIGHTPLAN, CInputState::INACTIVE);
	}

	// Reset currently on screen list
	if (tenSecT >= 10 && !aircraftOnScreen.empty()) {
		CLogger::Log(CLogType::NORM, "Refreshing internal aircraft on screen list.", "CRadarDisplay::OnRefresh");
		// Loop on screen aircraft
		auto idx = aircraftOnScreen.begin();
		while(idx != aircraftOnScreen.end()) {
			// Check if valid
			CRadarTarget target = GetPlugIn()->RadarTargetSelect(idx->first.c_str());
			bool isValid = CUtils::IsAircraftRelevant(this, &target, menuBar->IsButtonPressed(CMenuBar::BTN_ALL));
			if (!target.IsValid() || !isValid) { // If not valid
				// Erase aircraft selections if they are an asel
				if (idx->first == aircraftSel1 || idx->first == aircraftSel2) { // We need to annul the selection and disable tool if activated
					aircraftSel1 = "";
					// Reset RBL (if active)
					if (menuBar->IsButtonPressed(CMenuBar::BTN_RBL)) {
						menuBar->SetButtonState(CMenuBar::BTN_RBL, CInputState::INACTIVE);
						CLogger::Log(CLogType::NORM, "Resetting RBL tool.", "CRadarDisplay::OnRefresh");
					}
					// Reset SEP (if active)
					if (menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
						CLogger::Log(CLogType::NORM, "Resetting SEP tool.", "CRadarDisplay::OnRefresh");
						menuBar->SetButtonState(CMenuBar::BTN_SEP, CInputState::INACTIVE);
					}
					// Reset PIV (if active)
					if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
						CLogger::Log(CLogType::NORM, "Resetting PIV tool.", "CRadarDisplay::OnRefresh");
						menuBar->SetButtonState(CMenuBar::BTN_PIV, CInputState::INACTIVE);
					}
				}
				// Erase ASEL
				if (idx->first == asel) {
					asel = "";
				}

				// Erase STCA if exists
				auto jdx = CConflictDetection::CurrentSTCA.begin();
				// Loop this way to avoid a vector overflow
				while (jdx != CConflictDetection::CurrentSTCA.end()) {
					if (idx->first == jdx->CallsignA || idx->first == jdx->CallsignB) {
						CLogger::Log(CLogType::NORM, "Erasing STCA for " + idx->first + ".", "CRadarDisplay::OnRefresh");
						jdx = CConflictDetection::CurrentSTCA.erase(jdx);						
					}
					else {
						jdx++;
					}
				}

				// Erase route if exists
				auto kdx = CRoutesHelper::ActiveRoutes.begin();
				// Loop this way to avoid a vector overflow
				while (kdx != CRoutesHelper::ActiveRoutes.end()) {
					if (idx->first == *kdx || idx->first == *kdx) {
						CLogger::Log(CLogType::NORM, "Erasing route for " + idx->first + ".", "CRadarDisplay::OnRefresh");
						kdx = CRoutesHelper::ActiveRoutes.erase(kdx);						
					}
					else {
						kdx++;
					}
				}

				if (CDataHandler::GetFlightData(idx->first)->IsValid) {
					CLogger::Log(CLogType::NORM, "Deleting flight data for " + idx->first + ".", "CRadarDisplay::OnRefresh");
					CDataHandler::DeleteFlightData(idx->first);					
				}

				// Erase flight plan window
				menuBar->SetButtonState(menuBar->BTN_FLIGHTPLAN, CInputState::DISABLED);

				// Finally erase the on screen reference
				CLogger::Log(CLogType::NORM, "Erasing reference for " + idx->first + ".", "CRadarDisplay");
				idx = aircraftOnScreen.erase(idx);
			}
			else {
				++idx; // We increment here to avoid a vector overflow
			}
		}
	}

	// Redo the PIV calculations every 5 seconds
	if (fiveSecT >= 5 && menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
		CLogger::Log(CLogType::NORM, "Recalculating PIV between " + aircraftSel1 + " and " + aircraftSel2 + ".", "CRadarDisplay::OnRefresh");
		try {
			CConflictDetection::PIVLocations1.clear();
			CConflictDetection::PIVLocations2.clear();
			CConflictDetection::PIVSeparationStatuses.clear();
			CConflictDetection::PIVTool(this, aircraftSel1, aircraftSel2);
		}
		catch (exception & ex) {
			CLogger::Log(CLogType::ERR, "An error occurred when trying to recalculate PIV. " + string(ex.what()), "CRadarDisplay::OnRefresh");
		}
		
	}

	// Check if the altitude filter is on
	bool altFiltEnabled = false;
	if (menuBar->IsButtonPressed(CMenuBar::BTN_ALTFILT)) altFiltEnabled = true;

	// Get the radar area
	CRect RadarArea(GetRadarArea());
	RadarArea.top = RadarArea.top - 1;
	RadarArea.bottom = GetChatArea().bottom;

	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
		/// Write current lat lon on screen
		int sDC = dc.SaveDC();
		// Select font
		FontSelector::SelectMonoFont(12, &dc);
		dc.SetTextColor(TextWhite.ToCOLORREF());
		dc.SetTextAlign(TA_LEFT);
		// Get radar area and lat lon
		CRect radarBounds = this->GetRadarArea();
		dc.TextOutA(radarBounds.right - 185, radarBounds.bottom - dc.GetTextExtent("ABC").cy - 2, 
			CUtils::GetLatLonString(&appCursor->latLonPosition).c_str());
		// Selected aircraft, bottom centre: "BAW227 (Speedbird)   B788   EGLL - KBOS"
		if (!asel.empty()) {
			CFlightPlan selFP = GetPlugIn()->FlightPlanSelect(asel.c_str());
			string line = asel;
			string tele = CUtils::TelephonyFor(asel);
			if (!tele.empty()) line += " (" + tele + ")";
			if (selFP.IsValid()) {
				string type = selFP.GetFlightPlanData().GetAircraftFPType();
				string dep = selFP.GetFlightPlanData().GetOrigin();
				string dest = selFP.GetFlightPlanData().GetDestination();
				if (!type.empty()) line += "   " + type;
				if (!dep.empty() || !dest.empty()) line += "   " + (dep.empty() ? string("????") : dep) + " - " + (dest.empty() ? string("????") : dest);
			}
			int selAlign = dc.SetTextAlign(TA_CENTER);
			dc.TextOutA((radarBounds.left + radarBounds.right) / 2, radarBounds.bottom - dc.GetTextExtent("ABC").cy - 2, line.c_str());
			dc.SetTextAlign(selAlign);
		}
		// White circle around an aircraft that was just clicked on the FDD (3 seconds)
		if (!hlCallsign.empty()) {
			if (clock() < hlUntil) {
				CRadarTarget hlt = GetPlugIn()->RadarTargetSelect(hlCallsign.c_str());
				if (hlt.IsValid()) {
					POINT hp = ConvertCoordFromPositionToPixel(hlt.GetPosition().GetPosition());
					CPen hpen(PS_SOLID, 3, RGB(255, 255, 255));
					CPen* oldPen = dc.SelectObject(&hpen);
					CBrush* oldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
					dc.Ellipse(hp.x - 24, hp.y - 24, hp.x + 24, hp.y + 24);
					dc.SelectObject(oldBrush);
					dc.SelectObject(oldPen);
				}
			}
			else hlCallsign.clear();
		}
		dc.RestoreDC(sDC);

		// Draw overlays if enabled
		if (menuBar->IsButtonPressed(CMenuBar::BTN_OVERLAYS)) {
			COverlays::ShowCurrentOverlay(&dc, &g, this, menuBar);
		}

		// Get first aircraft
		CRadarTarget ac;
		ac = GetPlugIn()->RadarTargetSelectFirst();

		// Get entry time and direction
		int entryMinutes;
		bool direction;

		// Erase routes that have been on the screen for routeseconds
		if (!CRoutesHelper::ActiveRoutes.empty()) {
			clock_t nowClock = clock();
			for (int i = (int)CRoutesHelper::ActiveRoutes.size() - 1; i >= 0; i--) {
				auto it = g_routeExpiry.find(CRoutesHelper::ActiveRoutes[i]);
				if (it != g_routeExpiry.end() && nowClock >= it->second) {
					g_routeExpiry.erase(it);
					CRoutesHelper::ActiveRoutes.erase(CRoutesHelper::ActiveRoutes.begin() + i);
				}
			}
		}

		// Draw routes
		if (CRoutesHelper::ActiveRoutes.size() != CRoutesHelper::ActiveRoutes.empty() && CRoutesHelper::ActiveRoutes.size() != 0) {
			CCommonRenders::RenderRoutes(&dc, &g, this);
		}

		// Loop all aircraft
		while (ac.IsValid()) {
			// The system plan
			CAircraftFlightPlan aircraftFlightPlan;
			CDataHandler::GetFlightData(cs.c_str(), aircraftFlightPlan);

			// Flight plan
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(ac.GetCallsign());

			// Route
			CFlightPlanExtractedRoute rte = fp.GetExtractedRoute();

			// Time and direction
			entryMinutes = fp.GetSectorEntryMinutes();
			direction = CUtils::GetAircraftDirection(ac.GetPosition().GetReportedHeading());
			
			//string debug = string(ac.GetCallsign()) + ":" + to_string(entryMinutes) + ":" + to_string(direction) + ":" + to_string(aircraftFlightPlan.ExitTime) + ":" + to_string(CUtils::GetTargetModeInt(ac.GetPosition().GetRadarFlags())) + "\n";

			//CLogger::LogAircraftDebugInfo(debug);

			// Check if they are a selected aircraft
			if (ac.GetCallsign() == CAcTargets::SearchedAircraft) {
				if (((double)(clock() - CAcTargets::fiveSecondTimer) / ((double)CLOCKS_PER_SEC)) >= 5) {
					CAcTargets::SearchedAircraft = "";
					CAcTargets::fiveSecondTimer = clock();
				}
				else {
					CAcTargets::RenderSelectionHalo(&g, this, &ac);
				}
			}

			// If PSSR button not pressed
			/*if (!menuBar->IsButtonPressed(CMenuBar::BTN_PSSR)) {
				// Check their PSSR state to hide all non ADS-B aircraft
				if (CUtils::GetTargetMode(ac.GetPosition().GetRadarFlags()) != CRadarTargetMode::ADS_B) {
					// Select the next target
					ac = GetPlugIn()->RadarTargetSelectNext(ac);
					if (aircraftOnScreen.find(ac.GetCallsign()) != aircraftOnScreen.end()) aircraftOnScreen.erase(ac.GetCallsign());
					continue;
				}
			}*/

			// Check their altitude, if they are outside the filter, skip them
			if (altFiltEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_ALL)) {
				if (ac.GetPosition().GetPressureAltitude() / 100 < CUtils::AltFiltLow || ac.GetPosition().GetPressureAltitude() / 100 > CUtils::AltFiltHigh) {
					// Select the next target
					ac = GetPlugIn()->RadarTargetSelectNext(ac);
					if (aircraftOnScreen.find(ac.GetCallsign()) != aircraftOnScreen.end()) aircraftOnScreen.erase(ac.GetCallsign());
					continue;
				}
			}

			// Check track filtering
			if (menuBar->GetButtonState(menuBar->BTN_TCKCTRL) == CInputState::ACTIVE && !menuBar->IsButtonPressed(CMenuBar::BTN_ALL)) {
				// Primed plan
				string cs = (string)fp.GetCallsign();
				vector<string> tracks;
				auto idx = find_if(CConflictDetection::CurrentSTCA.begin(), CConflictDetection::CurrentSTCA.end(), [&cs](const CSTCAStatus& obj) {return obj.CallsignA == cs || obj.CallsignB == cs; });
				if (idx == CConflictDetection::CurrentSTCA.end())
					menuBar->GetSelectedTracks(tracks);
				bool skipAircraft = tracks.empty() ? false : true;
				for (int i = 0; i < tracks.size(); i++) {
					if (aircraftFlightPlan.IsValid) {
						bool showAc = aircraftFlightPlan.Track == tracks[i];
						if (showAc) {
							skipAircraft = false;
							break;
						}						
					}
					else if (CRoutesHelper::OnNatTrack(this, string(fp.GetCallsign())) == tracks[i]) {
						skipAircraft = false;
						break;
					}
					else {
						continue;
					}
				}

				if (skipAircraft) {
					// Select the next target
					ac = GetPlugIn()->RadarTargetSelectNext(ac);
					continue;
				}
			}
			
			// Parse inbound & other
			bool filtersDisabled = menuBar->IsButtonPressed(CMenuBar::BTN_ALL);
			if (CUtils::IsAircraftRelevant(this, &ac, filtersDisabled)) {
				
				// If not there then add the status
				if (tagStatuses.find(fp.GetCallsign()) == tagStatuses.end()) {
					pair<bool, POINT> pt = make_pair(false, POINT{ 0, 0 });
					tagStatuses.insert(make_pair(string(fp.GetCallsign()), pt));
				}

				// STCA
				if (tenSecT >= 10) {
					CConflictDetection::CheckSTCA(this, &ac, &aircraftOnScreen);
				}

				if (fiveSecT >= 5) {
					// If going westbound
					if (!direction && entryMinutes > 0) {
						if (menuBar->GetDropDownValue(CMenuBar::DRP_AREASEL) == "CZQX") {
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								// Find out if 30 west is in their flight plan
								if (rte.GetPointPosition(i).m_Longitude == -30.0) {
									// Test flight time
									if (rte.GetPointDistanceInMinutes(i) > 0 && rte.GetPointDistanceInMinutes(i) < 60) {
										// Add if within
										inboundList->AircraftList.push_back(CInboundAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
												rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), false));
										break;
									}
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherList->AircraftList.push_back(ac.GetCallsign());
							}
						}
						else {
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								// They are coming from land so check entry points
								if (CUtils::IsEntryPoint(rte.GetPointName(i), direction) || CUtils::IsExitPoint(rte.GetPointName(i), direction)) {
									// Add if within
									inboundList->AircraftList.push_back(CInboundAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
										rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), false));
									break;
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherList->AircraftList.push_back(ac.GetCallsign());
							}
						}
					}
					else if (direction && entryMinutes > 0) {
						if (menuBar->GetDropDownValue(CMenuBar::DRP_AREASEL) == "EGGX") {
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								// Find out if 30 west is in their flight plan
								if (rte.GetPointPosition(i).m_Longitude == -30.0) {
									// Test flight time
									if (rte.GetPointDistanceInMinutes(i) > 0 && rte.GetPointDistanceInMinutes(i) < 60) {
										// Add if within
										inboundList->AircraftList.push_back(CInboundAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
											rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), true));
										break;
									}
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherList->AircraftList.push_back(ac.GetCallsign());
							}
						}
						else {
							// They are coming from land so check entry points
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								if (CUtils::IsEntryPoint(rte.GetPointName(i), direction) || CUtils::IsExitPoint(rte.GetPointName(i), direction)) {
									// Add if within
									inboundList->AircraftList.push_back(CInboundAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
										rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), true));
									break;
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherList->AircraftList.push_back(ac.GetCallsign());
							}
						}
					}
				}

				// Store whether detailed tags are enabled
				bool detailedEnabled = false;

				// Now we check if all the tags are selected as detailed
				if (menuBar->IsButtonPressed(CMenuBar::BTN_EXT)) {
					detailedEnabled = true; // Set detailed on

					// Unpress detailed if not already
					if (menuBar->IsButtonPressed(CMenuBar::BTN_DETAILED) && !aselDetailed) {
						menuBar->SetButtonState(CMenuBar::BTN_DETAILED, CInputState::INACTIVE);
					}
				}

				// Check if only one is set to detailed
				if (menuBar->IsButtonPressed(CMenuBar::BTN_DETAILED)) {
					if (fp.GetCallsign() == asel) {
						detailedEnabled = true; // Set detailed on
					}

					// Unpress extended if not already
					if (menuBar->IsButtonPressed(CMenuBar::BTN_EXT) && aselDetailed) {
						menuBar->SetButtonState(CMenuBar::BTN_EXT, CInputState::INACTIVE);
					}
				}

				bool ptl = false;
				bool halo = false;
				// Set PTL and HALO if they are on
				if (menuBar->IsButtonPressed(CMenuBar::BTN_PTL)) {
					ptl = true;
				}
				if (menuBar->IsButtonPressed(CMenuBar::BTN_HALO)) {
					halo = true;
				}

				// Get STCA so it can be drawn
				CSTCAStatus stcaStatus(ac.GetCallsign(), "", CConflictStatus::OK, -1, -1); // Create default
				auto idx = CConflictDetection::CurrentSTCA.begin();
				for (idx = CConflictDetection::CurrentSTCA.begin(); idx != CConflictDetection::CurrentSTCA.end(); idx++) {
					if (ac.GetCallsign() == idx->CallsignA || ac.GetCallsign() == idx->CallsignB) {
						stcaStatus = *idx;
						break; // Break for optimisation
					}
				}

				// Draw the tag and target with the information if tags are turned on and within altitude filter
				if (menuBar->IsButtonPressed(CMenuBar::BTN_TAGS)) {
					if (aircraftOnScreen.find(ac.GetCallsign()) == aircraftOnScreen.end()) aircraftOnScreen.insert(make_pair(ac.GetCallsign(), 0));
					auto kv = tagStatuses.find(fp.GetCallsign());
					kv->second.first = detailedEnabled; // Set detailed on
					CAcTargets::RenderTarget(&g, &dc, this, &ac, true, &menuBar->GetToggleButtons(), halo, ptl, &stcaStatus);
					POINT tagPosition = CAcTargets::RenderTag(&g, &dc, this, &ac, &kv->second, direction, &stcaStatus, asel);

					// If tracking dialog open
					if (CAcTargets::OpenTrackingDialog != "" && CAcTargets::OpenTrackingDialog == ac.GetCallsign()) {
						CAcTargets::RenderCoordTagItem(&dc, this, ac.GetCallsign(), tagPosition);
					}

				}
				else {
					if (aircraftOnScreen.find(ac.GetCallsign()) == aircraftOnScreen.end()) aircraftOnScreen.insert(make_pair(ac.GetCallsign(), 0));
					CAcTargets::RenderTarget(&g, &dc, this, &ac, false, &menuBar->GetToggleButtons(), halo, ptl, &stcaStatus);
				}
			}
			else { // If not there, and the aircraft was on the screen, then delete
				if (aircraftOnScreen.find(ac.GetCallsign()) != aircraftOnScreen.end()) aircraftOnScreen.erase(ac.GetCallsign());
			}

			// Select the next target
			ac = GetPlugIn()->RadarTargetSelectNext(ac);
		}


		// Clear ASELs if none of the range/separation tools are pressed
		if (!menuBar->IsButtonPressed(CMenuBar::BTN_PIV)
			&& !menuBar->IsButtonPressed(CMenuBar::BTN_RBL)
			&& !menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
			// Reset ASELs if none enabled
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		/// RENDERING
		// Draw menu bar
		menuBar->RenderBar(&dc, &g, this, asel);

		// Draw lists
		inboundList->RenderList(&g, &dc, this);
		otherList->RenderList(&g, &dc, this);
		//rclList->RenderList(&g, &dc, this);
		conflictList->RenderList(&g, &dc, this);

		// SEP draw
		if (menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
			// If both aircraft selected then draw
			if (aircraftSel1 != "" && aircraftSel2 != "") {
				CConflictDetection::SepTool(&dc, &g, this, aircraftSel1, aircraftSel2);
			}
		}

		// RBL draw
		if (menuBar->IsButtonPressed(CMenuBar::BTN_RBL)) {
			if (aircraftSel1 != "" && aircraftSel2 != "") {
				CConflictDetection::RBLTool(&dc, &g, this, aircraftSel1, aircraftSel2);
			}
		}

		// PIV draw
		if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
			// If both aircraft selected then draw
			if (aircraftSel1 != "" && aircraftSel2 != "") {
				// Render
				CConflictDetection::RenderPIV(&dc, &g, this, aircraftSel1, aircraftSel2);
			}
		}

		// QDM draw
		if (menuBar->IsButtonPressed(CMenuBar::BTN_QDM)) {
			// Check if first position is set
			if (RulerPoint1.m_Latitude != 0.0 && RulerPoint1.m_Longitude != 0.0) {
				// Render
				CCommonRenders::RenderQDM(&dc, &g, this, &RulerPoint1, &RulerPoint2, appCursor->position, &appCursor->latLonPosition);
			}
		}

		// Draw track info window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_TCKINFO)) {
			trackWindow->RenderWindow(&dc, &g, this, menuBar);
		}

		// Draw message window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_MESSAGE)) {
			msgWindow->RenderWindow(&dc, &g, this);
		}

		// Draw flight plan window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_FLIGHTPLAN)) {
			fltPlnWindow->RenderWindow(&dc, &g, this);
		}

		// Draw notepad window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_NOTEPAD)) {
			npWindow->RenderWindow(&dc, &g, this);
		}

		// Finally, reset the clocks if time has been exceeded
		if (fiveSecT >= 5) {
			fiveSecondTimer = clock();
		}
		if (tenSecT >= 10) {
			CLogger::Log(CLogType::NORM, "Recalculating STCA.", "CRadarDisplay::OnRefresh");
			tenSecondTimer = clock();
		}
		if (double twoSecT = (double)(clock() - CAcTargets::twoSecondTimer) / ((double)CLOCKS_PER_SEC) >= 2.2) { // Ac target and tag colours
			CAcTargets::twoSecondTimer = clock();
		}
	}

	// De-allocation
	dc.Detach();
	g.ReleaseHDC(hDC);
	dc.DeleteDC();
}

// Data updates
void CRadarDisplay::OnRadarTargetPositionUpdate(CRadarTarget RadarTarget)
{
	// Check if they are relevant on the screen
	bool filtersDisabled = menuBar->IsButtonPressed(CMenuBar::BTN_ALL);
	if (CUtils::IsAircraftRelevant(this, &RadarTarget, filtersDisabled)) {
		// They are relevant so get the flight plan
		CAircraftFlightPlan* fp = CDataHandler::GetFlightData(RadarTarget.GetCallsign());

		// If not valid then it doesn't exist and we need to make it
		if (!fp->IsValid) {
			// Create it
			CDataHandler::CreateFlightData(this, RadarTarget.GetCallsign());
		}
		else {
			// Flight plan object
			CFlightPlan fpData = GetPlugIn()->FlightPlanSelect(RadarTarget.GetCallsign());

			// The copy made when the aircraft first appeared can be blank if the pilot's plan reached EuroScope later: fill the gaps
			if (fpData.IsValid()) {
				CFlightPlanData fd = fpData.GetFlightPlanData();
				string esType = fd.GetAircraftFPType(), esDep = fd.GetOrigin(), esDest = fd.GetDestination();
				bool changed = false;
				// "NOFP" is what EuroScope shows while it has no data for a field, so it counts as blank
				auto blank = [](const string& v) { return v.empty() || v == "NOFP"; };
				if (blank(fp->Type) && !blank(esType) && esType != fp->Type) { fp->Type = esType; changed = true; }
				if (blank(fp->Depart) && !blank(esDep) && esDep != fp->Depart) { fp->Depart = esDep; changed = true; }
				if (blank(fp->Dest) && !blank(esDest) && esDest != fp->Dest) { fp->Dest = esDest; changed = true; }
				if (changed) {
					fp->Etd = CUtils::ParseZuluTime(false, atoi(fd.GetEstimatedDepartureTime()));
					fp->IsEquipped = CUtils::IsAircraftEquipped(fd.GetRemarks(), fd.GetAircraftInfo(), fd.GetCapibilities());
					fp->WindowRefresh = true;
					CLogger::Log(CLogType::NORM, "Filled in blank flight plan details for " + string(RadarTarget.GetCallsign()) + " from EuroScope.", "CRadarDisplay::OnRadarTargetPositionUpdate");
				}
			}

			// Remember who I am so changes sent to the server carry my callsign
			CDataHandler::ControllerCallsign = GetPlugIn()->ControllerMyself().IsValid() ? GetPlugIn()->ControllerMyself().GetCallsign() : "";

			// Update exit time
			int exitMinutes = fpData.GetSectorExitMinutes();
			if (exitMinutes != -1) {
				fp->ExitTime = exitMinutes;
			}

			// Update selcal code (maybe move into an Update method in DataHandler if I find there is more to update than just the selcal code)
			if (!fp->SelcalManual) {
				string selcal = CUtils::GetSelcalCode(&fpData);
				fp->SELCAL = selcal != "" ? selcal : "N/A";
			}

			// Download the vNAAATS network data on the plane
			try {
				CLogger::Log(CLogType::NORM, "Polling vNAAATS network to update " + string(RadarTarget.GetCallsign()), "CRadarDisplay::OnRadarTargetPositionUpdate");
				CUtils::CNetworkAsyncData* data = new CUtils::CNetworkAsyncData();
				data->Screen = this;
				data->Callsign = fp->Callsign;
				if (!fp->IsFirstUpdate) fp->IsFirstUpdate = true;
				_beginthread(CDataHandler::DownloadNetworkAircraft, 0, (void*)data); // Async
			}
			catch (std::exception & ex) {
				CLogger::DebugLog(this, string("An exception occurred. ") + ex.what());
			}			

			// Timer
			double thirtySecT = (double)(clock() - thirtySecondTimer) / ((double)CLOCKS_PER_SEC);

			// Send mach and FL to FSD from vNAAATS if it is not there
			try {
				if (fp->IsCleared) {
					if (fpData.GetTrackingControllerIsMe()) {
						if (fpData.GetControllerAssignedData().GetAssignedMach() / 10 != stoi(fp->Mach)) {
							// Assign
							fpData.GetControllerAssignedData().SetAssignedMach(stoi(fp->Mach) * 10);
						}
						if (fpData.GetControllerAssignedData().GetFinalAltitude() / 100 != stoi(fp->FlightLevel)) {
							// Assign
							fpData.GetControllerAssignedData().SetFinalAltitude(stoi(fp->FlightLevel) * 100);
						}
					}
					else { // Back the other way, FSD to vNAAATS
						if (fpData.GetControllerAssignedData().GetAssignedMach() / 10 != stoi(fp->Mach)) {
							// Assign
							fp->Mach = to_string(fpData.GetControllerAssignedData().GetAssignedMach() / 10);
						}
						if (fpData.GetControllerAssignedData().GetFinalAltitude() / 100 != stoi(fp->FlightLevel)) {
							// Assign
							fp->FlightLevel = to_string(fpData.GetControllerAssignedData().GetFinalAltitude() / 100);
						}
					}
				}				
			}
			catch (exception & ex) {
				CLogger::DebugLog(this, "An exception occurred when attempting to update FSD: " + string(ex.what()));
			}

			// Update if aircraft is cleared
			if (fp->IsCleared && fpData.GetTrackingControllerIsMe() && thirtySecT >= 30.0) {
				try {
					// Set irrelevant			
					CNetworkFlightPlan* netFP = new CNetworkFlightPlan();
					netFP->Callsign = fp->Callsign;
					netFP->Type = fp->Type;
					netFP->AssignedLevel = stoi(fp->FlightLevel);
					netFP->AssignedMach = stoi(fp->Mach);
					netFP->Track = fp->Track;
					netFP->Departure = fp->Depart;
					netFP->Arrival = fp->Dest;
					netFP->Direction = CUtils::GetAircraftDirection(GetPlugIn()->RadarTargetSelect(fp->Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth());
					netFP->Selcal = fp->SELCAL == "N/A" ? "" : fp->SELCAL;
					netFP->DatalinkConnected = fp->DLStatus == "true" ? true : false;
					netFP->IsEquipped = fp->IsEquipped;
					netFP->State = fp->State;
					netFP->Etd = fp->Etd;
					netFP->Relevant = true;
					netFP->TargetMode = CUtils::GetTargetMode(GetPlugIn()->RadarTargetSelect(fp->Callsign.c_str()).GetPosition().GetRadarFlags());
					netFP->TrackedBy = GetPlugIn()->FlightPlanSelect(fp->Callsign.c_str()).GetTrackingControllerCallsign();
					netFP->TrackedById = GetPlugIn()->FlightPlanSelect(fp->Callsign.c_str()).GetTrackingControllerId();
					netFP->Route = ""; // Initialise
					netFP->RouteEtas = ""; // Initialise

					// Post data to the database
					DWORD activeCode;
					HANDLE hnd = CUtils::GetESProcess();
					GetExitCodeProcess(hnd, &activeCode);
					// Check if the app is still active
					if (activeCode == STILL_ACTIVE && GetPlugIn()->ControllerMyself().IsValid()) {
						if (netFP->Relevant != fp->IsRelevant) {
							fp->IsRelevant = netFP->Relevant;
							CUtils::CNetworkAsyncData* newData = new CUtils::CNetworkAsyncData();
							newData->Screen = this;
							newData->Callsign = fp->Callsign;
							newData->FP = netFP;
							_beginthread(CDataHandler::UpdateNetworkAircraft, 0, (void*)newData); // Async
						}
					}
				}
				catch (std::exception & ex) {
					CLogger::DebugLog(this, string("An exception occurred. ") + ex.what());
				}

				// Reset the clock
				thirtySecondTimer = clock();
			}				
		}
	}
	else { // Not relevant
		// Check if they have a flight plan data object
		CAircraftFlightPlan* fp = CDataHandler::GetFlightData(RadarTarget.GetCallsign());
		if (fp->IsValid) {
			std::async(CLogger::Log, CLogType::WARN, "Aircraft " + string(fp->Callsign) + " outside relevant scope and will be erased. SectorExitMinutes: " + to_string(fp->ExitTime) + ".", "CRadarDisplay::OnRadarTargetPositionUpdate");
			// Ask the draw thread to delete it (deleting here could pull it out from under the screen while it is drawing)
			CDataHandler::QueueDelete(RadarTarget.GetCallsign());
		}
	}
}

void CRadarDisplay::OnFlightPlanDisconnect(CFlightPlan FlightPlan) {
	// If I was tracking it, start the 5 minute window in which it is tracked again when the pilot returns
	if (CDataHandler::Retrack) CDataHandler::MemoDisconnected(FlightPlan.GetCallsign());
	// Close the flight plan window immediately and cancel the ASEL so that we don't get a crash
	if (FlightPlan.GetCallsign() == asel) {
		menuBar->SetButtonState(CMenuBar::BTN_FLIGHTPLAN, CInputState::INACTIVE);
		asel = "";
	}
	// Erase any route drawing
	if (CRoutesHelper::ActiveRoutes.size() != 0 || CRoutesHelper::ActiveRoutes.size() != CRoutesHelper::ActiveRoutes.empty()) {
		int found = -1; // Found flag so we can remove if needed
		for (int i = 0; i < CRoutesHelper::ActiveRoutes.size(); i++) {
			// If the route is currently on the screen
			if (CRoutesHelper::ActiveRoutes[i] == FlightPlan.GetCallsign()) {
				// Set to remove
				found = i;
				break;
			}
		}

		// Erase if the item was found, otherwise add
		if (found != -1) {
			CRoutesHelper::ActiveRoutes.erase(CRoutesHelper::ActiveRoutes.begin() + found);
		}

		// Set any vNAAATS network aircraft to irrelevant
		// Create network object
		try {
			CAircraftFlightPlan* primedPlan = CDataHandler::GetFlightData(FlightPlan.GetCallsign());
			if (primedPlan->IsValid && primedPlan->IsCleared) {
				CNetworkFlightPlan* netFP = new CNetworkFlightPlan();
				netFP->Callsign = primedPlan->Callsign;
				netFP->Type = primedPlan->Type;
				netFP->AssignedLevel = stoi(primedPlan->FlightLevel);
				netFP->AssignedMach = stoi(primedPlan->Mach);
				netFP->Track = primedPlan->Track;
				netFP->Departure = primedPlan->Depart;
				netFP->Arrival = primedPlan->Dest;
				netFP->Direction = CUtils::GetAircraftDirection(GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth());
				netFP->Selcal = primedPlan->SELCAL == "N/A" ? "" : primedPlan->SELCAL;
				netFP->DatalinkConnected = primedPlan->DLStatus == "true" ? true : false;
				netFP->IsEquipped = primedPlan->IsEquipped;
				netFP->State = primedPlan->State;
				netFP->Etd = primedPlan->Etd;
				netFP->Relevant = true;
				netFP->TargetMode = CUtils::GetTargetMode(GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetRadarFlags());
				netFP->TrackedBy = GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign();
				netFP->TrackedById = GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerId();
				netFP->Route = ""; // Initialise
				netFP->RouteEtas = ""; // Initialise

				// Get routes and estimates
				vector<CRoutePosition> rte;
				CRoutesHelper::GetRoute(this, &rte, primedPlan->Callsign);
				for (int i = 0; i < rte.size(); i++) {
					if (i != rte.size() - 1) {
						netFP->Route += rte[i].Fix + " ";
						netFP->RouteEtas += rte[i].Estimate + " ";
					}
					else {
						netFP->Route += rte[i].Fix;
						netFP->RouteEtas += rte[i].Estimate;
					}
				}

				// Post data to the database
				DWORD activeCode;
				HANDLE hnd = CUtils::GetESProcess();
				GetExitCodeProcess(hnd, &activeCode);
				// Check if the app is still active
				if (activeCode == STILL_ACTIVE && GetPlugIn()->ControllerMyself().IsValid()) {
					if (netFP->Relevant != primedPlan->IsRelevant) {
						primedPlan->IsRelevant = netFP->Relevant;
						CUtils::CNetworkAsyncData* newData = new CUtils::CNetworkAsyncData();
						newData->Screen = this;
						newData->Callsign = primedPlan->Callsign;
						newData->FP = netFP;
						_beginthread(CDataHandler::UpdateNetworkAircraft, 0, (void*)newData); // Async
					}
				}
			}
		}
		catch (std::exception & ex) {
			CLogger::DebugLog(this, string("An exception occurred. ") + ex.what());
		}
	}
}

void CRadarDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	// Move inbound list
	if (ObjectType == LIST_INBOUND) {
		inboundList->MoveList(Area);

		// To save
		CUtils::InboundX = Area.left;
		CUtils::InboundY = Area.top;
	}

	// Move other list
	if (ObjectType == LIST_OTHERS) {
		otherList->MoveList(Area);

		CUtils::OthersX = Area.left;
		CUtils::OthersY = Area.top;
	}

	// Move conflict list
	if (ObjectType == LIST_CONFLICT) {
		conflictList->MoveList(Area);

		// To save
		CUtils::ConflictX = Area.left;
		CUtils::ConflictY = Area.top;
	}

	// Move RCLs list
	if (ObjectType == LIST_RCLS) {
		//rclList->MoveList(Area);

		CUtils::RCLX = Area.left;
		CUtils::RCLY = Area.top;
	}

	// Move tag
	if (ObjectType == SCREEN_TAG || ObjectType == SCREEN_TAG_CS) {
		auto kv = tagStatuses.find(sObjectId);
		POINT acPosPix = ConvertCoordFromPositionToPixel(GetPlugIn()->RadarTargetSelect(sObjectId).GetPosition().GetPosition());
		kv->second.second = { Area.left - acPosPix.x, Area.top - acPosPix.y };
	}

	// Move window
	if (ObjectType == WINDOW) {
		if (string(sObjectId) == "TCKINFO")
		trackWindow->MoveWindow(Area);

		if (string(sObjectId) == "FLTPLN")
			fltPlnWindow->MoveWindow(Area);

		if (string(sObjectId) == "MSG")
			msgWindow->MoveWindow(Area);

		if (string(sObjectId) == "NOTEPAD")
			npWindow->MoveWindow(Area);

		CUtils::TrackWindowX = Area.left;
		CUtils::TrackWindowY = Area.top;
	}

	// Move subwindows for flight plan
	if (ObjectType == WIN_FLTPLN) {
		if (atoi(sObjectId) >= 400 && atoi(sObjectId) <= 420) {
			fltPlnWindow->MoveSubWindow(atoi(sObjectId), { Area.left, Area.top });
		}
		if (atoi(sObjectId) >= 500) {
			fltPlnWindow->Scroll(atoi(sObjectId), Pt, mousePointer);
		}
	}

	// Scrolling
	if (ObjectType == WIN_SCROLLBAR) {
		if (string(sObjectId) == "TCKINFO") trackWindow->Scroll(Area, mousePointer);
		if (string(sObjectId) == "520") msgWindow->Scroll(atoi(sObjectId), Pt, mousePointer);
	}

	// Mouse pointer
	mousePointer = Pt;

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area)
{
	mousePointer = Pt;
	// Dropdown
	if (ObjectType == MENBAR) {
		if (atoi(sObjectId) >= 800) {
			menuBar->OnOverDropDownItem(atoi(sObjectId));
		}
	}
	// If it is a message
	else if (ObjectType == ACTV_MESSAGE) {
		msgWindow->SelectedMessage = atoi(sObjectId);
	}
	// If it is a button on the tracking dialog
	if (ObjectType == SCREEN_TAG_CS_BTN) {
		CAcTargets::ButtonStates[sObjectId] = true;
	}
	// TODO: button state reset
	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	if (clock() < g_ignoreClicksUntil) return; // the plugin's own click (Contact Me), not the controller's
	// If menu button
	if (ObjectType == MENBAR) {
		if (Button == BUTTON_RIGHT) { // Toggle buttons
			menuBar->ButtonPress(atoi(sObjectId), Button, this);
		}
		else {
			if (!menuBar->IsButtonPressed(atoi(sObjectId))) {
				// Increase refresh resolution for QDM
				if (atoi(sObjectId) == CMenuBar::BTN_QDM) {
					CLogger::Log(CLogType::NORM, "QDM enabled. Increasing refresh resolution to " + to_string(RefreshResolution) + ".", "CRadarDisplay::OnClickScreenObject");
					RefreshResolution = 0.04;
				}
				menuBar->ButtonPress(atoi(sObjectId), Button, this);
			}
			else {
				// Disable all QDM if it is the QDM button
				if (atoi(sObjectId) == CMenuBar::BTN_QDM) {
					CLogger::Log(CLogType::NORM, "QDM disabled. Resetting refresh resolution to " + to_string(RefreshResolution) + " and clearing lat/lon values.", "CRadarDisplay::OnClickScreenObject");
					RulerPoint1.m_Latitude = 0.0;
					RulerPoint1.m_Longitude = 0.0;
					RulerPoint2.m_Latitude = 0.0;
					RulerPoint2.m_Longitude = 0.0;

					// Reset refresh resolution
					RefreshResolution = 0.2;
				}
				menuBar->ButtonUnpress(atoi(sObjectId), Button, this);
			}
		}
	} else if (ObjectType == WIN_FLTPLN) {
		if (!fltPlnWindow->IsButtonPressed(atoi(sObjectId))) {
			fltPlnWindow->ButtonPress(atoi(sObjectId));
		}
		else {
			fltPlnWindow->ButtonUnpress(atoi(sObjectId));
		}
	}

	// Left button actions
	if (Button == BUTTON_LEFT) {
		// If screen object is a tag
		if (ObjectType == SCREEN_TAG || ObjectType == SCREEN_TAG_CS) {
			// Set FPW to re-instantiate if the aircraft is not the currently ASELed one
			bool newFP = false;
			if (asel != sObjectId && !fltPlnWindow->IsClosed) {
				newFP = true;
			}

			// Set the ASEL
			asel = sObjectId;
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(sObjectId);
			GetPlugIn()->SetASELAircraft(fp);
			CLogger::Log(CLogType::NORM, "Selected aircraft changed to " + asel + ".", "CRadarDisplay::OnClickScreenObject");
			if (!CDataHandler::ControllerCallsign.empty()) _beginthread(CDataHandler::ReportAsel, 0, (void*)new CAselPush{ CDataHandler::ControllerCallsign, asel });
			
			// Re-instantiate FPW if needed
			if (newFP) {
				fltPlnWindow->Instantiate(this, asel);
			}

			// Probing tools
			if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)
				|| menuBar->IsButtonPressed(CMenuBar::BTN_RBL)
				|| menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
				// Make sure flight plans are valid
				if (aircraftSel1 == "" && CDataHandler::GetFlightData(asel)->IsValid) {
					aircraftSel1 = asel;
				}
				else if (aircraftSel2 == "" && aircraftSel1 != asel && CDataHandler::GetFlightData(asel)->IsValid) {
					aircraftSel2 = asel;
				}
			}

			if (fp.GetCallsign() == aircraftSel2 && menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
				CConflictDetection::PIVTool(this, aircraftSel1, aircraftSel2);
			}
		}

		// Flight plan button
		if (atoi(sObjectId) == CMenuBar::BTN_FLIGHTPLAN && menuBar->IsButtonPressed(CMenuBar::BTN_FLIGHTPLAN)) {
			fltPlnWindow->Instantiate(this, asel);
		}

		if (!menuBar->IsButtonPressed(CMenuBar::BTN_FLIGHTPLAN)) {
			fltPlnWindow->IsClosed = true;
		}

		// Qck Look button
		if (atoi(sObjectId) == CMenuBar::BTN_EXT) {
			CLogger::Log(CLogType::NORM, "Extended tags enabled.", "CRadarDisplay::OnClickScreenObject");
			aselDetailed = false;
		}

		// Detailed button
		if (atoi(sObjectId) == CMenuBar::BTN_DETAILED) {
			CLogger::Log(CLogType::NORM, "Detailed tag enabled for currently selected aircraft. Current ASEL: " + asel + ".", "CRadarDisplay::OnClickScreenObject");
			aselDetailed = true;
		}

		// If the button is the PIV button
		if (atoi(sObjectId) == CMenuBar::BTN_PIV) {
			CLogger::Log(CLogType::NORM, "PIV tool activated.", "CRadarDisplay::OnClickScreenObject");
			// Erase RBL (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_RBL)) {
				menuBar->SetButtonState(CMenuBar::BTN_RBL, CInputState::INACTIVE);
				CLogger::Log(CLogType::NORM, "Deactivating RBL tool.", "CRadarDisplay::OnClickScreenObject");
			}

			// Erase SEP (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
				CLogger::Log(CLogType::NORM, "Deactivating SEP tool.", "CRadarDisplay::OnClickScreenObject");
				menuBar->SetButtonState(CMenuBar::BTN_SEP, CInputState::INACTIVE);
			}

			// If PIV already active then clear everything
			if (!CConflictDetection::PIVSeparationStatuses.empty()) {
				CConflictDetection::PIVLocations1.clear();
				CConflictDetection::PIVLocations2.clear();
				CConflictDetection::PIVSeparationStatuses.clear();
				CLogger::Log(CLogType::NORM, "PIV already active. Clearing old PIV.", "CRadarDisplay::OnClickScreenObject");
			}
			// Reset ASELs
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		// If the button is the RBL button
		if (atoi(sObjectId) == CMenuBar::BTN_RBL) {
			CLogger::Log(CLogType::NORM, "RBL tool activated.", "CRadarDisplay::OnClickScreenObject");
			// Erase PIV (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
				CLogger::Log(CLogType::NORM, "Deactivating PIV tool.", "CRadarDisplay::OnClickScreenObject");
				menuBar->SetButtonState(CMenuBar::BTN_PIV, CInputState::INACTIVE);
			}

			// Erase SEP (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
				CLogger::Log(CLogType::NORM, "Deactivating SEP tool.", "CRadarDisplay::OnClickScreenObject");
				menuBar->SetButtonState(CMenuBar::BTN_SEP, CInputState::INACTIVE);
			}

			// Reset ASELs
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		// If the button is the SEP button
		if (atoi(sObjectId) == CMenuBar::BTN_SEP) {
			CLogger::Log(CLogType::NORM, "SEP tool activated.", "CRadarDisplay::OnClickScreenObject");
			// Erase RBL (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_RBL)) {
				CLogger::Log(CLogType::NORM, "Deactivating RBL tool.", "CRadarDisplay::OnClickScreenObject");
				menuBar->SetButtonState(CMenuBar::BTN_RBL, CInputState::INACTIVE);
			}

			// Erase PIV (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
				CLogger::Log(CLogType::NORM, "Deactivating PIV tool.", "CRadarDisplay::OnClickScreenObject");
				menuBar->SetButtonState(CMenuBar::BTN_PIV, CInputState::INACTIVE);
			}

			// Reset ASELs
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		// If a menu text entry
		if (ObjectType == ALTFILT_TEXT) {
			// If the low altitude filter
			if (string(sObjectId) == "ALTFILT_LOW") {
				GetPlugIn()->OpenPopupEdit(Area, FUNC_ALTFILT_LOW, "");
			}
			// If the high altitude filter
			if (string(sObjectId) == "ALTFILT_HIGH") {
				GetPlugIn()->OpenPopupEdit(Area, FUNC_ALTFILT_HIGH, "");
			}
		}

		if (ObjectType == MENBAR) {
			if (string(sObjectId) == to_string(menuBar->TXT_SEARCH))
				GetPlugIn()->OpenPopupEdit(Area, atoi(sObjectId), "");
		}

		// If a flight plan window text entry
		if (ObjectType == WIN_FLTPLN) {
			if (fltPlnWindow->IsTextInput(atoi(sObjectId)) && (fltPlnWindow->GetInputState(atoi(sObjectId)) != CInputState::DISABLED || fltPlnWindow->GetInputState(atoi(sObjectId)) != CInputState::INACTIVE)) {
				GetPlugIn()->OpenPopupEdit(Area, atoi(sObjectId), fltPlnWindow->GetTextValue(atoi(sObjectId)).c_str());
			}
		}

		// If it is a hide show button for a list
		if (ObjectType == LIST_INBOUND) {
			if (string(sObjectId) == "HIDESHOW") {
				inboundList->HideShowButton = inboundList->HideShowButton == true ? false : true;
				CLogger::Log(CLogType::NORM, "Toggling Inbound list. Current status: " + to_string(inboundList->HideShowButton), "CRadarDisplay::OnClickScreenObject");
			}
		}
		if (ObjectType == LIST_OTHERS) {
			if (string(sObjectId) == "HIDESHOW") {
				otherList->HideShowButton = otherList->HideShowButton == true ? false : true;
				CLogger::Log(CLogType::NORM, "Toggling Other list. Current status: " + to_string(inboundList->HideShowButton), "CRadarDisplay::OnClickScreenObject");
			}
		}

		// If it is a message
		if (ObjectType == ACTV_MESSAGE) {
			msgWindow->SelectedMessage = atoi(sObjectId);
		}

		// If a coordination
		if (ObjectType == WIN_FLTPLN_TSFR) {
			if (GetPlugIn()->FlightPlanSelect(fltPlnWindow->primedPlan->Callsign.c_str()).GetTrackingControllerIsMe() &&
				string(GetPlugIn()->FlightPlanSelect(fltPlnWindow->primedPlan->Callsign.c_str()).GetHandoffTargetControllerCallsign()) == "") {
				fltPlnWindow->selectedAuthority = sObjectId;
			}
		}
	}

	if (Button == BUTTON_RIGHT) {
		if (ObjectType == SCREEN_TAG || ObjectType == SCREEN_TAG_CS) {
			/// Set route drawing
			// Make sure flight plan exists otherwise it will crash, and also that they aren't PIV aircraft
			if (CDataHandler::GetFlightData(string(sObjectId))->IsValid && string(sObjectId) != aircraftSel1 && string(sObjectId) != aircraftSel2) {
				int found = -1; // Found flag so we can remove if needed
				for (int i = 0; i < CRoutesHelper::ActiveRoutes.size(); i++) {
					// If the route is currently on the screen
					if (CRoutesHelper::ActiveRoutes[i] == sObjectId) {
						// Set to remove
						found = i;
						break;
					}
				}

				// Erase if the item was found, otherwise add
				if (found != -1) {
					CLogger::Log(CLogType::NORM, "Erasing route draw for: " + string(sObjectId), "CRadarDisplay::OnClickScreenObject");
					CRoutesHelper::ActiveRoutes.erase(CRoutesHelper::ActiveRoutes.begin() + found);
					g_routeExpiry.erase(string(sObjectId));
				}
				else {
					CLogger::Log(CLogType::NORM, "Enabling route draw for: " + string(sObjectId), "CRadarDisplay::OnClickScreenObject");
					CRoutesHelper::ActiveRoutes.push_back(string(sObjectId));
					if (CDataHandler::RouteSeconds > 0) g_routeExpiry[string(sObjectId)] = clock() + (clock_t)CDataHandler::RouteSeconds * CLOCKS_PER_SEC;   // erased by itself after routeseconds
				}
			}
		}
	}

	CLogger::Log(CLogType::NORM, "Screen refresh forced.", "CRadarDisplay::OnClickScreenObject");
	RequestRefresh();
}

void CRadarDisplay::OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// Track info window
	if (ObjectType == WIN_TCKINFO) {
		trackWindow->ButtonDown(atoi(sObjectId));
	}

	// Flight plan window
	if (ObjectType == WIN_FLTPLN) {
		fltPlnWindow->ButtonDown(atoi(sObjectId));
	}

	// Note pad window
	if (ObjectType == WIN_NOTEPAD) {
		npWindow->ButtonDown(atoi(sObjectId));
	}

	// Message window
	if (ObjectType == WIN_MSG) {
		msgWindow->ButtonDown(atoi(sObjectId));
	}

	// Menu bar
	if (ObjectType == MENBAR) {
		menuBar->ButtonDown(atoi(sObjectId));
	}

	// Refresh
	CLogger::Log(CLogType::NORM, "Screen refresh forced.", "CRadarDisplay::OnButtonDownScreenObject");
	RequestRefresh();
}

void CRadarDisplay::OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// Track info window
	if (ObjectType == WIN_TCKINFO) {
		if (atoi(sObjectId) == CTrackInfoWindow::BTN_CLOSE) {
			// Close window if the close button
			menuBar->SetButtonState(CMenuBar::BTN_TCKINFO, CInputState::INACTIVE);
		}
		trackWindow->ButtonUp(atoi(sObjectId));
	}

	// Flight plan window
	if (ObjectType == WIN_FLTPLN) {
		if (atoi(sObjectId) == CFlightPlanWindow::BTN_CLOSE) {
			// Close window if the close button
			menuBar->SetButtonState(CMenuBar::BTN_FLIGHTPLAN, CInputState::INACTIVE);
		}
		if (atoi(sObjectId) < 200) {
			fltPlnWindow->ButtonUp(atoi(sObjectId), this);
		}
	}

	// Note pad window
	if (ObjectType == WIN_NOTEPAD) {
		if (atoi(sObjectId) == CNotePad::BTN_CLOSE) {
			// Close window if the close button
			menuBar->SetButtonState(CMenuBar::BTN_NOTEPAD, CInputState::INACTIVE);
		}
		npWindow->ButtonUp(atoi(sObjectId));
	}

	// Message window
	if (ObjectType == WIN_MSG) {
		if (atoi(sObjectId) == CMessageWindow::BTN_CLOSE) {
			// Close window if the close button
			menuBar->SetButtonState(CMenuBar::BTN_MESSAGE, CInputState::INACTIVE);
		}
		msgWindow->ButtonUp(atoi(sObjectId));
	}

	// Menu bar
	if (ObjectType == MENBAR) {
		// Clear active routes
		if (atoi(sObjectId) == CMenuBar::BTN_RTEDEL) {
			CRoutesHelper::ActiveRoutes.clear();
		}
		if (atoi(sObjectId) == CMenuBar::BTN_AUTOTAG) {
			tagStatuses.clear();
		}
		menuBar->ButtonDown(atoi(sObjectId));
	}

	// Refresh
	CLogger::Log(CLogType::NORM, "Screen refresh forced.", "CRadarDisplay::OnButtonUpScreenObject");
	RequestRefresh();
}

void CRadarDisplay::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
	// Set low alt filter
	if (FunctionId == FUNC_ALTFILT_LOW) {
		// Validation (range & type)
		// Check if it is a string
		bool isNumber = true;
		for (int i = 0; i < strlen(sItemString); i++) { // Check if string
			if (!isdigit(sItemString[i])) isNumber = false;
		}
		if (isNumber && (atoi(sItemString) < 1000 && atoi(sItemString) > 0)) {
			CUtils::AltFiltLow = atoi(sItemString); // Return if in range
		}
	}
	// Set high alt filter
	if (FunctionId == FUNC_ALTFILT_HIGH) {
		// Validation (range & type)
		bool isNumber = true;
		for (int i = 0; i < strlen(sItemString); i++) { // Check if string
			if (!isdigit(sItemString[i])) isNumber = false;
		}
		if (isNumber && (atoi(sItemString) < 999 && atoi(sItemString) > 0)) {
			CUtils::AltFiltHigh = atoi(sItemString); // Return if in range
		}
	}
	if (FunctionId == CMenuBar::TXT_SEARCH) {
		string itemString = string(sItemString);
		string value;
		for (int i = 0; i < itemString.size(); i++) {
			char c = itemString[i];
			if (isalpha(c))
				c = toupper(c);
			value += c;
		}
		if (GetPlugIn()->RadarTargetSelect(value.c_str()).IsValid()) {
			CAcTargets::SearchedAircraft = value;
			CAcTargets::fiveSecondTimer = clock();
			GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(value.c_str()));
		}
	}

	// If it is a flight plan window text input
	if (fltPlnWindow->IsTextInput(FunctionId)) {
		fltPlnWindow->SetTextValue(this, FunctionId, string(sItemString));
	}
}

void CRadarDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// If it is a message
	if (ObjectType == ACTV_MESSAGE) {
		msgWindow->ButtonDoubleClick(this, atoi(sObjectId), fltPlnWindow);
	}
	// If it concerns the flight plan window
	if (ObjectType == WIN_FLTPLN) {
		fltPlnWindow->ButtonDoubleClick(atoi(sObjectId));
	}
	// If it is an aircraft callsign
	if (ObjectType == SCREEN_TAG_CS) {
		//CAcTargets::OpenTrackingDialog = sObjectId;
	}
}

void CRadarDisplay::OnAsrContentToBeSaved(void)
{
	// Buttons
	CUtils::TagsEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_TAGS) ? true : false;
	CUtils::GridEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_GRID) ? true : false;
	CUtils::OverlayEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_OVERLAYS) ? true : false;
	CUtils::QckLookEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_QCKLOOK) ? true : false; // TODO: Change this to Ext
	
	// Save
	CUtils::SavePluginData(this);
	CLogger::Log(CLogType::NORM, "Saving plugin data...", "CRadarDisplay::OnAsrContentToBeSaved");
}

void CRadarDisplay::OnAsrContentLoaded(bool Loaded)
{
	if (!Loaded)
		return;

	CLogger::Log(CLogType::NORM, "ASR loaded, reading saved settings.", "CRadarDisplay::OnAsrContentLoaded");
	// Load the plugin data (a bad saved value must never take EuroScope down)
	try {
		CUtils::LoadPluginData(this);
	}
	catch (...) {
		CLogger::Log(CLogType::EXC, "Saved settings in the ASR could not be read - using defaults.", "CRadarDisplay::OnAsrContentLoaded");
	}
	CLogger::Log(CLogType::NORM, "Saved settings done, starting up.", "CRadarDisplay::OnAsrContentLoaded");

	// Populate it
	try {
		PopulateProgramData();
	}
	catch (const std::exception& e) {
		CLogger::Log(CLogType::EXC, string("Start-up failed: ") + e.what(), "CRadarDisplay::OnAsrContentLoaded");
	}
	catch (...) {
		CLogger::Log(CLogType::EXC, "Start-up failed (unknown error).", "CRadarDisplay::OnAsrContentLoaded");
	}
}

// TODO: Break into individual methods or create ScreenFunctions class/namespace
void CRadarDisplay::CursorStateUpdater(void* args)
{
	// Pointer to cursor
	CAppCursor* cursor = (CAppCursor*)args;

	try {
		// Timer
		clock_t hundredmsTimer = clock();
		clock_t refreshTimer = clock();

		// Monitor information for proper positioning
		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(MONITORINFO);

		// Get the process information - infinite loop in separate thread = bad unless you manually break the loop on application close
		DWORD activeCode;
		while (!cursor->isESClosed) {
			HANDLE hnd = CUtils::GetESProcess();
			GetExitCodeProcess(hnd, &activeCode);
			// Check if the app is still active
			if (cursor->isESClosed || activeCode != STILL_ACTIVE) {
				CLogger::Log(CLogType::NORM, "ES quitting. Thread destroyed.");
				break; // Break if not
			}

			// Ok so the app is still active let's get the cursor data
			if (((double)(clock() - hundredmsTimer) / ((double)CLOCKS_PER_SEC)) >= 0.04) { // Greater than or equal to 40ms
				// Get cursor position (only if previous cursor position is inside the radar area)
				bool isCursorInsideRadarArea = false;
				CRect radarArea = cursor->screen->GetRadarArea();

				// Get the position and monitor in which the point lies
				GetCursorPos(&cursor->position);

				// GetCursorPos is in SCREEN pixels, but EuroScope's pixel <-> lat/lon conversion
				// works in the radar window's own client pixels. Convert (this was why the readout
				// showed the wrong longitude, e.g. E30 over 30W, whenever the window was not at 0,0).
				HWND underCursor = WindowFromPoint(cursor->position);
				if (underCursor != NULL && ScreenToClient(underCursor, &cursor->position)) {
					// done: cursor->position is now relative to the radar window
				}
				else {
					// Fallback (old behaviour): wrap to the monitor size
					HMONITOR monitor = MonitorFromPoint(cursor->position, MONITOR_DEFAULTTONEAREST);
					GetMonitorInfo(monitor, &monitorInfo);
					int monResX = abs(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
					int monResY = abs(monitorInfo.rcMonitor.top - monitorInfo.rcMonitor.bottom);
					if (monResX > 0 && monResY > 0) {
						cursor->position.x = ((cursor->position.x % monResX) + monResX) % monResX;
						cursor->position.y = ((cursor->position.y % monResY) + monResY) % monResY;
					}
				}

				// Get button presses
				bool leftBtnPressed = (GetAsyncKeyState(VK_LBUTTON) & (1 << 15)) != 0;
				bool rightBtnPressed = (GetAsyncKeyState(VK_RBUTTON) & (1 << 15)) != 0;

				// Check if cursor inside radar area
				if (cursor->position.x > radarArea.left&&
					cursor->position.x < radarArea.right &&
					cursor->position.y > radarArea.top + MENBAR_HEIGHT && // We want *our* radar screen so we add the vNAAATS menu bar height
					cursor->position.y < radarArea.bottom) {
					isCursorInsideRadarArea = true;
				}

				// Remember which window is the radar (ours, under the mouse, mouse inside the radar area)
				if (isCursorInsideRadarArea && underCursor != NULL) {
					DWORD wpid = 0; GetWindowThreadProcessId(underCursor, &wpid);
					if (wpid == GetCurrentProcessId()) g_radarWnd = underCursor;
				}

				// Lat/lon position
				cursor->latLonPosition = cursor->screen->ConvertCoordFromPixelToPosition(cursor->position);

				// Check button presses
				if ((!leftBtnPressed && !rightBtnPressed) || !isCursorInsideRadarArea) {
					cursor->button = 0;
				}

				/// Events!
				// On left click
				if (leftBtnPressed && cursor->button == 0) {
					// QDM button
					if (cursor->screen->menuBar->IsButtonPressed(CMenuBar::BTN_QDM) && isCursorInsideRadarArea) {
						bool pointSet = false;
						// Activate QDM, first check if first ruler point already filled
						if (cursor->screen->RulerPoint1.m_Latitude == 0.0 && cursor->screen->RulerPoint1.m_Longitude == 0.0) {
							// It isn't filled so set it
							cursor->screen->RulerPoint1.m_Latitude = cursor->latLonPosition.m_Latitude;
							cursor->screen->RulerPoint1.m_Longitude = cursor->latLonPosition.m_Longitude;

							// So that we don't accidently set the 2nd point at the same time
							pointSet = true;
						}
						// Check the 2nd point
						if (!pointSet && (cursor->screen->RulerPoint2.m_Latitude == 0.0 && cursor->screen->RulerPoint2.m_Longitude == 0.0)) {
							// It isn't filled so set it
							cursor->screen->RulerPoint2.m_Latitude = cursor->latLonPosition.m_Latitude;
							cursor->screen->RulerPoint2.m_Longitude = cursor->latLonPosition.m_Longitude;

							// So we dont cancel the QDM automatically
							pointSet = true;
						}

						// Both points are down so we need to reset
						if (!pointSet && (cursor->screen->RulerPoint2.m_Latitude != 0.0 && cursor->screen->RulerPoint2.m_Longitude != 0.0)) {
							cursor->screen->RulerPoint1.m_Latitude = 0.0;
							cursor->screen->RulerPoint1.m_Longitude = 0.0;
							cursor->screen->RulerPoint2.m_Latitude = 0.0;
							cursor->screen->RulerPoint2.m_Longitude = 0.0;
						}
					}

					// Set the button so the event doesn't fire again
					cursor->button = 2;
				}
				// On right click
				if (rightBtnPressed && cursor->button == 0) {

					// Set the button so the event doesn't fire again
					cursor->button = 1;
				}

				// Reset clock
				hundredmsTimer = clock();
			}

			// Call refresh sequence more often
			if (((double)(clock() - refreshTimer) / ((double)CLOCKS_PER_SEC)) >= cursor->screen->RefreshResolution) {
				// Refresh the radar screen
				cursor->screen->RequestRefresh();

				// Reset clock
				refreshTimer = clock();
			}

		}

		// Check if the app is still active
		if (cursor->isESClosed) {
			CLogger::Log(CLogType::NORM, "ES quitting. Thread destroyed.");
		}

		
	}
	catch (exception & ex) {
		CLogger::DebugLog(cursor->screen, string("An exception occurred in the CursorStateUpdater. ") + ex.what());
		CLogger::Log(CLogType::ERR, "An error occurred. \nCursor position: " + to_string(cursor->latLonPosition.m_Latitude) + "," + to_string(cursor->latLonPosition.m_Longitude)
			+ "\nRefresh Resolution: " + to_string(cursor->screen->RefreshResolution) + string("\nVerbose details: ") + ex.what(), "CRadarDisplay::CursorStateUpdater");
	}

	// Clean up and return
	delete args;

	return;
}