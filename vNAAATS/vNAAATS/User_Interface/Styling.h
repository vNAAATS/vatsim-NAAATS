#pragma once
#include "../pch.h"
#include "User_Interface.h"
#include <gdiplus.h>

using namespace std;
using namespace User_Interface;
using namespace Gdiplus;

// UI Styling namespace (probably will be extracted from a config file at some stage)
namespace Styling {
	namespace Colours {
		// Monochromes
		const Color kWhite(255, 255, 255);
		const Color kBlack(0, 0, 0);
		const Color kGrey(237, 237, 237);
		const Color kDarkGrey(182, 182, 182);

		// User Interface
		const Color kUI(59, 110, 179); // Main Blue
		const Color kUILight(187, 214, 238); // Window borders & certain other applications
		const Color kUIDark(45, 69, 90); // Button presses, checkbox checked backgrounds, etc
		const Color kUIBevelLight(110, 149, 203);
		const Color kUIBevelDark(41, 80, 133);
		const Color kUIDisabled(164, 185, 215);

		// Situation Display
		const Color kTargetUncontrolled(130, 175, 192);
		const Color kTargetControlled(255, 128, 0);
		const Color kTransfer(0, 0, 0); // TODO
		const Color kOutdatedRoute(0, 0, 0); // TODO
		const Color kRAMWarning(0, 0, 0); // TODO
		const Color kCLAMWarning(0, 0, 0); // TODO
		const Color kAPWWarning(0, 0, 0); // TODO
		const Color kConflictWarning(240, 231, 76);
		const Color kConflictCritical(225, 96, 96);
	}

	namespace Typefaces {
		// Typeface enum
		enum class ETypeface {
			kRegular14,
			kRegular15,
			kRegular16,
			kRegular30,
			kMono12,
			kMono14,
			kMono15,
			kHMI14,
			kHMI15,
			kHMI16,
			kHMI18
		};

		// Font manager
		class CTextManager {
			public:
				// Fonts (all private)
				static CFont regularFont14;
				static CFont regularFont15;
				static CFont regularFont16;
				static CFont regularFont30;
				static CFont monoFont12;
				static CFont monoFont14;
				static CFont monoFont15;
				static CFont hmiFont14;
				static CFont hmiFont15;
				static CFont hmiFont16;
				static CFont hmiFont18;

			
				// Initialise the fonts
				static inline int InitialiseFonts() {
					LOGFONT lFont;

					// Get face for normal font
					memset(&lFont, 0, sizeof(LOGFONT));
					strcpy_s(lFont.lfFaceName, _T("Arial"));
					// Normal weight
					lFont.lfWeight = FW_SEMIBOLD;
					// Size 14
					lFont.lfHeight = 14;
					regularFont14.CreateFontIndirect(&lFont);
					// Size 15
					lFont.lfHeight = 15;
					regularFont15.CreateFontIndirect(&lFont);
					// Size 16
					lFont.lfHeight = 16;
					regularFont16.CreateFontIndirect(&lFont);
					// Size 30
					lFont.lfHeight = 30;
					regularFont30.CreateFontIndirect(&lFont);

					// Get the face for mono font
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
					hmiFont14.CreateFontIndirect(&lFont);
					// Size 15
					lFont.lfHeight = 15;
					hmiFont15.CreateFontIndirect(&lFont);
					// Size 16
					lFont.lfHeight = 16;
					hmiFont16.CreateFontIndirect(&lFont);
					// Size 18
					lFont.lfHeight = 18;
					hmiFont18.CreateFontIndirect(&lFont);

					return 0;
				};

				// Select a font
				static inline void SelectFont(CDC* dc, ETypeface typeface) {
					switch (typeface) {
					case ETypeface::kRegular14:
						dc->SelectObject(regularFont14);
						break;
					case ETypeface::kRegular15:
						dc->SelectObject(regularFont15);
						break;
					case ETypeface::kRegular16:
						dc->SelectObject(regularFont16);
						break;
					case ETypeface::kRegular30:
						dc->SelectObject(regularFont30);
						break;
					case ETypeface::kMono12:
						dc->SelectObject(monoFont12);
						break;
					case ETypeface::kMono14:
						dc->SelectObject(monoFont14);
						break;
					case ETypeface::kMono15:
						dc->SelectObject(monoFont15);
						break;
					case ETypeface::kHMI14:
						dc->SelectObject(hmiFont14);
						break;
					case ETypeface::kHMI15:
						dc->SelectObject(hmiFont15);
						break;
					case ETypeface::kHMI16:
						dc->SelectObject(hmiFont16);
						break;
					case ETypeface::kHMI18:
						dc->SelectObject(hmiFont18);
						break;
					default:
						dc->SelectObject(regularFont14);
						break;
					}
				};

				// Render a text label
				static void RenderTextLabel(CDC* dc, string str, POINT topLeft, Typefaces::ETypeface typeface, EAlignment textAlign, Color textColor = Colours::kWhite)
				{
					// Save context for later
					int sDC = dc->SaveDC();

					// Select font
					Typefaces::CTextManager::SelectFont(dc, typeface);

					// Select colour
					dc->SetTextColor(textColor.ToCOLORREF());

					// Text align
					if (textAlign == EAlignment::kRight) {
						dc->SetTextAlign(TA_RIGHT);
					}
					else if (textAlign == EAlignment::kCentre) {
						dc->SetTextAlign(TA_CENTER);
					}
					else { // Everything else
						dc->SetTextAlign(TA_LEFT);
					}

					// Text output
					dc->TextOutA(topLeft.x, topLeft.y, str.c_str());

					// Restore context
					dc->RestoreDC(sDC);
				}
		};
	}
}