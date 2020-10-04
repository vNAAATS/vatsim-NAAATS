#pragma once
#include "pch.h"
#include "Constants.h"
#include <gdiplus.h>

using namespace Gdiplus;

// Colours for the program
namespace Colours {
	const Color TextWhite(255, 255, 255);
	const Color Black(0, 0, 0);
	const Color Grey(237, 237, 237);
	const Color TargetOrange(255, 128, 0);
	const Color LightGreen(81, 188, 161);
	const Color ScreenBlue(59, 110, 179);
	const Color BevelLight(110, 149, 203);
	const Color BevelDark(41, 80, 133);
	const Color GreenBevelLight(76, 191, 196);
	const Color GreenBevelDark(33, 120, 144);
	const Color ButtonPressed(45, 69, 90);
	const Color WindowBorder(187, 214, 238);
	const Color WarningYellow(166, 161, 81);
	const Color CriticalRed(180, 21, 21);
}

// Fonts
class FontSelector 
{
	public:
		static void SelectNormalFont(int size, CDC* dc) {
			CFont font;
			LOGFONT lFont;

			// Clear out
			memset(&lFont, 0, sizeof(LOGFONT));
			// Get the face
			strcpy_s(lFont.lfFaceName, _T("Arial"));
			// Size
			lFont.lfHeight = size;
			// Normal weight
			lFont.lfWeight = FW_SEMIBOLD;
			// Finally create the font
			font.CreateFontIndirect(&lFont);

			// Select the object
			dc->SelectObject(&font);

			// Cleanup
			DeleteObject(font);
		}

		static void SelectATCFont(int size, CDC* dc) {
			CFont font;
			LOGFONT lFont;

			// Get font
			AddFontResourceEx("vNAAATS.ttf", FR_PRIVATE, 0);
			// Clear out
			memset(&lFont, 0, sizeof(LOGFONT));
			// Get the face
			strcpy_s(lFont.lfFaceName, _T("vNAAATS"));
			// Size
			lFont.lfHeight = size;
			// Normal weight
			lFont.lfWeight = FW_REGULAR;
			// Finally create the font
			font.CreateFontIndirect(&lFont);

			// Select the object
			dc->SelectObject(&font);

			// Cleanup
			DeleteObject(font);
		}

		static void SelectMonoFont(int size, CDC* dc) {
			CFont font;
			LOGFONT lFont;

			// Clear out
			memset(&lFont, 0, sizeof(LOGFONT));
			// Get the face
			strcpy_s(lFont.lfFaceName, _T("Lucida Console"));
			// Size
			lFont.lfHeight = size;
			// Normal weight
			lFont.lfWeight = FW_SEMIBOLD;
			// Finally create the font
			font.CreateFontIndirect(&lFont);

			// Select the object
			dc->SelectObject(&font);

			// Cleanup
			DeleteObject(font);
		}
};