#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include "BaseWindow.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CFlightPlanWindow : public CBaseWindow
{
	private: 
		CAircraftFlightPlan primedPlan;
		CAircraftFlightPlan copiedPlan;
		map<int, POINT> subWindowPositions;
		map<int, string> restrictionSelections;
		int selectedRestriction = 0;
	public:
		// Inherited methods
		CFlightPlanWindow(POINT topLeft);
		virtual void MakeWindowItems();
		virtual void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		CRect RenderDataPanel(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft, bool isCopy);
		void RenderConflictWindow(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft);
		void RenderMessageWindow(CDC* dc, Graphics* g, CRadarScreen* screen, POINT bottomLeft);
		void RenderClearanceWindow(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft);
		void RenderManEntryWindow(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft);
		void RenderCoordModal(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft);
		void RenderHistoryModal(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft);
		void RenderATCRestrictModal(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft);
		void RenderExchangeModal(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft);
		void MoveSubWindow(int id, POINT topLeft);

		bool IsButtonPressed(int id);

		// Button clicks
		virtual void ButtonDown(int id);
		virtual void ButtonUp(int id);
		virtual void ButtonPress(int id);
		virtual void ButtonUnpress(int id);
		virtual void SetButtonState(int id, CInputState state);

		// Fill data
		void UpdateData(CRadarScreen* screen, CAircraftFlightPlan status);
		void OnCloseFlightPlanWindow();
		int ChangeDataPoint(CRadarScreen* screen, int data, string str); // Master method to deal with all cases of text input

		// Panel states
		bool IsData = true;
		bool IsCopyMade = false;
		bool IsConflictWindow = false;
		bool IsClearanceOpen = false;
		bool IsManualEntryOpen = false;
		bool IsMessageOpen = false;
		bool IsCoordOpen = false;
		bool IsHistoryOpen = false;
		bool IsATCRestrictionsOpen = false;
		bool IsTransferOpen = false;

		// Subwindow definitions
		static const int SUBWIN_ATCR = 400;
		static const int SUBWIN_COORD = 401;
		static const int SUBWIN_HIST = 402;
		static const int SUBWIN_XCHANGE = 403;

		// Button definitions
		static const int BTN_CLOSE = 0;
		static const int BTN_COPY = 1;
		static const int BTN_UNCLEAR = 2;
		static const int BTN_COORD = 3;
		static const int BTN_MANENTRY = 4;
		static const int BTN_PROBE = 5;
		static const int BTN_DELETE = 6;
		static const int BTN_ADS = 7;
		static const int BTN_READBK = 8;
		static const int BTN_MSG = 9;
		static const int BTN_HIST = 10;
		static const int BTN_SAVE = 11;
		static const int BTN_ATCR = 12;
		static const int BTN_ATCR_CPY = 13;
		static const int BTN_MSG_REMOVE = 14;
		static const int BTN_MSG_DONE = 15;
		static const int BTN_MSG_CLOSE = 16;
		static const int BTN_MSG_REQUEUE = 17;
		static const int BTN_MSG_FORWARD = 18;
		static const int BTN_CONF_ACCCL = 19;
		static const int BTN_CONF_MANCL = 20;
		static const int BTN_CONF_COORD = 21;
		static const int BTN_CONF_CLOSE = 22;
		static const int BTN_MAN_CANCEL = 23;
		static const int BTN_MAN_SUBMIT = 24;
		static const int BTN_CLRC_READBK = 25;		
		static const int BTN_CLRC_SEND = 26;
		static const int BTN_CLRC_VOICE = 27;
		static const int BTN_CLRC_REJECT = 28;
		static const int BTN_ATCR_ADD = 29;
		static const int BTN_ATCR_CANCEL = 30;
		static const int BTN_ATCR_OK = 31;
		static const int BTN_XCHANGE_NOTIFY = 32;
		static const int BTN_XCHANGE_CLOSE = 33;
		static const int BTN_XCHANGE_ACCEPT = 34;
		static const int BTN_XCHANGE_TRANSFER = 35;
		static const int BTN_XCHANGE_REJECT = 36;
		static const int BTN_XCHANGE_TRACK = 37;
		static const int BTN_COORD_CLOSE = 38;
		static const int BTN_COORD_SENDOK = 39;
		static const int BTN_HIST_CLOSE = 40;

		// Dropdown definitions
		static const int DRP_ATCR = 200;
		static const int DRP_ATCR_CPY = 201;

		// Text definitions
		static const int TXT_ACID = 100;
		static const int TXT_TYPE = 101;
		static const int TXT_DEPART = 102;
		static const int TXT_ETD = 103;
		static const int TXT_SELCAL = 104;
		static const int TXT_DATALINK = 105;
		static const int TXT_COMMS = 106;
		static const int TXT_OWNERSHIP = 107;
		static const int TXT_SPD = 108;
		static const int TXT_LEVEL = 109;
		static const int TXT_DEST = 110;
		static const int TXT_TCK = 111;
		static const int TXT_STATE = 112;
		static const int TXT_SPD_CPY = 113;
		static const int TXT_LEVEL_CPY = 114;
		static const int TXT_DEST_CPY = 115;
		static const int TXT_TCK_CPY = 116;
		static const int TXT_STATE_CPY = 117;
		static const int TXT_MAN_FL = 118;
		static const int TXT_MAN_SPD = 119;
		static const int TXT_MAN_TCK = 120;
		static const int TXT_MAN_DEST = 121;
		static const int TXT_MAN_EP = 122;
		static const int TXT_MAN_EPTIME = 123;
		static const int TXT_XCHANGE_CURRENT = 124;
		static const int TXT_XCHANGE_NEXT = 125;

		// Checkbox definitions
		static const int CHK_CLRC_ORCA = 300;
		static const int CHK_CLRC_VOX = 301;
		static const int CHK_CLRC_CPDLC = 302;
		static const int CHK_CLRC_TXT = 303;
		static const int CHK_COORD_CZQOV = 304;
		static const int CHK_COORD_EGGXV = 305;
		static const int CHK_COORD_BIRDV = 306;
		static const int CHK_COORD_ZNYV = 307;
		static const int CHK_COORD_LPPOV = 308;
		static const int CHK_COORD_EISNV = 309;
		static const int CHK_COORD_CZQXV = 310;
		static const int CHK_COORD_CZULV = 311;
		static const int CHK_COORD_EGPXV = 312;
		static const int CHK_COORD_LFRRV = 313;
		static const int CHK_COORD_PLANV = 314;
		static const int CHK_COORD_ENRV = 315;

		static const int CHK_COORD_CZQOT = 360; // We start here from 360 because we need to be able to know that this is manual
		static const int CHK_COORD_EGGXT = 361;
		static const int CHK_COORD_BIRDT = 362;
		static const int CHK_COORD_ZNYT = 363;
		static const int CHK_COORD_LPPOT = 364;
		static const int CHK_COORD_EISNT = 365;
		static const int CHK_COORD_CZQXT = 366;
		static const int CHK_COORD_CZULT = 367;
		static const int CHK_COORD_EGPXT = 368;
		static const int CHK_COORD_LFRRT = 369;
		static const int CHK_COORD_PLANT = 370;
		static const int CHK_COORD_ENRT = 371;

		// Selection definitions
		static const int SEL_ATCR_LCHG = 400;
		static const int SEL_ATCR_MCHG = 401;
		static const int SEL_ATCR_EPC = 402;
		static const int SEL_ATCR_RERUTE = 403;
		static const int SEL_ATCR_RTD = 404;
		static const int SEL_ATCR_UNABLE = 405;
		static const int SEL_ATCR_ATA = 406;
		static const int SEL_ATCR_ATB = 407;
		static const int SEL_ATCR_XAT = 408;
		static const int SEL_ATCR_INT = 409;

		// Scroll bar definitions
		static const int SCRL_DATA = 500;
		static const int SCRL_CPY = 501;
		static const int SCRL_CONF_X = 502;
		static const int SCRL_CONF_Y = 503;
		static const int SCRL_HIST = 504;
		static const int SCRL_MSG = 505;
		static const int SCRL_CLRC = 506;
		static const int SCRL_CLRC_XTRA = 507;
		static const int SCRL_MANENTRY = 508;
		static const int SCRL_COORD_STATIONS = 509;
		static const int SCRL_COORD_HIST = 510;
		static const int SCRL_XCHANGE = 511;
};

