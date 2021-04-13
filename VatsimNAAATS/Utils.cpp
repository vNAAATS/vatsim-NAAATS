#define _USE_MATH_DEFINES
#include "pch.h"
#include<cmath>
#include "Utils.h"
#include "Constants.h"
#include "RadarDisplay.h"

// Default values
int CUtils::InboundX = 1400;
int CUtils::InboundY = 170;
int CUtils::OthersX = 1200;
int CUtils::OthersY = 150;
int CUtils::ConflictX = 60;
int CUtils::ConflictY = 120;
int CUtils::RCLX = 600;
int CUtils::RCLY = 150;
int CUtils::TrackWindowX = 300;
int CUtils::TrackWindowY = 200;
int CUtils::AltFiltLow = 0;
int CUtils::AltFiltHigh = 700;
bool CUtils::GridEnabled = false;
bool CUtils::TagsEnabled = true;
bool CUtils::QckLookEnabled = false;
bool CUtils::OverlayEnabled = false;
int CUtils::AreaSelection = 802;
int CUtils::SelectedOverlay = 800;
int CUtils::PosType = 802;
char CUtils::DllPathFile[_MAX_PATH];
string CUtils::DllPath;

// Save plugin data
void CUtils::SavePluginData(CRadarScreen* screen) {

	// Inbound list
	screen->SaveDataToAsr(SET_INBNDX.c_str(), "X position of Inbound list.", to_string(InboundX).c_str());
	screen->SaveDataToAsr(SET_INBNDY.c_str(), "Y position of Inbound list.", to_string(InboundY).c_str());

	// Others list
	screen->SaveDataToAsr(SET_OTHERSX.c_str(), "X position of Other list.", to_string(OthersX).c_str());
	screen->SaveDataToAsr(SET_OTHERSY.c_str(), "Y position of Other list.", to_string(OthersY).c_str());

	// Altitude filter (TODO)
	screen->SaveDataToAsr(SET_ALTFILT_LOW.c_str(), "Lower level for altitude filter.", to_string(AltFiltLow).c_str());
	screen->SaveDataToAsr(SET_ALTFILT_HIGH.c_str(), "Upper level for altitude filter.", to_string(AltFiltHigh).c_str());

	// Misc display settings
	screen->SaveDataToAsr(SET_GRID.c_str(), "Grid enabled/disabled.", GridEnabled ? "true" : "false");
	screen->SaveDataToAsr(SET_TAGS.c_str(), "Tags enabled/disabled.", TagsEnabled ? "true" : "false");
	screen->SaveDataToAsr(SET_OVERLAY.c_str(), "Overlay enabled/disabled.", OverlayEnabled ? "true" : "false");
	screen->SaveDataToAsr(SET_QCKLOOK.c_str(), "Quick Look enabled/disabled.", QckLookEnabled ? "true" : "false");

	// Dropdown values
	screen->SaveDataToAsr(SET_AREASEL.c_str(), "Selected area ownership.", to_string(AreaSelection).c_str());
	screen->SaveDataToAsr(SET_OVERLAYSEL.c_str(), "Selected overlay.", to_string(SelectedOverlay).c_str());
	screen->SaveDataToAsr(SET_POSTYPESEL.c_str(), "Selected position type.", to_string(PosType).c_str());
}

bool CUtils::WrapText(CDC* dc, string textToWrap, char wrapChar, int contentWidth,  vector<string>* ptrWrappedText) {
	// Split string
	vector<string> tokens;
	StringSplit(textToWrap, wrapChar, &tokens);

	// Intermediate string to store line data whilst length is being calculated
	string intermediate = "";

	// Iterate through and calculate text extent along the way
	for (int i = 0; i < tokens.size(); i++) {
		// Variable to calculate text extent before adding it to the intermediate line
		string beforeAdd = intermediate + tokens[i] + wrapChar;

		// If the text extent of the temporary variable is now greater than the content width
		if (dc->GetTextExtent(beforeAdd.c_str()).cx > contentWidth) {
			// Add the intermediate string to the return vector and clear the intermediate
			ptrWrappedText->push_back(intermediate);
			intermediate = tokens[i] + wrapChar;
		}
		else {
			// Add the token to the intermediate string
			intermediate += tokens[i] + wrapChar;
		}
	}

	// Reached the end, there may be a bit of left over string, if so, add it
	if (intermediate != "") {
		ptrWrappedText->push_back(intermediate);
	}

	return 0;
}

bool CUtils::StringSplit(string str, char splitBy, vector<string>* ptrTokens) {
	// String stream
	stringstream stream(str);

	// Intermediate value
	string intermediate;

	// Tokenise the string
	while (getline(stream, intermediate, splitBy))
	{
		ptrTokens->push_back(intermediate);
	}

	return 0;
}

// Phraseology parser
string CUtils::ParseToPhraseology(string rawInput, CMessageType type, string callsign) {
	try {
		// Split the string
		vector<string> splitString;
		StringSplit(rawInput, ':', &splitString);

		// CALLSIGN:LOG_ON:CONTROLLER
		if (type == CMessageType::LOG_ON) {
			return "REQ DATALINK LOG-ON STATION " + splitString[2];
		}
		// STATION_CALLSIGN:TRANSFER:AIRCRAFT_CALLSIGN
		else if (type == CMessageType::TRANSFER) {
			return "STATION " + splitString[0] + " REQUEST TRANSFER " + splitString[2] + " TO YOU";
		}
		// STATION_CALLSIGN:TRANSFER_ACCEPT:AIRCRAFT_CALLSIGN
		else if (type == CMessageType::TRANSFER_ACCEPT) {
			return "STATION " + splitString[0] + " ACCEPTED TRANSFER " + splitString[2];
		}
		// STATION_CALLSIGN:TRANSFER_REJECT:AIRCRAFT_CALLSIGN
		else if (type == CMessageType::TRANSFER_REJECT) {
			return "STATION " + splitString[0] + " REJECTED TRANSFER " + splitString[2];
		}
		// CALLSIGN:CLEARANCE_REQUEST:ICAO CODE:ROUTE STRING OR NULL:WAYPOINT:EST AS ZULU:LETTER OR RR:LEVEL:MACH:<FREETEXT>
		else if (type == CMessageType::CLEARANCE_REQ) {
			string returnString = "OCA CLR REQ: CLA TO " + splitString[2] + " VIA ";
			if (splitString[3] != "NULL") {
				returnString += "RANDOM ROUTING " + splitString[3] + ".";
			}
			else {
				returnString += "TRACK " + splitString[6] + ".";
			}
			// Flight level and mach
			returnString += " F" + splitString[7] + " M" + PadWithZeros(3, stoi(splitString[8]));
			// Estimated time
			returnString += " EST " + splitString[4] + " AT " + splitString[5];
			// Freetext
			//returnString += ". FREE: " + splitString[9];
			return returnString;
		}
		// CALLSIGN:CLEARANCE_ISSUE:ICAO CODE:ROUTE STRING OR NULL:WAYPOINT:EST AS ZULU:LETTER OR RR:LEVEL:MACH:ATC/:LCHG:MCHG:RERUTE
		else if (type == CMessageType::CLEARANCE_ISSUE) {
			string returnString = "CZQX CLRNCE: CLA TO " + splitString[2] + " VIA ";
			if (splitString[3] != "NULL") {
				returnString += "RANDOM ROUTING " + splitString[3] + ".";
			}
			else {
				returnString += splitString[4] + " NAT TRACK " + splitString[6] + ".";
			}
			// Estimate
			returnString += " FM " + splitString[4] + "/" + splitString[5];
			// Flight level and mach
			returnString += " MNTN F" + splitString[7] + " M" + PadWithZeros(3, stoi(splitString[8])) + " ";
			// Freetext
			//returnString += ". FREE: " + splitString[9];

			// Restrictions
			auto restrictionsIndex = std::find(splitString.begin(), splitString.end(), "ATC/");
			for (auto idx = restrictionsIndex; idx != splitString.end(); idx++) {
				if (*idx == "RERUTE")
					returnString += "ROUTE HAS BEEN CHANGED.";
			}
			return returnString;
		}
		// CALLSIGN:REVISION_REQ:MCHG:CONTENT:LCHG:CONTENT:RERUTE:CONTENT
		else if (type == CMessageType::REVISION_REQ) {
			string returnString = "REQUEST [";
			int counter = 0; // How many revisions (so that comma can be placed)
			for (int i = 0; i < splitString.size(); i++) {
				if (splitString[i] == "MCHG") {
					returnString += "M" + PadWithZeros(3, stoi(splitString[i + 1])) + "]";
				}
				if (splitString[i] == "LCHG") {
					returnString += "[F" + splitString[i + 1] + "] ";
				}
				if (splitString[i] == "RERUTE") {
					returnString += "[" + splitString[i + 1] + "] ";
				}
			}
			return returnString;
		}
		else if (type == CMessageType::REVISION_ISSUE) {
			// Flight data
			CAircraftFlightPlan* primedPlan = CDataHandler::GetFlightData(callsign);
			string returnString;
			// Split the current message
			vector<string> splitString;
			StringSplit(rawInput, ':', &splitString);
			int machChange = -1;
			int levelChange = -1;
			int rerute = -1;
			for (int i = 0; i < splitString.size(); i++) {
				if (splitString[i] == "ATC/")
					break;
				if (splitString[i] == "MCHG") {
					machChange = i + 1;
				}
				if (splitString[i] == "LCHG") {
					levelChange = i + 1;
				}
				if (splitString[i] == "RERUTE") {
					rerute = i + 1;
				}
			}

			bool isLevelRestriction = false;
			bool isMachRestriction = false;
			bool isRouteRestriction = false;

			vector<string> restrictions = { "LCHG, MCHG, ATA", "ATB", "XAT", "UNABLE", "INT" };

			// Restrictions
			auto restrictionsIndex = std::find(splitString.begin(), splitString.end(), "ATC/");
			for (auto idx = restrictionsIndex; idx != splitString.end(); idx++) {
				if (*idx == "LCHG") {
					isLevelRestriction = true;
					int isTime = false;
					returnString += "CLIMB TO AND MAINTAIN F" + splitString[levelChange] + " CROSS " + *(idx + 1) + " AT F" + splitString[levelChange] + " REPORT LEAVING F" + primedPlan->FlightLevel + " REPORT LEVEL F" + splitString[levelChange] + ". ";
				}
				else if (*idx == "MCHG") {
					isMachRestriction = true;
					returnString += "MAINTAIN MACH 0" + splitString[machChange] + ". CROSS " + *(idx + 1) + " AT " + splitString[machChange] + ". ";
				}
				else if (*idx == "EPC") {

				}
				else if (*idx == "RTD") {

				}
				else if (*idx == "UNABLE") {
					vector<string> unables;
					if (std::find(restrictions.begin(), restrictions.end(), *(idx + 1)) != restrictions.end()) {
						unables.push_back(*(idx + 1));
					}
					if (std::find(restrictions.begin(), restrictions.end(), *(idx + 2)) != restrictions.end()) {
						unables.push_back(*(idx + 2));
					}
					if (std::find(restrictions.begin(), restrictions.end(), *(idx + 3)) != restrictions.end()) {
						unables.push_back(*(idx + 3));
					}
					int counter = 0;
					for (int i = 0; i < unables.size(); i++) {
						if (unables[i] == "LCHG") {
							if (counter > 1)
								returnString += ", LEVEL CHANGE";
							else
								returnString += "UNABLE LEVEL CHANGE ";
						}
						else if (unables[i] == "MCHG") {
							if (counter > 1)
								returnString += ", SPD CHANGE";
							else
								returnString += "UNABLE SPD CHANGE ";
						}
						else {
							if (counter > 1)
								returnString += ", RERUTE";
							else
								returnString += "UNABLE RERUTE ";
						}
					}
					returnString += " ";
				}
				else if (*idx == "ATA") {
					returnString += "CROSS " + *(idx + 1) + " AFTER " + *(idx + 2) + ". ";
				}
				else if (*idx == "ATB") {
					returnString += "CROSS " + *(idx + 1) + " BEFORE " + *(idx + 2) + ". ";
				}
				else if (*idx == "XAT") {
					returnString += "CROSS " + *(idx + 1) + " AT " + *(idx + 2) + ". ";
				}
			}

			if (machChange != -1) {
				returnString += "MAINTAIN MACH 0" + splitString[machChange] + ". ";
			}
			if (levelChange != -1 && !isLevelRestriction) {
				if (!returnString.empty())
					returnString += " ";

				returnString += "CLIMB TO AND MAINTAIN F" + splitString[levelChange] + " REPORT LEAVING F" + primedPlan->FlightLevel + " REPORT LEVEL F" + splitString[levelChange] + ". ";
			}
			if (rerute != -1 && !isRouteRestriction) {
				if (!returnString.empty())
					returnString += " ";
				returnString += "ROUTE HAS BEEN CHANGED CLEARED " + splitString[rerute] + ". ";
			}

			return returnString;
		}
		// CALLSIGN:WILCO
		else if (type == CMessageType::WILCO) {
			return "WILCO LAST MSG";
		}
		// CALLSIGN:UNABLE
		else if (type == CMessageType::UNABLE) {
			return "UNABLE LAST REQUEST";
		}
		// CALLSIGN:ROGER
		else if (type == CMessageType::ROGER) {
			return "ROGER LAST MSG";
		}
		else { // Default
			return rawInput;
		}
	}
	catch(std::exception e) {
		return rawInput;
	}
}

// Raw format parser
string CUtils::ParseToRaw(string callsign, CMessageType type, CAircraftFlightPlan* copy) {
	// Flight data
	CAircraftFlightPlan* fp = copy != nullptr ? copy : CDataHandler::GetFlightData(callsign);

	if (type == CMessageType::LOG_ON_CONFIRM) {
		return fp->Callsign + ":LOG_ON_CONFIRM";
	}
	if (type == CMessageType::LOG_ON_REJECT) {
		return fp->Callsign + ":LOG_ON_REJECT";
	}
	if (type == CMessageType::TRANSFER_ACCEPT) {
		return fp->CurrentMessage->From + ":TRANSFER_ACCEPT:" + fp->Callsign;
	}
	if (type == CMessageType::TRANSFER_REJECT) {
		return fp->CurrentMessage->From + ":TRANSFER_REJECT:" + fp->Callsign;
	}
	if (type == CMessageType::CLEARANCE_ISSUE) {
		// Split the current message
		vector<string> splitString;
		StringSplit(fp->CurrentMessage->MessageRaw, ':', &splitString);

		// Get the route
		string routeString;
		if (fp->Track == "RR") {
			for (int i = 0; i < fp->RouteRaw.size(); i++)
				routeString += fp->RouteRaw[i] + " ";
			routeString = routeString.substr(0, routeString.size() - 1);
		}
		else {
			routeString = "NULL";
		}
		string returnString = fp->Callsign + ":CLEARANCE_ISSUE:" + fp->Dest + ":" + routeString + ":" + splitString[4] + ":" + splitString[5] + ":" + fp->Track + ":" + fp->FlightLevel + ":" + fp->Mach + ":" + "ATC/";
		if (fp->Restrictions.empty()) {
			returnString += "NULL";
		}
		else {
			for (int i = 0; i < fp->Restrictions.size(); i++) {
				if (fp->Restrictions[i].Type == CRestrictionType::LCHG) {
					returnString += ":LCHG:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::MCHG) {
					returnString += ":MCHG:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::EPC) {
					returnString += ":EPC:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::RERUTE) {
					returnString += ":RERUTE";
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::RTD) {
					returnString += ":RTD";
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::UNABLE) {
					returnString += ":UNABLE:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::ATA) {
					returnString += ":ATA:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::ATB) {
					returnString += ":ATB:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::XAT) {
					returnString += ":XAT:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::INT) {
					returnString += ":INT:" + fp->Restrictions[i].Content;
				}
			}
		}

		return returnString;
	}
	if (type == CMessageType::CLEARANCE_REJECT) {
		return fp->Callsign + ":CLEARANCE_REJECT";
	}
	if (type == CMessageType::REVISION_ISSUE) {
		// Flight data
		CAircraftFlightPlan* primedPlan = CDataHandler::GetFlightData(callsign);
		string returnString;
		string routeString;
		for (int i = 0; i < primedPlan->RouteRaw.size(); i++)
			routeString += primedPlan->RouteRaw[i] + " ";

		string copyRouteString;
		for (int i = 0; i < fp->RouteRaw.size(); i++)
			copyRouteString += fp->RouteRaw[i] + " ";
		
		returnString += fp->Callsign + ":REVISION_ISSUE";
		// Switch
		if (fp->Mach != primedPlan->Mach) {
			returnString += ":MCHG:" + fp->Mach;
		}
		if (fp->FlightLevel != primedPlan->FlightLevel) {
			returnString += ":LCHG:" + fp->FlightLevel;
		}
		if (copyRouteString != routeString) {
			returnString += ":RERUTE:" + copyRouteString;
		}
		
		returnString += ":ATC/";
		if (fp->Restrictions.empty()) {
			returnString += "NULL";
		}
		else {
			for (int i = 0; i < fp->Restrictions.size(); i++) {
				if (fp->Restrictions[i].Type == CRestrictionType::LCHG) {
					returnString += ":LCHG:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::MCHG) {
					returnString += ":MCHG:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::EPC) {
					returnString += ":EPC:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::RERUTE) {
					returnString += ":RERUTE:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::RTD) {
					returnString += ":RTD:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::UNABLE) {
					returnString += ":UNABLE:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::ATA) {
					returnString += ":ATA:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::ATB) {
					returnString += ":ATB:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::XAT) {
					returnString += ":XAT:" + fp->Restrictions[i].Content;
				}
				else if (fp->Restrictions[i].Type == CRestrictionType::INT) {
					returnString += ":INT:" + fp->Restrictions[i].Content;
				}
			}
		}
		return returnString;
	}
	if (type == CMessageType::REVISION_REJECT) {
		return fp->Callsign + ":REVISION_REJECT";
	}
}
// Load plugin data
void CUtils::LoadPluginData(CRadarScreen* screen) {
	// Strings to parse data
	const char* stra;
	const char* strb;

	// Inbound list
	stra = screen->GetDataFromAsr(SET_INBNDX.c_str());
	strb = screen->GetDataFromAsr(SET_INBNDY.c_str());
	if (stra != NULL && strb != NULL) {
		InboundX = stoi(stra);
		InboundY = stoi(strb);
	}

	// Others list
	stra = screen->GetDataFromAsr(SET_OTHERSX.c_str());
	strb = screen->GetDataFromAsr(SET_OTHERSY.c_str());
	if (stra != NULL && strb != NULL) {
		OthersX = stoi(stra);
		OthersY = stoi(strb);
	}

	// Others list
	stra = screen->GetDataFromAsr(SET_ALTFILT_LOW.c_str());
	strb = screen->GetDataFromAsr(SET_ALTFILT_HIGH.c_str());
	if (stra != NULL && strb != NULL) {
		AltFiltLow = stoi(stra);
		AltFiltHigh = stoi(strb);
	}
	// Grid enabled
	stra = screen->GetDataFromAsr(SET_GRID.c_str());
	if (stra != NULL) {
		GridEnabled = (stra[0] == 't');
	}
	
	// Tags enabled
	stra = screen->GetDataFromAsr(SET_TAGS.c_str());
	if (stra != NULL) {
		TagsEnabled = (stra[0] == 't');
	}

	// Quick look enabled
	stra = screen->GetDataFromAsr(SET_QCKLOOK.c_str());
	if (stra != NULL) {
		QckLookEnabled = (stra[0] == 't');
	}

	// Overlay enabled
	stra = screen->GetDataFromAsr(SET_OVERLAY.c_str());
	if (stra != NULL) {
		OverlayEnabled = (stra[0] == 't');
	}

	// Area selection
	stra = screen->GetDataFromAsr(SET_AREASEL.c_str());
	if (stra != NULL) {
		AreaSelection = stoi(stra);
	}

	// Overlay selection
	stra = screen->GetDataFromAsr(SET_OVERLAYSEL.c_str());
	if (stra != NULL) {
		SelectedOverlay = stoi(stra);
	}

	// Position type selection
	stra = screen->GetDataFromAsr(SET_POSTYPESEL.c_str());
	if (stra != NULL) {
		PosType = stoi(stra);
	}

	// Show a user message saying that the plugin was loaded successfully
	screen->GetPlugIn()->DisplayUserMessage("Message", "vNAAATS Plugin", string("version " + PLUGIN_VERSION + " initialised.").c_str(), false, false, false, false, false);

	if (IS_ALPHA)
		screen->GetPlugIn()->DisplayUserMessage("Message", "vNAAATS Plugin", string("This is a BETA version. Please report any issues to a.ogden@vatcan.ca or submit them here: https://ganderoceanic.com/vnaaats-feedback.").c_str(), false, false, false, false, false);
}

// Returns the requested format, or returns the same string if the format was unchanged
string CUtils::ConvertCoordinateFormat(string coordinateString, int format) { // format = 0 (slash format), 1 (xxxxN), 2 (xxNxxxW)
	// Return var
	string returnFormat;
	try {
		// First we make sure there are numbers
		int isAllAlpha = true;
		for (int j = 0; j < coordinateString.size(); j++) {
			if (isdigit(coordinateString.at(j))) {
				isAllAlpha = false;
			}
		}
		// Check the current format of the input string
		int currentFormat = -1;
		if (coordinateString.find('/') != string::npos) {
			currentFormat = 0;
		}
		else if (coordinateString.find('W') == string::npos && coordinateString.find('/') == string::npos && coordinateString.size() == 5) {
			currentFormat = 1;
		}
		else if (coordinateString.find('W') != string::npos) {
			currentFormat = 2;
		}

		// Check the current format, if -1 or matches, just return the input string
		if (currentFormat == -1 || currentFormat == format || isAllAlpha || coordinateString.size() > 7) {
			return coordinateString;
		}

		// Otherwise, change the format
		if (format == 0) {
			if (currentFormat == 1) {
				returnFormat = coordinateString.substr(0, 2) + "/" + coordinateString.substr(2, 2);

			}
			else if (currentFormat == 2) {
				returnFormat = coordinateString.substr(0, 2) + "/" + coordinateString.substr(4, 2);
			}
		}
		else if (format == 1) {
			if (currentFormat == 0) {
				returnFormat = coordinateString.substr(0, 2) + coordinateString.substr(3, 2) + "N";
			}
			else if (currentFormat == 2) {
				returnFormat = coordinateString.substr(0, 2) + coordinateString.substr(4, 2) + "N";
			}
		}
		else {
			if (currentFormat == 0) {
				returnFormat = coordinateString.substr(0, 2) + "N0" + coordinateString.substr(3, 2) + "W";
			}
			else if (currentFormat == 1) {
				returnFormat = coordinateString.substr(0, 2) + "N0" + coordinateString.substr(2, 2) + "W";
			}
		}
	}
	catch (exception & ex) {
		// Return the old one if an exception occurs
		return coordinateString;
	}
	
	// Return the string
	return returnFormat;
}


bool CUtils::GetAircraftDirection(int heading) {
	if ((heading <= 359) && (heading >= 180)) {
		return false; // Westbound
	}
	else if ((heading >= 0) && (heading <= 179)) {

		return true; // Eastbound
	}
}

bool CUtils::IsEntryPoint(string pointName, bool side) {
	if (side) { // Gander
		if (find(pointsGander.begin(), pointsGander.end(), pointName) != pointsGander.end()) {
			return true; // Match
		}
		else {
			return false; // No match
		}
	}
	else { // Shanwick
		if (find(pointsShanwick.begin(), pointsShanwick.end(), pointName) != pointsShanwick.end()) {
			return true; // Match
		}
		else {
			return false; // No match
		}
	}
}

bool CUtils::IsExitPoint(string pointName, bool side) {
	if (side) { // Gander
		if (find(pointsShanwick.begin(), pointsShanwick.end(), pointName) != pointsShanwick.end()) {
			return true; // Match
		}
		else {
			return false; // No match
		}
	}
	else { // Shanwick
		if (find(pointsGander.begin(), pointsGander.end(), pointName) != pointsGander.end()) {
			return true; // Match
		}
		else {
			return false; // No match
		}
	}
}

bool CUtils::IsAircraftRelevant(CRadarScreen* screen, CRadarTarget* target, bool filtersDisabled) {
	// Flag
	bool valid = true;

	// Flight plan & position
	CFlightPlan fp = screen->GetPlugIn()->FlightPlanSelect(target->GetCallsign());
	CPosition pos = target->GetPosition().GetPosition();

	// Time and direction
	int entryMinutes = fp.GetSectorEntryMinutes();

	/// We check the selection values
	// Position type
	// Direction & area selection
	bool direction = GetAircraftDirection(target->GetTrackHeading());
	int areaSel = AreaSelection;
	if (PosType == 800) {

		// If greater than sixty minutes out or already in the airspace
		if (entryMinutes == 0 || entryMinutes < 0 || entryMinutes > 60) {
			valid = false;
		}
		
		// If wrong direction don't show
		if (direction && areaSel == 802 && entryMinutes != 0) {
			valid = false;
		}
		if (!direction && areaSel == 801 && entryMinutes != 0) {
			valid = false;
		}
	}
	else if (PosType == 801) {
		// If not ever going to enter, or greater than 60 min out
		if (entryMinutes < 0 || entryMinutes > 60) {
			valid = false;
		}

		// If wrong direction don't show
		if (direction && areaSel == 802 && entryMinutes != 0) {
			valid = false;
		}
		if (!direction && areaSel == 801 && entryMinutes != 0) {
			valid = false;
		}

		// Check now to not display aircraft east/west of certain longitude based on area selection
		if (areaSel == 802 && pos.m_Longitude < -32 && entryMinutes == 0) {
			valid = false;
		}
		if (areaSel == 801 && pos.m_Longitude > -28 && entryMinutes == 0) {
			valid = false;
		}
	}
	else {
		// If not ever going to enter, or greater than 20 min out
		if (entryMinutes < 0 || entryMinutes > 20) {
			valid = false;
		}

		// If wrong direction don't show
		if (direction && areaSel == 802 && entryMinutes != 0) {
			valid = false;
		}
		if (!direction && areaSel == 801 && entryMinutes != 0) {
			valid = false;
		}

		// Check now to not display aircraft east/west of certain longitude based on area selection
		if (areaSel == 802 && pos.m_Longitude < -32 && entryMinutes == 0) {
			valid = false;
		}
		if (areaSel == 801 && pos.m_Longitude > -28 && entryMinutes == 0) {
			valid = false;
		}
	}	

	// Let's check if filtering disabled
	if (filtersDisabled) { // ALL btn is pressed
		// If not ever going to enter, or greater than 90 min out
		if (entryMinutes < 0 || entryMinutes > 90) {
			valid = false;
		}
		else {
			valid = true;
		}
	}

	/// However we should keep them on the screen if they aren't long out of the airspace
	CAircraftFlightPlan* acFp = CDataHandler::GetFlightData(target->GetCallsign());
	if (acFp->IsValid) {
		// Current time
		time_t now = time(0);

		// Get the difference in seconds
		int diffSecs = (int)now - ((int)now + ((int)acFp->ExitTime * 60));

		// If greater than 10 minutes
		if (diffSecs > 600) {
			valid = false;
		}
		if (diffSecs >= 0 && diffSecs <= 600) {
			valid = true; // Override postype stuff
		}
	}
	
	string callsign = (string)screen->GetPlugIn()->ControllerMyself().GetCallsign();
	if (!screen->GetPlugIn()->ControllerMyself().IsController() || callsign.find("SUP") != string::npos) {

		if (pos.m_Longitude > -70 && pos.m_Longitude < -5)
			valid = true;
		if (pos.m_Latitude < 80 && pos.m_Longitude < -35)
			valid = true;
	}

	// Lastly let's check if they are assumed
	if (fp.GetTrackingControllerIsMe()) {
		valid = true;
	}

	return valid;
}

bool CUtils::IsAircraftEquipped(string rawRemarks, string rawAcInfo, char equipCode) {
	// Check for PBN string code
	size_t pbnFound = rawRemarks.find("PBN/");

	// Extract B1 & B2 if they exist
	if (pbnFound != string::npos) {
		pbnFound += 4;
		for (char c = rawRemarks[pbnFound]; c != ' '; c = rawRemarks[pbnFound]) {
			if (c == 'B' && (rawRemarks[pbnFound + 1] == '1' || rawRemarks[pbnFound + 1] == '2')) {
				return true;
			}
			// Increment
			pbnFound++;
		}
		// We didn't find one so return false
		return false;
	}

	// Check equipment code
	if (equipCode != '?') {
		// Check if it L
		if (equipCode == 'L')
			return true; // Aircraft is AGCS equipped
		return false;
	}
}

// Get radar target mode
CRadarTargetMode CUtils::GetTargetMode(int radarFlags) {
	// Switch the flags
	switch (radarFlags) {
		case 0:
			return CRadarTargetMode::ADS_B;
			break;
		case 1:
		case 7:
			return CRadarTargetMode::PRIMARY;
			break;
		case 3:
		case 2:
			return CRadarTargetMode::SECONDARY_C;
			break;
		case 6:
			return CRadarTargetMode::SECONDARY_S;
			break;
		default:
			return CRadarTargetMode::ADS_B;
	}
}

CPosition CUtils::PositionFromLatLon(double lat, double lon) {
	// Latitude
	int degrees = (int)floor(lat);
	double minutes = (lat - (double)degrees) * 60;
	double seconds = (minutes - floor(minutes)) * 60;
	degrees = abs(degrees); // Get absolute value
	string degreesFormatted;
	string minutesFormatted;
	string secondsFormatted;
	if (degrees < 10) { // Format degrees
		degreesFormatted = "N" + to_string(0) + to_string(0) + to_string(degrees);
	}
	else if (degrees < 100) {
		degreesFormatted = "N" + to_string(0) + to_string(degrees);
	}
	else {
		degreesFormatted = "N" + to_string(degrees);
	}
	if (minutes < 10) { // Format minutes
		minutesFormatted = to_string(0) + to_string((int)minutes);
	}
	else if (minutes == 0) {
		minutesFormatted = to_string(0) + to_string(0);
	}
	else {
		minutesFormatted = to_string((int)minutes);
	}
	if (seconds < 10 && seconds > 0) { // Format seconds
		secondsFormatted = to_string(0) + to_string(seconds);
	}
	else if (minutes == 0) {
		secondsFormatted = to_string(0) + to_string(0);
	}
	else {
		secondsFormatted = to_string(seconds);
	}
	string latitude = degreesFormatted + "." + minutesFormatted + "." + secondsFormatted;
	// Longitude
	degrees = (int)ceil(lon);
	minutes = (lon - (double)degrees) * 60;
	seconds = (minutes - floor(minutes)) * 60;
	degrees = abs(degrees); // Get absolute value
	if (degrees < 10) { // Format degrees
		degreesFormatted = "W" + to_string(0) + to_string(0) + to_string(abs(degrees));
	}
	else if (degrees < 100) {
		degreesFormatted = "W" + to_string(0) + to_string(abs(degrees));
	}
	else {
		degreesFormatted = "W" + to_string(abs(degrees));
	}
	if (minutes < 10) { // Format minutes
		minutesFormatted = to_string(0) + to_string((int)minutes);
	}
	else if (minutes == 0) {
		minutesFormatted = to_string(0) + to_string(0);
	}
	else {
		minutesFormatted = to_string((int)minutes);
	}
	if (seconds < 10 && seconds > 0) { // Format seconds
		secondsFormatted = to_string(0) + to_string(seconds);
	}
	else if (seconds == 0) {
		secondsFormatted = to_string(0) + to_string(0);
	}
	else {
		secondsFormatted = to_string(seconds);
	}
	string longitude = degreesFormatted + "." + minutesFormatted + "." + secondsFormatted;

	// Return
	CPosition pos;
	pos.LoadFromStrings(longitude.c_str(), latitude.c_str());
	return pos;
}

int CUtils::GetMach(int groundSpeed, int speedSound) {
	double result = ((double)groundSpeed / (double)speedSound) * 100.0;
	return (int)result;
}

string CUtils::PadWithZeros(int width, int number) {
	std::stringstream ss;
	ss << setfill('0') << setw(width) << number;
	return ss.str();
}

bool CUtils::IsAllAlpha(string str) {
	// Check whether there are any numbers in the item
	bool isAllAlpha = true;
	for (int i = 0; i < str.size(); i++) {
		if (isdigit(str[i])) {
			isAllAlpha = false;
		}
	}

	return isAllAlpha;
}

string CUtils::ParseZuluTime(bool delimit, int deltaTime, CFlightPlan* fp, int fix) {
	time_t now = time(0);
	tm* zuluTime = gmtime(&now);
	int deltaMinutes = 0;
	if (deltaTime != -1) {
		deltaMinutes = deltaTime;
	}
	if (fix != -1) {
		deltaMinutes = fp->GetExtractedRoute().GetPointDistanceInMinutes(fix);
	}
	int hours = zuluTime->tm_hour;
	int minutes = zuluTime->tm_min + deltaMinutes;

	if (minutes >= 60) {
		// Get minutes
		int minRemainder = minutes % 60;

		// Get number of hours
		hours += (minutes - minRemainder) / 60;

		// Reassign number of minutes
		minutes = minRemainder;
	}

	// Check if over 24 hours
	if (hours >= 24) {
		hours = hours - 24;
	}


	// Pad for zeros
	string strHours;
	if (hours < 10) {
		if (hours == 0) {
			strHours = "00";
		}
		else {
			strHours = "0" + to_string(hours);
		}
	}
	else {
		strHours = to_string(hours);
	}

	string strMinutes;
	if (minutes < 10) {
		if (minutes == 0) {
			strMinutes = "00";
		}
		else {
			strMinutes = "0" + to_string(minutes);
		}
	}
	else {
		strMinutes = to_string(minutes);
	}

	if (delimit) {
		return strHours + ":" + strMinutes;
	}
	else {
		return strHours + strMinutes;
	}
}

int CUtils::GetDistanceBetweenPoints(POINT p1, POINT p2) {
	// Get distance
	return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

POINT CUtils::GetMidPoint(POINT p1, POINT p2) {
	// Return midpoint
	return { (p1.x + p2.x) / 2, (p1.y + p2.y) / 2 };
}

int CUtils::GetTimeDistanceSpeed(int distanceNM, int speedGS) {
	// Get time in minutes
	int temp = ((float)distanceNM / (float)speedGS) * 60;
	return ((float)distanceNM / (float)speedGS) * 60;
}

double CUtils::GetDistanceSpeedTime(int speedGS, int timeSec) {
	// Get distance in metres
	return ((float)speedGS * 0.514444) * ((float)timeSec);
}

double CUtils::ToRadians(double degrees) {
	return degrees / 180.0 * M_PI;
}

double CUtils::ToDegrees(double radians) {
	return radians / M_PI * 180.0;
}

double CUtils::MetresToNauticalMiles(double metres) {
	return double(metres * 0.00053996);
}
 
template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

// This algorithm is a general solution to get the angle of intersection to any given path
double CUtils::GetPathAngle(double hdg1, double hdg2) {
	// Get theta
	double theta = abs(hdg2 - hdg1);

	// If the angle is obtuse, the aircraft are approaching, so minus 180
	if (theta > 90) return theta - 180;

	// Otherwise just return theta
	return theta;
}

CPosition CUtils::GetPointDistanceBearing(CPosition position, int distanceMetres, int heading) {

	// Get distance in nautical miles
	double distance = MetresToNauticalMiles(distanceMetres) / 60 * M_PI / 180;

	// Convert track heading and coordinates to radians
	double track = ToRadians(heading);
	double lat = ToRadians(position.m_Latitude);
	double lon = ToRadians(position.m_Longitude);

	// Calculate lat
	double newLat = asin(sin(lat) * cos(distance) + cos(lat) * sin(distance) * cos(track));

	// Calculate lon
	double newLon = cos(newLat) == 0 ? lon : fmod(lon + asin(sin(track) * sin(distance) / cos(newLat)) + M_PI, 2 * M_PI) - M_PI;
	
	// Return
	CPosition newPos;
	newPos.m_Latitude = CUtils::ToDegrees(newLat);
	newPos.m_Longitude = CUtils::ToDegrees(newLon);
	return newPos;
}

// Get the intersection between two vectors from two screen coordinates and bearings
// This algorithm utilises Euclidean geometry, taking advantage of the available conversions between lat/lon and screen coordinates to get accurate points. 
// If this creates inaccuracies, I will re-do the algorithm in non-Euclidian terms.
POINT CUtils::GetIntersectionFromPointBearing(POINT position1, POINT position2, double bearing1, double bearing2) {
	/* We solve the linear system using Cramer's rule: 
	 * ax1 - y1 = b 
	 * cx2 - y2 = d
	 * to find our intercept. (Origin is 0,0 to lat/lon)
	 * 
	 * Cramer's rule in terms of this solution: 
	 * Ax = b => xi = det(Ai)/det(A) | i = 1, 2
	 * 
	 * We use trigonometry to convert the bearings into positive and negative slopes:
	 * slope = 1 / tan(bearing) where bearing is in radians
	 */
	
	// Get the slopes
	double slope1 = 1.0 / tan(ToRadians(bearing1));
	double slope2 = 1.0 / tan(ToRadians(bearing2));

	// Get our b and d values
	double b = (slope1 * position1.x) + position1.y;
	double d = (slope2 * position2.x) + position2.y;

	// Calculate determinents using the coefficient matrix:
	// [slope1][1]
	// [slope2][1]
	// and the answer matrix:
	// [b]
	// [d]
	double determinent = slope1 - slope2;
	double detX = b - d;
	double detY = (slope1 * d) - (slope2 * b);

	// From the determinents, get our new X and Y values
	int newX = round(detX / determinent);
	int newY = round(detY / determinent);

	// Return the screen coordinates
	return POINT({ newX, newY });
}

