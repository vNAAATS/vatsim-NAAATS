#pragma once
#include <string>
#include <EuroScopePlugIn.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

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
        "GONAN",
        "ATSUR"
};

/// VALUES
// Plugin info
const string PLUGIN_NAME = "vNAAATS";
const string PLUGIN_VERSION = "v1.3";
const string PLUGIN_AUTHOR = "Andrew Ogden github.com/andrewogden1678";
const string PLUGIN_COPYRIGHT = "(C) 2021 Gander Oceanic vNAAATS";
const bool IS_ALPHA = false;
const bool DEBUG_MODE = true;

// Sector file & geo constants
const int SECTELEMENT_COORD_IDX = 7;
const int RADIUS_EARTH_NM = 3440;

// Screen details
#define DISPLAY_NAME "vNAAATS Display"

// Text, margins and padding
const int MEN_FONT_SIZE = 16;
const int BTN_PAD_SIDE = 4;
const int BTN_PAD_TOP = 6;

// Lists
const int LIST_INBOUND_WIDTH = 400;
const int LIST_OTHERS_WIDTH = 110;
const int LIST_RCLS_WIDTH = 110;
const int LIST_CONFLICT_WIDTH = 150;

// Menu bar
const int MENBAR_HEIGHT = 100;
const int MENBAR_BTN_HEIGHT = 30;
const int RECT1_WIDTH = 500;
const int RECT2_WIDTH = 340;
const int RECT3_WIDTH = 160;
const int RECT4_WIDTH = 100;
const int RECT5_WIDTH = 250;
const int RECT6_WIDTH = 100;
const int RECT7_WIDTH = 180;
const int RECT8_WIDTH = 190;

// Window sizes
const int WINSZ_TITLEBAR_HEIGHT = 20;
const int WINSZ_TCKINFO_WIDTH = 500;
const int WINSZ_TCKINFO_HEIGHT = 250;
const int WINSZ_FLTPLN_WIDTH = 540;
const int WINSZ_FLTPLN_HEIGHT_INIT = 175;
const int WINSZ_FLTPLN_HEIGHT_DATA = 190;
const int WINSZ_FLTPLN_HEIGHT_MANETRY = 175;
const int WINSZ_FLTPLN_HEIGHT_CPY = 190;
const int WINSZ_FLTPLN_HEIGHT_MSG = 175;
const int WINSZ_FLTPLN_HEIGHT_XTRA = 165;
const int WINSZ_FLTPLN_WIDTH_MDL = 375;
const int WINSZ_FLTPLN_WIDTH_COORD = 350;
const int WINSZ_FLTPLN_HEIGHT_COORD = 280;
const int WINSZ_FLTPLN_HEIGHT_ATCR = 260;
const int WINSZ_FLTPLN_HEIGHT_HIST = 250;
const int WINSZ_FLTPLN_WIDTH_HIST = 500;
const int WINSZ_FLTPLN_HEIGHT_TSFR = 260;
const int WINSZ_MSG_WIDTH = 550;
const int WINSZ_MSG_HEIGHT = 315;
const int WINSZ_NP_WIDTH = 250;
const int WINSZ_NP_HEIGHT = 300;

// Conflict
const int SEPTOOL_TIME = 2700; // 45 minutes
const int STCA_TIME = 480; // 8 minutes

/// OBJECT HANDLES
// Screen
const int RADAR_SCREEN = -1;
const int SCREEN_TAG = 1;
const int SCREEN_TAG_CS = 11;
const int SCREEN_TAG_CS_BTN = 12;
const int MENBAR = 2;
const int WINDOW = 3;

// Window object handles (for buttons, etc)
const int WIN_TCKINFO = 101;
const int WIN_FLTPLN = 102;
const int WIN_SCROLLBAR = 103;
const int WIN_MSG = 104;
const int WIN_NOTEPAD = 105;
const int WIN_FLTPLN_TSFR = 106;

// Text inputs and functions
const int ALTFILT_TEXT = 200;
const int FUNC_ALTFILT_LOW = 201;
const int FUNC_ALTFILT_HIGH = 202;
const int ACTV_MESSAGE = 203;

// Lists
const int LIST_INBOUND = 300;
const int LIST_OTHERS = 301;
const int LIST_RCLS = 302;
const int LIST_CONFLICT = 303;

// Dropdown
const int DRP_AREA_EGGX = 801;
const int DRP_AREA_CZQX = 802;
const int DRP_AREA_BDBX = 803;
const int DRP_OVL_ALL = 800;
const int DRP_OVL_EAST = 801;
const int DRP_OVL_WEST = 802;
const int DRP_OVL_SEL = 803;
const int DRP_TYPE_DEL = 801;
const int DRP_TYPE_ENR = 802;
const int DRP_TYPE_MULTI = 803;

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

// Button states
enum class CInputState {
    INACTIVE,
    ACTIVE,
    DISABLED
};

// Flight plan mode
enum class CFlightPlanMode {
    NOT_OWNED,
    INIT,
    DATA,
    DATA_COPY,
    CLEARANCE,
    PROBE
};

enum class CMessageType {
    LOG_ON,
    LOG_ON_CONFIRM,
    LOG_ON_REJECT,
    TRANSFER,
    TRANSFER_ACCEPT,
    TRANSFER_REJECT,
    CLEARANCE_REQ,
    CLEARANCE_ISSUE,
    CLEARANCE_REJECT,
    REVISION_REQ,
    REVISION_ISSUE,
    REVISION_REJECT,
    WILCO,
    ROGER,
    UNABLE
};

enum class CRestrictionType {
    LCHG,
    MCHG,
    EPC,
    RERUTE,
    UNABLE,
    RTD,
    ATA,
    ATB,
    XAT,
    INT
};

enum class CRadarTargetMode {
    PRIMARY, // Asterisk (0)
    SECONDARY_S, // Diamond with line (1)
    SECONDARY_C, // Star (2)
    ADS_B, // Airplane icon (also for cleared aircraft) (3)
};

enum class CLogType {
    INIT, // Initialisation message
    NORM, // General log item
    WARN, // Warning
    ERR, // Handled exception
    EXC, // Unhandled exception
};