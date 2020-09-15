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

// Text, margins, padding and sizes
const int MEN_FONT_SIZE = 16;
const int BTN_PAD_SIDE = 4;
const int BTN_PAD_TOP = 6;

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
