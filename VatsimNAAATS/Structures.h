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
	CWinButton(int id, int type, string lbl, CInputState state) {
		Id = id;
		Type = type;
		Label = lbl;
		State = state;
	}
	int Id;
	int Type;
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
};

// Describes a dropdown
struct CDropDown {
	CDropDown() {}; // Default constructor
	CDropDown(int id, int type, string value, map<int, CDropDownItem>* items) {
		Id = id;
		Type = type;
		Value = value;
		Items = *items;
	}
	int Id;
	int Type;
	string Value;
	map<int, CDropDownItem> Items;
};

struct CAcFPStatus {
	CAcFPStatus() {}; // Default constructor
	CAcFPStatus(string cs, CFlightPlanMode mode) {
		Callsign = cs;
		Mode = mode;
	}
	string Callsign;
	CFlightPlanMode Mode;
};
