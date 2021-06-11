#pragma once
#include "EuroScopePlugIn.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Utils.h"

using namespace std;
using namespace EuroScopePlugIn;

class CLogger
{
	public:
		// Generate the log file
		static void InstantiateLogFile();

		// Log
		static void Log(vector<string> text, CLogType type);

		// Get log prefix (ID + date and time)
		static void GeneratePrefix(CLogType type);
		
	private:
		// The file to log to
		static string logFilePath;
		
		// This writes
		static void WriteLine(string logString);
};

