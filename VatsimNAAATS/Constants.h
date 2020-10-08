#pragma once
#include <string>
#include <EuroScopePlugIn.h>
#include <vector>

using namespace std;
using namespace EuroScopePlugIn;

/// WAYPOINTS
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

/// VALUES
// Plugin info
const string PLUGIN_NAME = "vNAAATS";
const string PLUGIN_VERSION = "v0.1a";
const string PLUGIN_AUTHOR = "Andrew Ogden github.com/andrewogden1678";
const string PLUGIN_COPYRIGHT = "Gander Oceanic vNAAATS";

// Sector file & geo constants
const int SECTELEMENT_COORD_IDX = 7;
const int RADIUS_EARTH_NM = 3440.1;

// Screen details
#define DISPLAY_NAME "vNAAATS Display"

// Text, margins and padding
const int MEN_FONT_SIZE = 16;
const int BTN_PAD_SIDE = 4;
const int BTN_PAD_TOP = 6;

// Lists
const int LIST_INBOUND_WIDTH = 400;
const int LIST_OTHERS_WIDTH = 100;

// Menu bar
const int MENBAR_HEIGHT = 100;
const int MENBAR_BTN_HEIGHT = 30;
const int RECT1_WIDTH = 500;
const int RECT2_WIDTH = 340;
const int RECT3_WIDTH = 160;
const int RECT4_WIDTH = 100;
const int RECT5_WIDTH = 250;
const int RECT6_WIDTH = 100;
const int RECT7_WIDTH = 270;
const int RECT8_WIDTH = 90;

// Window sizes
const int WINSZ_TITLEBAR_HEIGHT = 20;
const int WINSZ_TCKINFO_WIDTH = 450;
const int WINSZ_TCKINFO_HEIGHT = 250;
const int WINSZ_FLTPLN_WIDTH = 550;
const int WINSZ_FLTPLN_HEIGHT = 600;

// Conflict
const int SEPTOOL_TIME = 2700; // 45 minutes
const int STCA_TIME = 480; // 8 minutes

/// OBJECTS
// Lists
const int LIST_INBOUND = 300;
const int LIST_OTHERS = 301;

// Dropdown
const int DROPDOWN = 800;
const int DRP_AREA_EGGX = 801;
const int DRP_AREA_CZQX = 802;
const int DRP_AREA_BDBX = 803;
const int DRP_OVL_ALL = 801;
const int DRP_OVL_EAST = 802;
const int DRP_OVL_WEST = 803;
const int DRP_OVL_SEL = 804;
const int DRP_TYPE_DEL = 801;
const int DRP_TYPE_ENR = 802;
const int DRP_TYPE_MULTI = 803;

// Screen
const int SCREEN_TAG = 1;
const int WINDOW = 100;

// Window specific buttons
const int WINBTN_CLOSE = 101;
const int WINBTN_TCKINFO_REFRESH = 102;
const int WINBTN_FLTPLN_COPY = 102;
const int WINBTN_FLTPLN_COORD = 103;
const int WINBTN_FLTPLN_MSG = 104;
const int WINBTN_FLTPLN_VOXCLEAR = 105;
const int WINBTN_FLTPLN_DELETE = 106;
const int WINBTN_FLTPLN_PROBE = 107;
const int WINBTN_FLTPLN_HIST = 108;
const int WINBTN_FLTPLN_SAVE = 109;
const int WINBTN_FLTPLN_ADS = 110;


// Menu buttons, dropdowns & text fields
const int MENBTN_SETUP = 400;
const int MENBTN_NOTEPAD = 401;
const int MENBTN_ADSC = 402;
const int MENBTN_TCKINFO = 403;
const int MENBTN_MISC = 404;
const int MENBTN_MESSAGE = 405;
const int MENBTN_TAGS = 406;
const int MENBTN_FLIGHTPLAN = 407;
const int MENBTN_DETAILED = 408;
const int MENBTN_AREASEL = 409;
const int MENBTN_TCKCTRL = 410;
const int MENBTN_OVERLAYS = 411;
const int MENBTN_TYPESEL = 412;
const int MENBTN_ALTFILT = 413;
const int MENBTN_HALO = 414;
const int MENBTN_RBL = 415;
const int MENBTN_RINGS = 416;
const int MENBTN_MTT = 417;
const int MENBTN_PTL = 418;
const int MENBTN_PIV = 419;
const int MENBTN_GRID = 420;
const int MENBTN_SEP = 421;
const int MENBTN_POS = 422;
const int MENBTN_QCKLOOK = 423;
const int MENDRP_AREASEL = 424;
const int MENDRP_TCKCTRL = 425;
const int MENDRP_OVERLAYS = 426;
const int MENDRP_TYPESEL = 427;
const int MENTXT_ALTFILT = 428;

/// SETTINGS VARIABLES
const string SET_INBNDX = "InboundX";
const string SET_INBNDY = "InboundY";
const string SET_OTHERSX = "OthersX";
const string SET_OTHERSY = "OthersY";
const string SET_ALTFILT_LOW = "AltFiltLow";
const string SET_ALTFILT_HIGH = "AltFiltHigh";
const string SET_GRID = "GridEnabled";
const string SET_TAGS = "TagsEnabled";
const string SET_QCKLOOK = "QckLookEnabled";
const string SET_OVERLAY = "OverlayEnabled";
const string SET_AREASEL = "SelectedArea";
const string SET_OVERLAYSEL = "SelectedOverlay";
const string SET_POSTYPESEL = "SelectedPosType";

/// ENUMS
// Path type enum
enum class CPathType {
    RTE,
    PIV,
    TCKS
};

// Track direction enum
enum class CTrackDirection {
    UNKNOWN,
    WEST,
    EAST
};

// Type of overlay
enum class COverlayType {
    TCKS_ALL,
    TCKS_EAST,
    TCKS_WEST,
    TCKS_ACTV,
    TCKS_SEL
};

// Conflict status
enum class CConflictStatus {
    OK,
    WARNING,
    CRITICAL
};

// Longitudinal trackstatus 
enum class CTrackStatus {
    NA,
    CROSSING,
    RECIPROCAL,
    OPPOSITE,
    SAME
};

// Message type
enum class CMessageType {
    OK,
    WARNING,
    CRITICAL
};
