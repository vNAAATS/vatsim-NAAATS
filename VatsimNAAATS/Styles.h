#pragma once
#include "pch.h"
#include "Constants.h"
#include <iostream>
#include <gdiplus.h>
#include "Utils.h"
#include "Logger.h"

using namespace CUtils;
using namespace Gdiplus;

// Colours for the program
namespace Colours {
	const Color TextWhite(255, 255, 255);
	const Color Disabled(164, 185, 215);
	const Color Black(0, 0, 0);
	const Color Grey(237, 237, 237);
	const Color DarkGrey(182, 182, 182);
	const Color TargetOrange(255, 128, 0);
	const Color TargetBlue(130, 175, 192);
	const Color LightGreen(81, 188, 161);
	const Color ScreenBlue(59, 110, 179);
	const Color LightBackground(165, 199, 249);
	const Color RouteBox(195, 219, 255);
	const Color BevelLight(110, 149, 203);
	const Color BevelDark(41, 80, 133);
	const Color GreenBevelLight(76, 191, 196);
	const Color GreenBevelDark(33, 120, 144);
	const Color ButtonPressed(45, 69, 90);
	const Color WindowBorder(187, 214, 238);
	const Color WarningYellow(240, 231, 76);
	const Color CriticalRed(195, 67, 67);
	const Color CriticalRedLight(225, 96, 96);
}

// Fonts
class CFontSelector
{
public:
	static int InitialiseFonts() {
		// So that fonts are only ever initialised once
		if (fontsInitialised) return 0;

		LOGFONT lFont;

		// Get face for normal font
		memset(&lFont, 0, sizeof(LOGFONT));
		strcpy_s(lFont.lfFaceName, _T("Arial"));
		// Normal weight
		lFont.lfWeight = FW_SEMIBOLD;
		// Size 14
		lFont.lfHeight = 14;
		normalFont14.CreateFontIndirect(&lFont);
		// Size 15
		lFont.lfHeight = 15;
		normalFont15.CreateFontIndirect(&lFont);
		// Size 16
		lFont.lfHeight = 16;
		normalFont16.CreateFontIndirect(&lFont);
		// Size 30
		lFont.lfHeight = 30;
		normalFont30.CreateFontIndirect(&lFont);

		// Get the face
		strcpy_s(lFont.lfFaceName, _T("Lucida Console"));
		// Normal weight
		lFont.lfWeight = FW_SEMIBOLD;
		// Size 12
		lFont.lfHeight = 12;
		monoFont12.CreateFontIndirect(&lFont);
		// Size 14
		lFont.lfHeight = 14;
		monoFont14.CreateFontIndirect(&lFont);
		// Size 15
		lFont.lfHeight = 15;
		monoFont15.CreateFontIndirect(&lFont);

		// Get for ATC font
		AddFontResourceEx("9x15B.bdf", FR_PRIVATE, 0);
		strcpy_s(lFont.lfFaceName, _T("FixedBold"));
		// Normal weight
		lFont.lfWeight = FW_REGULAR;
		// Size 14
		lFont.lfHeight = 14;
		atcFont14.CreateFontIndirect(&lFont);
		// Size 15
		lFont.lfHeight = 15;
		atcFont15.CreateFontIndirect(&lFont);
		// Size 16
		lFont.lfHeight = 16;
		atcFont16.CreateFontIndirect(&lFont);
		// Size 18
		lFont.lfHeight = 18;
		atcFont18.CreateFontIndirect(&lFont);

		// Initialised
		fontsInitialised = true;
		CLogger::Log(CLogType::NORM, "Fonts initialised.", "CFontSelector");
		return 0;
	}

	static void SelectNormalFont(int size, CDC* dc) {
		// Select font based on font size
		if (size == 14) {
			dc->SelectObject(normalFont14);
		}
		else if (size == 15) {
			dc->SelectObject(normalFont15);
		}
		else if (size == 16) {
			dc->SelectObject(normalFont16);
		}
		else if (size == 30) {
			dc->SelectObject(normalFont30);
		}
	}

	static void SelectMonoFont(int size, CDC* dc) {
		// Select font based on font size
		if (size == 12) {
			dc->SelectObject(monoFont12);
		}
		else if (size == 14) {
			dc->SelectObject(monoFont14);
		}
		else if (size == 15) {
			dc->SelectObject(monoFont15);
		}
	}

	static void SelectATCFont(int size, CDC* dc) {
		// Select font based on font size
		if (size == 14) {
			dc->SelectObject(atcFont14);
		}
		if (size == 15) {
			dc->SelectObject(atcFont15);
		}
		else if (size == 16) {
			dc->SelectObject(atcFont16);
		}
		else if (size == 18) {
			dc->SelectObject(atcFont18);
		}
	}

	private:
		static CFont normalFont14;
		static CFont normalFont15;
		static CFont normalFont16;
		static CFont normalFont30;
		static CFont monoFont12;
		static CFont monoFont14;
		static CFont monoFont15;
		static CFont atcFont14;
		static CFont atcFont15;
		static CFont atcFont16;
		static CFont atcFont18;
		static bool fontsInitialised; // So that we don't try and re-initialise all the fonts
};