#include "pch.h"
#include "Logger.h"
#include "DataHandler.h"
#include <json.hpp>
#include <mutex>
#include <atomic>
#include <WinInet.h>
#include <process.h>
#pragma comment(lib,"WinInet.Lib")

string CLogger::logFilePath = "";
bool CLogger::initialised = false;
bool CLogger::initialisedAc = false;
string CLogger::RemoteUrl = "";
int CLogger::RemoteMode = 1;
volatile long CLogger::RemoteDetail = 0;
static std::mutex remoteLogLock;
static std::vector<string> remoteLines;       // waiting to be sent
static std::vector<string> remotePrevLines;   // tail of the previous session's log (sent once)
static std::atomic<bool> remoteBusy(false);
static clock_t remoteLastFlush = 0;

// Keys and codes never leave the computer
string CLogger::Redact(string text) {
	static const char* names[] = { "code=", "key=", "logon=", "password=", "token=" };
	for (const char* n : names) {
		size_t pos = 0, len = strlen(n);
		while ((pos = text.find(n, pos)) != string::npos) {
			size_t start = pos + len, end = start;
			while (end < text.size() && text[end] != '&' && text[end] != ' ' && text[end] != '\n' && text[end] != '\r' && text[end] != '"') end++;
			text.replace(start, end - start, "***");
			pos = start + 3;
		}
	}
	return text;
}

void CLogger::QueueRemote(CLogType type, const string& line) {
	if (RemoteMode <= 0) return;
	if (type == CLogType::NORM && RemoteMode < 2 && !RemoteDetail) return;   // detail only when asked for
	try {
		string clean = Redact(line);
		std::lock_guard<std::mutex> lock(remoteLogLock);
		remoteLines.push_back(clean);
		if (remoteLines.size() > 800) remoteLines.erase(remoteLines.begin(), remoteLines.begin() + 200);   // never grow without limit
	}
	catch (...) {}
}

void CLogger::Log(CLogType type, string text, string invokedBy) {
	// Format invokedBy if not an empty string
	string by = invokedBy != "" ? "[" + invokedBy + "] " : "";
	QueueRemote(type, GeneratePrefix(type) + by + text);

	if (DEBUG_MODE) {
		// Prefix
		string prefix = GeneratePrefix(type);
		invokedBy = by;

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

	// Keep the end of the last session's log (it is about to be overwritten): after a crash it shows what happened last
	try {
		ifstream old(logFilePath.c_str());
		vector<string> tail;
		string ln;
		while (getline(old, ln)) {
			tail.push_back(Redact(ln));
			if (tail.size() > 80) tail.erase(tail.begin());
		}
		std::lock_guard<std::mutex> lock(remoteLogLock);
		remotePrevLines = tail;
	}
	catch (...) {}

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

// ---- Admin log sender: batches lines and POSTs them from a background thread (never blocks the radar)
static bool PostLines(const string& url, const vector<string>& lines, bool prev, bool& detail) {
	nlohmann::json body;
	body["lines"] = nlohmann::json::array();
	for (auto& l : lines) body["lines"].push_back(l);
	if (prev) body["prev"] = true;
	string payload = body.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);

	URL_COMPONENTSA uc = { 0 };
	char host[256] = { 0 }, path[2048] = { 0 };
	uc.dwStructSize = sizeof(uc);
	uc.lpszHostName = host; uc.dwHostNameLength = sizeof(host);
	uc.lpszUrlPath = path; uc.dwUrlPathLength = sizeof(path);
	if (!InternetCrackUrlA(url.c_str(), 0, 0, &uc)) return false;
	bool https = uc.nScheme == INTERNET_SCHEME_HTTPS;

	HINTERNET net = InternetOpenA("vNAAATS", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!net) return false;
	DWORD timeout = 6000;
	InternetSetOptionA(net, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
	InternetSetOptionA(net, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
	InternetSetOptionA(net, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
	bool ok = false;
	HINTERNET con = InternetConnectA(net, host, uc.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (con) {
		DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | (https ? INTERNET_FLAG_SECURE : 0);
		HINTERNET req = HttpOpenRequestA(con, "POST", path, NULL, NULL, NULL, flags, 0);
		if (req) {
			const char* hdr = "Content-Type: application/json\r\n";
			if (HttpSendRequestA(req, hdr, (DWORD)strlen(hdr), (LPVOID)payload.c_str(), (DWORD)payload.size())) {
				DWORD status = 0, sz = sizeof(status);
				HttpQueryInfoA(req, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &sz, NULL);
				char buf[512];
				DWORD got = 0;
				string resp;
				while (resp.size() < 2048 && InternetReadFile(req, buf, sizeof(buf), &got) && got > 0) resp.append(buf, got);
				ok = status == 200;
				if (ok) detail = resp.find("\"detail\": true") != string::npos || resp.find("\"detail\":true") != string::npos;
			}
			InternetCloseHandle(req);
		}
		InternetCloseHandle(con);
	}
	InternetCloseHandle(net);
	return ok;
}

void CLogger::FlushRemoteThread(void* args) {
	try {
		string url = RemoteUrl;
		vector<string> prev, lines;
		{
			std::lock_guard<std::mutex> lock(remoteLogLock);
			prev.swap(remotePrevLines);
			size_t bytes = 0, n = 0;
			while (n < remoteLines.size() && n < 150 && bytes < 12000) bytes += remoteLines[n++].size() + 8;
			lines.assign(remoteLines.begin(), remoteLines.begin() + n);
			remoteLines.erase(remoteLines.begin(), remoteLines.begin() + n);
		}
		bool detail = RemoteDetail != 0;
		if (!url.empty()) {
			// url already holds ?code=KEY; add who we are and which version
			string full = url + "&me=" + CDataHandler::UrlEncode(CDataHandler::ControllerCallsign) + "&v=" + PLUGIN_VERSION;
			if (!prev.empty()) {
				vector<string> chunk(prev.begin() + (prev.size() > 60 ? prev.size() - 60 : 0), prev.end());
				chunk.insert(chunk.begin(), "[INIT] ----- end of the previous session's log (before this start) -----");
				PostLines(full, chunk, true, detail);
			}
			if (!lines.empty()) PostLines(full, lines, false, detail);   // a failed send drops the lines: the file log still has them
			else { vector<string> none; PostLines(full, none, false, detail); }   // keeps "full detail" in step with the admin's switch
			InterlockedExchange(&RemoteDetail, detail ? 1 : 0);
		}
	}
	catch (...) {}
	remoteBusy = false;
}

void CLogger::MaybeFlushRemote() {
	if (RemoteMode <= 0 || RemoteUrl.empty() || remoteBusy) return;
	if ((double)(clock() - remoteLastFlush) / (double)CLOCKS_PER_SEC < 5.0) return;
	remoteLastFlush = clock();
	remoteBusy = true;
	_beginthread(CLogger::FlushRemoteThread, 0, nullptr);
}
