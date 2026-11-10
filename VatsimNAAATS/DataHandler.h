#pragma once
#include "vector"
#include "Structures.h"
#include "EuroScopePlugIn.h"
#include "Overlays.h"
#include "Utils.h"
#include "Logger.h"
#include <json.hpp>
#include <mutex>
#include <map>
#include <ctime>
#include <atomic>

using namespace std;
using namespace EuroScopePlugIn;

struct CTrackedPush { string Me, List; }; // the aircraft I am tracking, reported to the server
struct CAselPush { string Me, Callsign; }; // reported to the server when the selected aircraft changes

class CDataHandler
{
	public:
	// Check plugin version number
	static int CheckPluginVersion(CPlugIn* plugin);

	// Download nat track data
	static int PopulateLatestTrackData(CPlugIn* plugin);

	// Get flight data
	static CAircraftFlightPlan* GetFlightData(string callsign);
	static void GetFlightData(string callsign, CAircraftFlightPlan& fp);

	// Update a flight data object
	static int UpdateFlightData(CRadarScreen* screen, string callsign, bool updateRoute);

	// Create a new flight data object
	static int CreateFlightData(CRadarScreen* screen, string callsign);

	// Deletes a flight data object out of the flights map
	static int DeleteFlightData(string callsign);

	// Set route
	static int SetRoute(string callsign, vector<CWaypoint>* route, string track, CAircraftFlightPlan* copiedPlan = nullptr);

	/// vNAAATS network methods
	// Download aircraft data (single)
	static void DownloadNetworkAircraft(void* args);
	static void QueueDelete(const string& callsign); // any thread: ask for flight data to be erased
	static std::vector<string> TakeQueuedDeletes(); // draw thread only
	static void ApplyPendingNetwork(CRadarScreen* screen); // draw thread: applies the downloaded data

	// Download all aircraft data
	static void GetAllNetworkAircraft();

	// Post new aircraft data
	static void PostNetworkAircraft(void* args);

	// Update aircraft data
	static void UpdateNetworkAircraft(void* args);

	// Read vnaaats.cfg (server address, API key, squawk option) from the DLL folder
	static void LoadSettings();
	static string SettingsUrl; // server + /PluginSettings?code=KEY: options chosen on the FDD Settings box
	static long long SettingsStamp; // settingsstamp= in vnaaats.cfg: the version of the FDD settings already written to it
	static volatile long SettingsBusy; // 1 while a fetch is running
	static volatile long SettingsMessagePending; // 1 = tell the controller to restart EuroScope
	static void FetchSettings(void* args); // writes changed FDD settings into vnaaats.cfg (they apply when EuroScope is restarted)
	static bool BackgroundType; // backgroundtype=1 (experimental, default off): type into EuroScope without bringing it to the front
	static int RouteSeconds; // routeseconds=10 (default): a route drawn with a right click on a tag is erased after this many seconds (0 = stays until clicked again)
	static int ContactButton; // contactbutton=left|middle|right: the mouse button the plugin clicks the aircraft with (match your EuroScope ASEL button)
	static bool AutoClick; // autoclick=1 (default): after typing .contactme, click the aircraft for you
	static bool Highlight; // highlight=1 (default): circle the aircraft in white when it is clicked on the FDD
	static bool AutoType; // autotype=1: type .selcal / .contactme / .msg into EuroScope when the FDD asks
	static bool SquawkOnClearance; // assign 2000 when I start tracking an aircraft
	static string ControllerCallsign; // my EuroScope callsign, sent with changes
	static string DownloadUrl; // download page (server + /download)
	static string FddUrl; // flight data display address (server + /fdd)
	static string ScratchUrl; // server + /PluginScratch?code=KEY
	static string SyncScratch(CFlightPlan& esFp); // two-way sync of the scratchpad note between EuroScope and the server; returns the note
	static void PushScratch(void* args);
	static bool Retrack; // retrack=1 (default): after a crash / reconnect, track the aircraft I had again
	static string TrackedUrl; // server + /PluginTracked?code=KEY
	static string RetrackUrl; // server + /PluginRetrack?code=KEY
	static void ReportTracked(void* args); // tells the server which aircraft I am tracking
	static void FetchRetrack(void* args); // asks the server which aircraft I had before the disconnection
	static vector<string> TakeRetrack(); // callsigns fetched by FetchRetrack
	static volatile long RetrackFetched; // 1 when FetchRetrack has finished
	static string AselUrl; // server + /PluginAsel?code=KEY
	static string RouteUrl; // server + /PluginRoute?code=KEY (the filed route of an aircraft nobody has handled yet)
	static void ReportAsel(void* args); // tells the server which aircraft was clicked in EuroScope
	static string UrlEncode(const string& value); // percent-encode a query value
	// Commands sent from the web flight data display (track / untrack / transfer)
	struct CRemoteCommand {
		int Id = 0;
		string Type, Callsign, Level, Mach, Selcal, Target, Freq, Text, Plan;
	};
	// Aircraft I track, remembered so that if the pilot disconnects and comes back within 5 minutes I can track them again with the same data
	struct CTrackMemo {
		string Level, Mach, Selcal;
		time_t DisconnectedAt = 0; // 0 = still connected
	};
	static void MemoTracked(const string& callsign, const string& level, const string& mach, const string& selcal); // I track it now: remember FL / Mach / SELCAL
	static void MemoDisconnected(const string& callsign); // its pilot has gone: start the 5 minute window (only if I was tracking it)
	static void MemoForget(const string& callsign);
	static vector<std::pair<string, CTrackMemo>> MemoSnapshot();
	static void QueueLocalCommand(const CRemoteCommand& cmd); // run a command as if the web display had sent it (Id 0: no server answer)
	static void PollRemoteCommands(void* args); // async: asks the server for waiting commands
	static void ReportRemoteCommand(void* args); // async: tells the server how a command went
	static vector<CRemoteCommand> TakeRemoteCommands(); // commands collected so far (called from the EuroScope thread)
	static void ReportRemoteResult(int id, bool ok, const string& msg);
	static std::atomic<bool> RemotePollBusy;
	static clock_t RemoteLastPoll;
	static void PostCpdlcHandoff(void* args); // tell the server a handoff was started (it messages the pilot if on CPDLC)
	
	private:
		// Methods
		static int GetTrackSource(CPlugIn* plugin); // Event tracks or not

		// Version URL
		static const string PluginVersion;       // GitHub copy, used only if our own server cannot be reached
		static string VersionUrl;                // our own server: <server>/pluginversion (plain text)

		// NAT Track URL
		static string TrackURL;
		static map<string, CAircraftFlightPlan> flights;

		// vNAAATS API Links
		static string TrackSource;
		static string GetSingleAircraft;
		static string FlightDataUpdate;
		static string PostSingleAircraft;
		static string CpdlcHandoffUrl;
		static string CommandsUrl;
		static string CommandDoneUrl;
		static std::mutex remoteLock;
		struct CPendingNet { CNetworkFlightPlan net; string scratch; bool routeOnly = false; string routeText; string routeTrack; }; // routeOnly: the aircraft has no saved record, only the filed route from the server
		static std::mutex deleteLock;
		static std::vector<string> deleteQueue;
		static std::mutex pendingLock;
		static std::map<string, CPendingNet> pending;
		static vector<CRemoteCommand> remoteQueue;
};
