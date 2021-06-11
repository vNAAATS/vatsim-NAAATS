#include "pch.h"
#include "Logger.h"

string CLogger::logFilePath = "";

void CLogger::WriteLine(string logString) {
	// Open, write then close
	ofstream log;
	log.open(logFilePath.c_str());
	log << logString.c_str() << "\n";
	log.close();
}

void CLogger::InstantiateLogFile() {
	// Date string
	time_t now = time(0);
	tm* date = gmtime(&now);
	string strDate;
	strDate += to_string(date->tm_mon + 1);
	strDate += to_string(date->tm_mday);
	strDate += to_string(1900 + date->tm_year);

	// Generate a new file name
	logFilePath = CUtils::DllPath + "\\" + "vNAAATS-" + strDate;

	// Write the initialisation
	vector<string> output;
	output.push_back
}

void CLogger::Log(vector<string> text, CLogType type) {

}

void CLogger::GeneratePrefix(CLogType type) {
	// This will store the prefix
	string prefix = "[";

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
}