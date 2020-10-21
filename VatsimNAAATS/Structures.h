#pragma once
#include <cmath>
#include <string>
#include <vector>
#include "Constants.h"
#include <map>

// Describes a NAT track
struct CTrack {
	string Identifier;
	string TMI;
	vector<string> Route;
	vector<CPosition> RouteRaw;
	vector<int> FlightLevels;
	CTrackDirection Direction;
};

// Describes a point along an aircraft's route
struct CRoutePosition {
	string Fix;
	CPosition PositionRaw;
	string Estimate;
	int DistanceFromLastPoint;
	int FlightLevel;
};

// Describes an inbound aircraft
struct CInboundAircraft {
	CInboundAircraft(string cs, int fA, int cL, string pt, string est, string dest, bool direction) {
		Callsign = cs;
		FinalAltitude = fA;
		ClearedLevel = cL;
		Point = pt;
		Estimate = est;
		Destination = dest;
		Direction = direction;
	}
	string Callsign;
	int FinalAltitude;
	int ClearedLevel;
	string Point;
	string Estimate;
	string Destination;
	bool Direction;
};

// Describes a aircraft status
struct CAircraftStatus {
	CAircraftStatus() {}; // Default constructor
	CAircraftStatus(string cs, int alt, int gs, int hdg, CPosition pos) {
		Callsign = cs;
		Altitude = alt;
		GroundSpeed = gs;
		Heading = hdg;
		Position = pos;
	}
	string Callsign;
	int Altitude;
	int GroundSpeed;
	int Heading;
	CPosition Position;
};

// STCA status
struct CSTCAStatus {
	CSTCAStatus() {}; // Default constructor
	CSTCAStatus(string csA, string csB, CConflictStatus status) {
		CallsignA = csA;
		CallsignB = csB;
		ConflictStatus = status;
	}
	string CallsignA;
	string CallsignB;
	CConflictStatus ConflictStatus;
};

// Describes a separation status
struct CSepStatus {
	int DistanceAsTime;
	int DistanceAsNM;
	int AltDifference;
	bool IsDistanceClosing;
	CTrackStatus TrackStatus;
	CConflictStatus ConflictStatus;
	pair<CPosition, CPosition> AircraftLocations;
};

// Describes a window text input
struct CTextInput {
	CTextInput() {}; // Default constructor
	CTextInput(int id, int type, string lbl, string content, int width, CInputState state) {
		Id = id;
		Type = type;
		Label = lbl;
		Content = content;
		Width = width;
		State = state;
	}
	int Id;
	int Type;
	string Label;
	string Content;
	int Width;
	CInputState State;
};

// Describes a window check box
struct CCheckBox {
	CCheckBox() {}; // Default constructor
	CCheckBox(int id, int type, string lbl, bool isChecked, CInputState state) {
		Id = id;
		Type = type;
		Label = lbl;
		IsChecked = isChecked;
		State = state;
	}
	int Id;
	int Type;
	string Label;
	bool IsChecked;
	CInputState State;
};

// Describes a button
struct CWinButton {
	CWinButton() {}; // Default constructor
	CWinButton(int id, int type, string lbl, CInputState state, int width = -1, int cycle = 0) {
		Id = id;
		Type = type;
		Width = width;
		Label = lbl;
		State = state;
		Cycle = cycle;
	}
	int Id;
	int Type;
	int Width;
	int Cycle;
	string Label;
	CInputState State;
};

// Describes a dropdown item
struct CDropDownItem {
	CDropDownItem() {}; // Default constructor
	CDropDownItem(int id, int type, string lbl, bool hovered, bool checkItem, CInputState state) {
		Id = id;
		Type = type;
		Label = lbl;
		IsHovered = hovered;
		IsCheckItem = checkItem;
		State = state;
	}
	int Id;
	int Type;
	string Label;
	bool IsHovered;
	bool IsCheckItem;
	CInputState State;
	int Width;
};

// Describes a dropdown
struct CDropDown {
	CDropDown() {}; // Default constructor
	CDropDown(int id, int type, string value, map<string, bool>* dropDownItems, CInputState state, int width = -1) {
		Id = id;
		Type = type;
		Value = value;
		int counter = 800;
		for (auto kv : *dropDownItems) {
			Items.insert(make_pair(counter, CDropDownItem(counter, type, kv.first, false, kv.second, CInputState::INACTIVE)));
			counter++;
		}
		State = state;
		Width = width;
	}
	int Id;
	int Type;
	string Value;
	map<int, CDropDownItem> Items;
	CInputState State;
	int Width;
};

struct CWinScrollBar {
	CWinScrollBar() {}
	CWinScrollBar(int id, int type, int cSize, int fSize, bool isX) {
		Id = id;
		Type = type;
		ContentSize = cSize;
		FrameSize = fSize;
		ContentRatio = (double)FrameSize / (double)ContentSize;
		TotalScrollableArea = ContentSize - FrameSize;
		IsHorizontal = isX;

		// Grip size (subtract 12 because buttons)
		GripSize = (FrameSize - 12) * ContentRatio;
		if (GripSize < 20) GripSize = 20; // Minimum grip size
		if (GripSize > FrameSize - 12) GripSize = FrameSize - 12; // Maximum grip size
		PositionDelta = 0;
		WindowPos = 0;

	}
	int Id;
	int Type;
	int ContentSize;
	int FrameSize;
	double ContentRatio;
	double GripSize;
	double PositionDelta;
	double WindowPos;
	int TotalScrollableArea;
	bool IsHorizontal;
};

struct CAircraftFlightPlan {
	CAircraftFlightPlan() {}; // Default constructor
	CAircraftFlightPlan(string cs) {
		Callsign = cs;
	}
	string Callsign;
};

/*struct CAircraftFlightPlan {
	CAircraftFlightPlan() {}; // Default constructor
	CAircraftFlightPlan(string cs) {
		
	}
	string Callsign;
	string Type;
	string Depart;
	string Dest;
	string Etd;
	string SELCAL;
	string DLStatus;
	string Communications;
	string Sector;
	string FlightLevel;
	string Mach;
	string Track;
	string State;
	CMessage CurrentMessage;
	vector<string> FlightHistory;
	vector<CRoutePosition> Route;
	vector<CFlightRestriction> Restrictions;
	bool IsCleared;
};

struct CMessage {
	CMessage() {}; // Default constructor
	CMessage() {

	}
	string To;
	string From;
	string MessageRaw;
	string MessageHuman;
	CMessageType Type;
};

struct CFlightRestriction {
	CFlightRestriction() {}; // Default constructor
	CFlightRestriction() {
		
	}
	int Type;
	string Content;
};*/
