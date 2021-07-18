#pragma once
#include "../UIComponentBase.h"
namespace User_Interface {
	// SIZING
	const int kMenuBarHeight = 100;

	// MenuBar class
	class CMenuBar : public CUIComponentBase
	{
		public:
			// Construct
			inline CMenuBar(int id, int locId) : CUIComponentBase(id, locId) {
			
				// Register all the window elements
				RegisterElements();
			};

			// Inherited methods
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void RegisterElements();
			virtual void Interact(int id, EElementType type);

			/// ELEMENT IDENTIFICATION \\\
			// BUTTONS
			static const int kBtnSetup = 100;
			static const int kBtnNotePad = 101;
			static const int kBtnContracts = 102;
			static const int kBtnTrackInfo = 103;
			static const int kBtnMisc = 104;
			static const int kBtnMessage = 105;
			static const int kBtnTags = 106;
			static const int kBtnFlightPlan = 107;
			static const int kBtnDetailed = 108;

			private:
			// Let's be friends <3
			friend class CMenuBarController;
			// Called in RenderContent
			virtual void RenderBase(CDC* dc, Graphics* g, CRadarScreen* screen);
			// Height of the bar itself
			int Height_ = kMenuBarHeight;
			// Panel sizes and positions. We set the x coordinate at runtime.
			pair<int, POINT> kPanels_[9] = { std::make_pair(500, POINT{ 0, 0 }), std::make_pair(340, POINT{ 0, 0 }),
				std::make_pair(160, POINT{ 0, 0 }), std::make_pair(100, POINT{ 0, 0 }), std::make_pair(250, POINT{ 0, 0 }),
				std::make_pair(100, POINT{ 0, 0 }), std::make_pair(180, POINT{ 0, 0 }), std::make_pair(190, POINT{ 0, 0 }), 
				std::make_pair(91, POINT{ 0, 0 }) };
			// Element matrices for each panel (In order of render)
			const int panel1Matrix_[2][6] = { { kVoid, kVoid, kBtnSetup, kBtnNotePad, kBtnContracts, kBtnTrackInfo },
											{ kBtnMisc, kBtnMessage, kBtnTags, kBtnFlightPlan, kBtnDetailed, kVoid } };
	};

	class CMenuBarController : public CUIComponentController {
		public:
			virtual void OnButtonDown();
			virtual void OnButtonUp();
			virtual void OnDoubleClick();
	};
}