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
namespace User_Interface {
	/// This is the User_Interface namespace. If it draws onto the screen, in most cases, the code can be found in here.
	/// Each User_Interface element has a specific range of IDs assigned to them associated with a location 
	/// (usually a specific window) so that each element can have an unique ID.
	///

	/// ID ranges:
	/// - (-1)-9 (LOCATION IDS (LOC))
	/// - 10-49 (WINDOWS AND MODALS)
	/// - 50-59 (LISTS)
	/// - 60-69 (CUSTOM COMPONENTS)
	/// - 100-199 (BUTTON)
	/// - 200-299 (DROPDOWN) and 2000-2099 (DROPDOWN ITEM)
	/// - 300-399 (TEXT INPUT)
	/// - 400-499 (CHECKBOX)
	/// - 500-599 (SCROLL BAR)

	// For empty spaces in element matrices
	const int kVoid = -1; 

	// RESERVED LOC IDs
	const int kScreen = 0;
	const int kTarget = 1;
	const int kTag = 2;

	/// *** CUSTOM COMPONENTS *** \\\
	// CONSTANTS
	const int kMenuBarId = 60;

	/// *** WINDOWS *** \\\
	// CONSTANTS
	const int kFlightPlanWindowId = 10;
	const int kTrackInfoWindow = 11;
	const int kSetupWindow = 12;
	const int kNotePadWindow = 13;

	/// *** LISTS *** \\\
	// CONSTANTS
	const int kInboundListId = 50;
	const int kOtherListId = 51;
	const int kConflictListId = 52;

	// ELEMENT TYPES
	enum class EElementType {
		kButton,
		kCheckBox,
		kDropDown,
		kDropDownElement, // ?
		kInput,
		kRichText,
		kScrollBar,
		kContextMenu
	};

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

	// CONTENT ALIGNMENT
	enum class EAlignment {
		kLeftUp,
		kCentreUp,
		kRightUp,
		kLeft,
		kCentre,
		kRight,
		kLeftDown,
		kCentreDown,
		kRightDown
	};
}