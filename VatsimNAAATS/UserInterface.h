#pragma once
#include "EuroScopePlugIn.h"
#include <gdiplus.h>
#include <utility>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

// CUIElement base class and other UI structs
namespace UserInterface {
	/// This is the UserInterface namespace. If it draws onto the screen, in most cases, the code can be found in here.
	/// Each UserInterface element has a specific range of IDs assigned to them associated with a location 
	/// (usually a specific window) so that each element can have an unique ID.
	///
	/// ID ranges:
	/// - 0-10 (RESERVED)
	/// - 10-99 (Button)
	/// - 100-199 (Dropdown) and 1000-1099 (Dropdown items)
	/// - 200-299 (Text input)
	/// - 300-399 (Checkbox)
	/// - 400-499 (Scroll bar)

	/// WINDOW FORWARD DECLARATIONS
	class CUIWindow; // Base
	class CFlightPlanWindow;
	class CTrackInfoWindow;
	class CSetupWindow;
	class CNotePadWindow;

	/// LIST FORWARD DECLARATIONS
	class CUIList; // Base
	class CInboundList;
	class COtherList;

	/// ELEMENT FORWARD DECLARATIONS
	class CUIElement; // Base
	class CUIButton;
	class CUICheckbox;
	class CUIDropDown;
	class CUIInput;
	class CUIScrollBar;

	// ELEMENT STATES
	enum class CElementState {
		kInactive,
		kActive,
		kDisabled
	};
}