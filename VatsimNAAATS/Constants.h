#pragma once
#include <string>
#include <EuroScopePlugIn.h>

using namespace std;
using namespace EuroScopePlugIn;

// Plugin info
const string PLUGIN_NAME = "vNAAATS";
const string PLUGIN_VERSION = "dev";
const string PLUGIN_AUTHOR = "Andrew Ogden github.com/andrewogden1678";
const string PLUGIN_COPYRIGHT = "GPL v3";

// Screen details
#define DISPLAY_NAME "vNAAATS Display"

// Text, margins and padding
const int MEN_FONT_SIZE = 14;
const int BTN_PAD_SIDE = 5;
const int BTN_PAD_TOP = 2;

// Menu bar sizes
const int RECT1_WIDTH = 500;
const int RECT2_WIDTH = 340;
const int RECT3_WIDTH = 160;
const int RECT4_WIDTH = 110;
const int RECT5_WIDTH = 280;
const int RECT6_WIDTH = 100;
const int RECT7_WIDTH = 250;
const int RECT8_WIDTH = 70;

// Menu buttons
const int MENBTN_MISC = 400;
const int MENBTN_MESSAGE = 401;
const int MENBTN_SETUP = 402;
const int MENBTN_TAGS = 403;
const int MENBTN_NOTEPAD = 404;
const int MENBTN_FLIGHTPLAN = 405;
const int MENBTN_ADSC = 406;
const int MENBTN_DESTAPT = 407;
const int MENBTN_FREQUENCY = 408;