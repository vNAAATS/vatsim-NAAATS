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
		static void Log(string text, CLogType type);

		// Generate the log file
		static void InstantiateLogFile();
		
	private:
		// The file to log to
		static string logFilePath;
		
		// Get log prefix (ID + date and time)
		static string GeneratePrefix(CLogType type);
};

