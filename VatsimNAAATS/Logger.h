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

		static void LogAircraftDebugInfo(string text);

		// Generate the log file
		static void InstantiateLogFile();
		
	private:
		// The file to log to
		static string logFilePath;
		
		// Get log prefix (ID + date and time)
		static string GeneratePrefix(CLogType type);

		// To append lines after initialisation instead of overwriting
		static bool initialised;
		static bool initialisedAc;
};

