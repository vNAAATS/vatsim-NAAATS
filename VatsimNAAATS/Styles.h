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
	const Color WarningYellow(221, 213, 88);
	const Color CriticalRed(195, 67, 67);
}

// Font familes
namespace FontFamilies {
	constexpr WCHAR* NormalFont = L"Arial";
	constexpr WCHAR* ATCFont = L"vNAAATS";
	constexpr WCHAR* MonoFont = L"Lucida Console";
}

// Fonts
class FontSelector 
{
	public:
		static void SelectNormalFont(int size, CDC* dc) {
			CFont font;
			LOGFONT lFont;

			// Get face
			memset(&lFont, 0, sizeof(LOGFONT));
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
			font.DeleteObject();
		}

		static void SelectATCFont(int size, CDC* dc) {
			CFont font;
			LOGFONT lFont;

			// Get face
			AddFontResourceEx("vNAAATS.ttf", FR_PRIVATE, 0);
			memset(&lFont, 0, sizeof(LOGFONT));
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
			font.DeleteObject();
		}

		static void SelectMonoFont(int size, CDC* dc) {
			CFont font;
			LOGFONT lFont;

			// Get the face
			memset(&lFont, 0, sizeof(LOGFONT));
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
			font.DeleteObject();
		}
};