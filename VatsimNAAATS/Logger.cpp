#include "pch.h"
#include "Logger.h"

string CLogger::logFilePath = "";
bool CLogger::initialised = false;
bool CLogger::initialisedAc = false;

void CLogger::Log(CLogType type, string text, string invokedBy) {
	if (DEBUG_MODE) {
		// Prefix
		string prefix = GeneratePrefix(type);

		// Format invokedBy if not an empty string
		invokedBy = invokedBy != "" ? "[" + invokedBy + "] " : "";

		// Open, write then close
		ofstream log;
		if (!initialised) { // Overwrite first time
			log.open(logFilePath.c_str());
			initialised = true;
		}
		else { // Append thereafter
			log.open(logFilePath.c_str(), std::ios_base::app | std::ios_base::out);
		}
		log << prefix.c_str() << invokedBy << text.c_str() << "\n";
		log.close();
	}
}

void CLogger::DebugLog(CRadarScreen* screen, string text) {
	screen->GetPlugIn()->DisplayUserMessage("vNAAATS", "Log", text.c_str(), true, true, true, true, true);
}

void CLogger::LogAircraftDebugInfo(string text) // Debug logger for some raw AC data
{
	// Open, write then close
	ofstream log;
	if (!initialisedAc) { // Overwrite first time
		log.open(CUtils::DllPath + "\\raw_ac_data.txt");
		initialisedAc = true;
	}
	else { // Append thereafter
		log.open(CUtils::DllPath + "\\raw_ac_data.txt", std::ios_base::app | std::ios_base::out);
	}
	log << text.c_str() << "\n";
	log.close();
}

void CLogger::InstantiateLogFile() {
	// Instantiate file path
	logFilePath = CUtils::DllPath + "\\vNAAATS.log";

	// Get system memory size in MB
	uint64_t memKB;
	GetPhysicallyInstalledSystemMemory(&memKB);
	long installedMemoryMB = memKB / 1000;
	
	// Build output
	string output;
	output += "***** LOG INITIALISED FOR vNAAATS VERSION ";
	output += PLUGIN_VERSION;
	output += " *****\n";
	output += "COPYRIGHT ";
	output += PLUGIN_COPYRIGHT;
	output += "\nRAM (MB): ";
	output += to_string(installedMemoryMB);
	output += "\nAlpha?:";
	output += IS_ALPHA ? " TRUE" : " FALSE";
	output += "\n----------------------------------";

	// Write
	Log(CLogType::INIT, output);
}

string CLogger::GeneratePrefix(CLogType type) {
	// This will store the prefix
	string prefix = "[";

	// Date string (incl. milliseconds)
	SYSTEMTIME t;
	GetSystemTime(&t);
	string strTime;
	strTime += CUtils::PadWithZeros(2, t.wHour);
	strTime += ":";
	strTime += CUtils::PadWithZeros(2, t.wMinute);
	strTime += ":";
	strTime += CUtils::PadWithZeros(2, t.wSecond);
	strTime += ".";
	strTime += to_string(t.wMilliseconds);

	// Begin
	switch (type) {
	case CLogType::INIT:
		prefix += "INIT ";
		break;
	case CLogType::NORM:
		prefix += "LOG ";
		break;
	case CLogType::WARN:
		prefix += "WARN ";
		break;
	case CLogType::ERR:
		prefix += "ERR ";
		break;
	case CLogType::EXC:
		prefix += "EXC ";
		break;
	}

	// Append date/time and closing bracket
	prefix += strTime;
	prefix += "] ";

	// Return the prefix
	return prefix;
}