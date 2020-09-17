#pragma once
#include <string>
#include <EuroScopePlugIn.h>
#include <vector>

using namespace std;
using namespace EuroScopePlugIn;

/// VALUES
// Entry Waypoints
const vector<string> pointsGander = { "AVPUT",
        "CLAVY",
        "EMBOK",
        "KETLA",
        "LIBOR",
        "MAXAR",
        "NIFTY",
        "PIDSO",
        "RADUN",
        "SAVRY",
        "TOXIT",
        "URTAK",
        "VESMI",
        "AVUTI",
        "BOKTO",
        "CUDDY",
        "DORYY",
        "ENNSO",
        "HOIST",
        "IRLOK",
        "JANJO",
        "KODIK",
        "LOMSI",
        "MELDI",
        "NEEKO",
        "PELTU",
        "RIKAL",
        "SAXAN",
        "TUDEP",
        "UMESI",
        "ALLRY",
        "BUDAR",
        "ELSIR",
        "IBERG",
        "JOOPY",
        "MUSAK",
        "NICSO",
        "OMSAT",
        "PORTI",
        "RELIC",
        "SUPRY",
        "RAFIN",
        "JAROM",
        "BOBTU" 
};

const vector<string> pointsShanwick = { "RATSU",
        "LUSEN",
        "ATSIX",
        "ORTAV",
        "BALIX",
        "ADODO",
        "ERAKA",
        "ETILO",
        "GOMUP",
        "AGORI",
        "SUNOT",
        "BILTO",
        "PIKIL",
        "ETARI",
        "RESNO",
        "VENER",
        "DOGAL",
        "NEBIN",
        "MALOT",
        "TOBOR",
        "LIMRI",
        "ADARA",
        "DINIM",
        "RODEL",
        "SOMAX",
        "KOGAD",
        "BEDRA",
        "NERTU",
        "NASBA",
        "OMOKO",
        "TAMEL",
        "GELPO",
        "LASNO",
        "ETIKI",
        "UMLER",
        "SEPAL",
        "BUNAV",
        "SIVIR",
        "BEGAS",
        "DIVAT",
        "DIXIS",
        "BERUX",
        "PITAX",
        "PASAS",
        "NILAV",
        "GONAN" 
};

// Plugin info
const string PLUGIN_NAME = "vNAAATS";
const string PLUGIN_VERSION = "dev";
const string PLUGIN_AUTHOR = "Andrew Ogden github.com/andrewogden1678";
const string PLUGIN_COPYRIGHT = "GPL v3";

// Screen details
#define DISPLAY_NAME "vNAAATS Display"

// Text, margins and padding
const int MEN_FONT_SIZE = 16;
const int BTN_PAD_SIDE = 4;
const int BTN_PAD_TOP = 6;

// Lists
const int LIST_INBOUND_WIDTH = 400;

// Menu bar
const int MENBAR_HEIGHT = 100;
const int MENBAR_BTN_HEIGHT = 30;
const int RECT1_WIDTH = 500;
const int RECT2_WIDTH = 340;
const int RECT3_WIDTH = 160;
const int RECT4_WIDTH = 110;
const int RECT5_WIDTH = 280;
const int RECT6_WIDTH = 100;
const int RECT7_WIDTH = 250;
const int RECT8_WIDTH = 70;

/// OBJECTS
// Lists
const int LIST_INBOUND = 300;

// Menu buttons
const int MENBTN_SETUP = 400;
const int MENBTN_NOTEPAD = 401;
const int MENBTN_ADSC = 402;
const int MENBTN_FREQUENCY = 403;
const int MENBTN_MISC = 404;
const int MENBTN_MESSAGE = 405;
const int MENBTN_TAGS = 406;
const int MENBTN_FLIGHTPLAN = 407;
const int MENBTN_DESTAPT = 408;