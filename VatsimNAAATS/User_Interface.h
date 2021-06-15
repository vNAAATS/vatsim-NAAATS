#pragma once
#include "EuroScopePlugIn.h"
#include "UIBase.h"
#include <gdiplus.h>
#include <utility>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

// CUIElement base class and other UI structs
namespace User_Interface {
	/// This is the User_Interface namespace. If it draws onto the screen, in most cases, the code can be found in here.
	/// Each User_Interface element has a specific range of IDs assigned to them associated with a location 
	/// (usually a specific window) so that each element can have an unique ID.
	///
	/// ID ranges:
	/// - 0-10 (RESERVED)
	/// - 10-99 (Button)
	/// - 100-199 (Dropdown) and 1000-1099 (Dropdown items)
	/// - 200-299 (Text input)
	/// - 300-399 (Checkbox)
	/// - 400-499 (Scroll bar)

	/// *** WINDOWs *** \\\
	// FORWARD DECLARATIONS
	class CUIWindow; // Base
	class CFlightPlanWindow;
	class CTrackInfoWindow;
	class CSetupWindow;
	class CNotePadWindow;
	// CONSTANTS
	const int kWindowTitlebarHeight = 20;

	/// *** LISTS *** \\\
	// FORWARD DECLARATIONS
	class CUIList; // Base
	class CInboundList;
	class COtherList;
	// CONSTANTS
	const int kListHandleHeight = 15;

	/// *** ELEMENTs *** \\\
	// FORWARD DECLARATIONS
	class CUIElement; // Base
	class CUIButton;
	class CUICheckbox;
	class CUIDropDown;
	class CUIInput;
	class CUIScrollBar;
	class CUIContextMenu;

	// ELEMENT STATES
	enum class EElementState {
		kInactive,
		kActive,
		kDisabled
	};

	// LIST CONTENT TYPE
	enum class EListColumnType {
		kString,
		kInteger
	};
}