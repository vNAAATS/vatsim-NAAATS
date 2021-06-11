#include "pch.h"
#include "Logger.h"

string CLogger::logFilePath = "";

void CLogger::Log(string text, CLogType type) {
	// Prefix
	string prefix = GeneratePrefix(type);

	// Open, write then close
	ofstream log;
	log.open(logFilePath.c_str());
	log << prefix.c_str() << text.c_str() << "\n";
	log.close();
}

void CLogger::InstantiateLogFile() {
	
	// Date string
	time_t now = time(0);
	tm* date = gmtime(&now);
	string strDate;
	strDate += to_string(date->tm_hour);
	strDate += to_string(date->tm_min);
	strDate += to_string(date->tm_sec);
	strDate += to_string(date->tm_mday);
	strDate += to_string(date->tm_mon + 1);
	strDate += to_string(1900 + date->tm_year);

	// Generate a new file name
	logFilePath = CUtils::DllPath + "\\" + "vNAAATS_" + strDate + ".log";

	// Get rudimentary system information
	CPluginSysInfo info;
	
	// Build output
	string output;
	output += "***** LOG INITIALISED FOR vNAAATS VERSION ";
	output += PLUGIN_VERSION;
	output += " *****\n";
	output += "COPYRIGHT ";
	output += PLUGIN_COPYRIGHT;
	output += "\n\n OS: ";
	output += info.OSVersion;
	output += "\nRAM (MB): ";
	output += info.InstalledMemoryMB;

	// Write
	Log(output, CLogType::INIT);
}

string CLogger::GeneratePrefix(CLogType type) {
	// This will store the prefix
	string prefix = "[";

	// Date string (incl. milliseconds)
	SYSTEMTIME t;
	GetSystemTime(&t);
	time_t now = time(0);
	tm* date = gmtime(&now);
	string strDate;
	strDate += to_string(date->tm_mday);
	strDate += "-";
	strDate += to_string(date->tm_mon + 1);
	strDate += "-";
	strDate += to_string(1900 + date->tm_year);
	strDate += " ";
	strDate += to_string(t.wHour);
	strDate += ":";
	strDate += to_string(t.wMinute);
	strDate += ":";
	strDate += to_string(t.wSecond);
	strDate += ".";
	strDate += to_string(t.wMilliseconds);

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
	prefix += strDate;
	prefix += "] ";

	// Return the prefix
	return prefix;
}