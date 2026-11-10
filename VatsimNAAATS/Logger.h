#pragma once
#include "EuroScopePlugIn.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <windows.h>
#include "Utils.h"

using namespace std;
using namespace EuroScopePlugIn;

class CLogger
{
	public:
		// Log a message
		static void Log(CLogType type, string text, string invokedBy = "");

		// Log to chat
		static void DebugLog(CRadarScreen* screen, string text);

		static void LogAircraftDebugInfo(string text);

		// Generate the log file
		static void InstantiateLogFile();

		// Admin log: a copy of the log lines is sent to the vNAAATS server (see vnaaats.cfg: remotelog=0/1/2)
		static string RemoteUrl;                  // full POST address incl. ?code=KEY, empty = not sending
		static int RemoteMode;                    // 0 off, 1 warnings and errors only (default), 2 everything
		static volatile long RemoteDetail;        // the admin asked for full detail from this controller
		static void MaybeFlushRemote();           // call often (draw thread); starts a background send when due
		static void FlushRemoteThread(void* args);
		
	private:
		// The file to log to
		static string logFilePath;
		
		// Get log prefix (ID + date and time)
		static string GeneratePrefix(CLogType type);

		// To append lines after initialisation instead of overwriting
		static bool initialised;
		static bool initialisedAc;

		static void QueueRemote(CLogType type, const string& line);
		static string Redact(string text);
};

