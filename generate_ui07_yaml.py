import yaml
import os
import hashlib

def get_md5_hash(content):
    return hashlib.md5(content.encode('utf-8')).hexdigest()

def get_content_lite(content, lines=200):
    return "\\n".join(content.splitlines()[:lines])

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if "\\n" in data or "\n" in data: # Check for newlines
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

# Content of the C++ files
cpp_files_content = {
    "wxwidgets/palette_house.cpp": """//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "palette_house.h"

#include "settings.h"

#include "brush.h"
#include "editor.h"
#include "map.h"

#include "application.h"
#include "map_display.h"

#include "house_brush.h"
#include "house_exit_brush.h"
#include "spawn_brush.h"

// ============================================================================
// House palette

BEGIN_EVENT_TABLE(HousePalettePanel, PalettePanel)
EVT_TIMER(PALETTE_LAYOUT_FIX_TIMER, HousePalettePanel::OnLayoutFixTimer)

EVT_CHOICE(PALETTE_HOUSE_TOWN_CHOICE, HousePalettePanel::OnTownChange)

EVT_LISTBOX(PALETTE_HOUSE_LISTBOX, HousePalettePanel::OnListBoxChange)
EVT_LISTBOX_DCLICK(PALETTE_HOUSE_LISTBOX, HousePalettePanel::OnListBoxDoubleClick)
EVT_CONTEXT_MENU(HousePalettePanel::OnListBoxContextMenu)

EVT_BUTTON(PALETTE_HOUSE_ADD_HOUSE, HousePalettePanel::OnClickAddHouse)
EVT_BUTTON(PALETTE_HOUSE_EDIT_HOUSE, HousePalettePanel::OnClickEditHouse)
EVT_BUTTON(PALETTE_HOUSE_REMOVE_HOUSE, HousePalettePanel::OnClickRemoveHouse)

EVT_TOGGLEBUTTON(PALETTE_HOUSE_BRUSH_BUTTON, HousePalettePanel::OnClickHouseBrushButton)
EVT_TOGGLEBUTTON(PALETTE_HOUSE_SELECT_EXIT_BUTTON, HousePalettePanel::OnClickSelectExitButton)

EVT_MENU(PALETTE_HOUSE_CONTEXT_MOVE_TO_TOWN, HousePalettePanel::OnMoveHouseToTown)
END_EVENT_TABLE()

HousePalettePanel::HousePalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	map(nullptr),
	do_resize_on_display(true),
	fix_size_timer(this, PALETTE_LAYOUT_FIX_TIMER) {
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* tmpsizer;

	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Houses");
	town_choice = newd wxChoice(this, PALETTE_HOUSE_TOWN_CHOICE, wxDefaultPosition, wxDefaultSize, (int)0, (const wxString*)nullptr);
	sidesizer->Add(town_choice, 0, wxEXPAND);

	house_list = newd SortableListBox(this, PALETTE_HOUSE_LISTBOX, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_EXTENDED);
#ifdef __APPLE__
	// Used for detecting a deselect
	house_list->Bind(wxEVT_LEFT_UP, &HousePalettePanel::OnListBoxClick, this);
#endif
	// Bind context menu event to the list box
	house_list->Bind(wxEVT_CONTEXT_MENU, &HousePalettePanel::OnListBoxContextMenu, this);
	sidesizer->Add(house_list, 1, wxEXPAND);

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags sizerFlags(1);
	tmpsizer->Add(add_house_button = newd wxButton(this, PALETTE_HOUSE_ADD_HOUSE, "Add", wxDefaultPosition, wxSize(50, -1)), sizerFlags);
	tmpsizer->Add(edit_house_button = newd wxButton(this, PALETTE_HOUSE_EDIT_HOUSE, "Edit", wxDefaultPosition, wxSize(50, -1)), sizerFlags);
	tmpsizer->Add(remove_house_button = newd wxButton(this, PALETTE_HOUSE_REMOVE_HOUSE, "Remove", wxDefaultPosition, wxSize(70, -1)), sizerFlags);
	sidesizer->Add(tmpsizer, wxSizerFlags(0).Right());

	topsizer->Add(sidesizer, 1, wxEXPAND);

	// Temple position
	sidesizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Brushes", wxDefaultPosition, wxSize(150, 200)), wxVERTICAL);

	// sidesizer->Add(180, 1, wxEXPAND);

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	house_brush_button = newd wxToggleButton(this, PALETTE_HOUSE_BRUSH_BUTTON, "House tiles");
	tmpsizer->Add(house_brush_button);
	sidesizer->Add(tmpsizer, wxSizerFlags(1).Center());

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	select_position_button = newd wxToggleButton(this, PALETTE_HOUSE_SELECT_EXIT_BUTTON, "Select Exit");
	tmpsizer->Add(select_position_button);
	sidesizer->Add(tmpsizer, wxSizerFlags(1).Center());

	topsizer->Add(sidesizer, 0, wxEXPAND);

	SetSizerAndFit(topsizer);

	// Create context menu
	context_menu = newd wxMenu();
	context_menu->Append(PALETTE_HOUSE_CONTEXT_MOVE_TO_TOWN, "Move to Town...");
}

HousePalettePanel::~HousePalettePanel() {
	////
}

void HousePalettePanel::SetMap(Map* m) {
	g_gui.house_brush->setHouse(nullptr);
	map = m;
	OnUpdate();
}

void HousePalettePanel::OnSwitchIn() {
	PalettePanel::OnSwitchIn();
	// Extremely ugly hack to fix layout issue
	if (do_resize_on_display) {
		fix_size_timer.Start(100, true);
		do_resize_on_display = false;
	}
}

void HousePalettePanel::OnLayoutFixTimer(wxTimerEvent& WXUNUSED(event)) {
	wxWindow* w = this;
	while ((w = w->GetParent()) && dynamic_cast<PaletteWindow*>(w) == nullptr)
		;

	if (w) {
		w->SetSize(w->GetSize().GetWidth(), w->GetSize().GetHeight() + 1);
		w->SetSize(w->GetSize().GetWidth(), w->GetSize().GetHeight() - 1);
	}
}

void HousePalettePanel::SelectFirstBrush() {
	SelectHouseBrush();
}

Brush* HousePalettePanel::GetSelectedBrush() const {
	if (select_position_button->GetValue()) {
		House* house = GetCurrentlySelectedHouse();
		if (house) {
			g_gui.house_exit_brush->setHouse(house);
		}
		return (g_gui.house_exit_brush->getHouseID() != 0 ? g_gui.house_exit_brush : nullptr);
	} else if (house_brush_button->GetValue()) {
		g_gui.house_brush->setHouse(GetCurrentlySelectedHouse());
		return (g_gui.house_brush->getHouseID() != 0 ? g_gui.house_brush : nullptr);
	}
	return nullptr;
}

bool HousePalettePanel::SelectBrush(const Brush* whatbrush) {
	if (!whatbrush) {
		return false;
	}

	if (whatbrush->isHouse() && map) {
		const HouseBrush* house_brush = static_cast<const HouseBrush*>(whatbrush);
		for (HouseMap::iterator house_iter = map->houses.begin(); house_iter != map->houses.end(); ++house_iter) {
			if (house_iter->second->getID() == house_brush->getHouseID()) {
				for (uint32_t i = 0; i < town_choice->GetCount(); ++i) {
					Town* town = reinterpret_cast<Town*>(town_choice->GetClientData(i));
					// If it's "No Town" (nullptr) select it, or if it has the same town ID as the house
					if (town == nullptr || town->getID() == house_iter->second->townid) {
						SelectTown(i);
						for (uint32_t j = 0; j < house_list->GetCount(); ++j) {
							if (house_iter->second->getID() == reinterpret_cast<House*>(house_list->GetClientData(j))->getID()) {
								SelectHouse(j);
								return true;
							}
						}
						return true;
					}
				}
			}
		}
	} else if (whatbrush->isSpawn()) {
		SelectExitBrush();
	}
	return false;
}

int HousePalettePanel::GetSelectedBrushSize() const {
	return 0;
}

PaletteType HousePalettePanel::GetType() const {
	return TILESET_HOUSE;
}

void HousePalettePanel::SelectTown(size_t index) {
	ASSERT(town_choice->GetCount() >= index);

	if (map == nullptr || town_choice->GetCount() == 0) {
		// No towns :(
		add_house_button->Enable(false);
	} else {
		Town* what_town = reinterpret_cast<Town*>(town_choice->GetClientData(index));

		// Clear the old houselist
		house_list->Clear();

		for (HouseMap::iterator house_iter = map->houses.begin(); house_iter != map->houses.end(); ++house_iter) {
			if (what_town) {
				if (house_iter->second->townid == what_town->getID()) {
					house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
				}
			} else {
				// "No Town" selected!
				if (map->towns.getTown(house_iter->second->townid) == nullptr) {
					// The town doesn't exist
					house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
				}
			}
		}
		house_list->Sort();

		// Select first house
		SelectHouse(0);
		town_choice->SetSelection(index);
		add_house_button->Enable(what_town != nullptr);
		ASSERT(what_town == nullptr || add_house_button->IsEnabled() || !IsEnabled());
	}
}

void HousePalettePanel::SelectHouse(size_t index) {
	ASSERT(house_list->GetCount() >= index);

	if (house_list->GetCount() > 0) {
		edit_house_button->Enable(true);
		remove_house_button->Enable(true);
		select_position_button->Enable(true);
		house_brush_button->Enable(true);

		// Clear any existing selections first
		for (unsigned int i = 0; i < house_list->GetCount(); ++i) {
			house_list->Deselect(i);
		}

		// Select the house
		house_list->SetSelection(index);
		SelectHouseBrush();
	} else {
		// No houses :(
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
		select_position_button->Enable(false);
		house_brush_button->Enable(false);
	}

	SelectHouseBrush();
	g_gui.RefreshView();
}

House* HousePalettePanel::GetCurrentlySelectedHouse() const {
	wxArrayInt selections;
	if (house_list->GetCount() > 0 && house_list->GetSelections(selections) > 0) {
		// Return the first selected house (for brush operations)
		return reinterpret_cast<House*>(house_list->GetClientData(selections[0]));
	}
	return nullptr;
}

void HousePalettePanel::SelectHouseBrush() {
	if (house_list->GetCount() > 0) {
		house_brush_button->SetValue(true);
		select_position_button->SetValue(false);
	} else {
		house_brush_button->SetValue(false);
		select_position_button->SetValue(false);
	}
}

void HousePalettePanel::SelectExitBrush() {
	if (house_list->GetCount() > 0) {
		house_brush_button->SetValue(false);
		select_position_button->SetValue(true);
	}
}

void HousePalettePanel::OnUpdate() {
	int old_town_selection = town_choice->GetSelection();

	town_choice->Clear();
	house_list->Clear();

	if (map == nullptr) {
		return;
	}

	if (map->towns.count() != 0) {
		// Create choice control
		for (TownMap::iterator town_iter = map->towns.begin(); town_iter != map->towns.end(); ++town_iter) {
			town_choice->Append(wxstr(town_iter->second->getName()), town_iter->second);
		}
		town_choice->Append("No Town", (void*)(nullptr));
		if (old_town_selection <= 0) {
			SelectTown(0);
		} else if ((size_t)old_town_selection <= town_choice->GetCount()) {
			SelectTown(old_town_selection);
		} else {
			SelectTown(old_town_selection - 1);
		}

		house_list->Enable(true);
	} else {
		town_choice->Append("No Town", (void*)(nullptr));
		select_position_button->Enable(false);
		select_position_button->SetValue(false);
		house_brush_button->Enable(false);
		house_brush_button->SetValue(false);
		add_house_button->Enable(false);
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);

		SelectTown(0);
	}
}

void HousePalettePanel::OnTownChange(wxCommandEvent& event) {
	SelectTown(event.GetSelection());
	g_gui.SelectBrush();
}

void HousePalettePanel::OnListBoxChange(wxCommandEvent& event) {
	// Check if there are multiple selections
	wxArrayInt selections;
	int count = house_list->GetSelections(selections);

	if (count == 1) {
		// Only one selection - handle it
		SelectHouse(event.GetSelection());
		g_gui.SelectBrush();
	} else if (count > 1) {
		// Multiple selections - enable/disable buttons appropriately
		edit_house_button->Enable(false); // Can only edit one house at a time
		remove_house_button->Enable(true);
		house_brush_button->Enable(true);
		select_position_button->Enable(true);
	}
}

void HousePalettePanel::OnListBoxDoubleClick(wxCommandEvent& event) {
	House* house = reinterpret_cast<House*>(event.GetClientData());
	// I find it extremly unlikely that one actually wants the exit at 0,0,0, so just treat it as the null value
	if (house && house->getExit() != Position(0, 0, 0)) {
		g_gui.SetScreenCenterPosition(house->getExit());
	}
}

void HousePalettePanel::OnListBoxContextMenu(wxContextMenuEvent& event) {
	if (map == nullptr || house_list->GetCount() == 0) {
		return;
	}

	// Only enable the menu if at least one house is selected
	wxArrayInt selections;
	if (house_list->GetSelections(selections) > 0) {
		// Get mouse position in screen coordinates
		wxPoint position = event.GetPosition();
		// If position is (-1, -1), this means the event was generated from the keyboard (e.g., Shift+F10)
		// In this case, get the position of the first selected item
		if (position == wxPoint(-1, -1)) {
			// Get the client rect of the first selected item
			wxRect rect;
			house_list->GetItemRect(selections[0], rect);
			// Convert to screen coordinates
			position = house_list->ClientToScreen(rect.GetPosition());
		}
		// Show context menu at the proper position
		PopupMenu(context_menu, house_list->ScreenToClient(position));
	}
}

void HousePalettePanel::OnMoveHouseToTown(wxCommandEvent& event) {
	if (map == nullptr || map->towns.count() == 0) {
		return;
	}

	// Get all selected houses
	wxArrayInt selections;
	int count = house_list->GetSelections(selections);

	if (count == 0) {
		return;
	}

	// Create a more informative title based on number of selected houses
	wxString title = count == 1 ? "Move House to Town" : wxString::Format("Move %d Houses to Town", count);

	// Create dialog to select town
	wxDialog* dialog = newd wxDialog(this, wxID_ANY, title, wxDefaultPosition, wxSize(220, 150));
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

	// Create choice control with towns
	wxChoice* town_list = newd wxChoice(dialog, wxID_ANY);
	for (TownMap::const_iterator town_iter = map->towns.begin(); town_iter != map->towns.end(); ++town_iter) {
		town_list->Append(wxstr(town_iter->second->getName()), town_iter->second);
	}

	if (town_list->GetCount() > 0) {
		town_list->SetSelection(0);
	}

	sizer->Add(newd wxStaticText(dialog, wxID_ANY, "Select destination town:"), 0, wxEXPAND | wxALL, 5);
	sizer->Add(town_list, 0, wxEXPAND | wxALL, 5);

	// Add OK/Cancel buttons
	wxSizer* button_sizer = newd wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(newd wxButton(dialog, wxID_OK, "OK"), wxSizerFlags(1).Center());
	button_sizer->Add(newd wxButton(dialog, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center());
	sizer->Add(button_sizer, 0, wxALIGN_CENTER | wxALL, 5);

	dialog->SetSizer(sizer);

	// Show dialog
	if (dialog->ShowModal() == wxID_OK) {
		if (town_list->GetSelection() != wxNOT_FOUND) {
			Town* town = reinterpret_cast<Town*>(town_list->GetClientData(town_list->GetSelection()));
			if (town) {
				// Change town for each selected house
				for (size_t i = 0; i < selections.GetCount(); ++i) {
					House* house = reinterpret_cast<House*>(house_list->GetClientData(selections[i]));
					if (house) {
						house->townid = town->getID();
					}
				}

				// Refresh the house list
				RefreshHouseList();

				// Refresh the map
				g_gui.RefreshView();
			}
		}
	}

	dialog->Destroy();
}

void HousePalettePanel::RefreshHouseList() {
	// Preserve current selections
	wxArrayInt selections;
	house_list->GetSelections(selections);
	std::vector<uint32_t> selected_house_ids;

	// Store IDs of all selected houses
	for (size_t i = 0; i < selections.GetCount(); ++i) {
		House* house = reinterpret_cast<House*>(house_list->GetClientData(selections[i]));
		if (house) {
			selected_house_ids.push_back(house->getID());
		}
	}

	// Reload the house list
	Town* what_town = reinterpret_cast<Town*>(town_choice->GetClientData(town_choice->GetSelection()));

	// Clear the old houselist
	house_list->Clear();

	for (HouseMap::iterator house_iter = map->houses.begin(); house_iter != map->houses.end(); ++house_iter) {
		if (what_town) {
			if (house_iter->second->townid == what_town->getID()) {
				house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
			}
		} else {
			// "No Town" selected!
			if (map->towns.getTown(house_iter->second->townid) == nullptr) {
				// The town doesn't exist
				house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
			}
		}
	}
	house_list->Sort();

	// Try to restore previous selections
	bool foundAny = false;
	for (unsigned int i = 0; i < house_list->GetCount(); ++i) {
		House* house = reinterpret_cast<House*>(house_list->GetClientData(i));
		if (house) {
			for (uint32_t selected_id : selected_house_ids) {
				if (house->getID() == selected_id) {
					house_list->SetSelection(i);
					foundAny = true;
					break;
				}
			}
		}
	}

	// If no selections could be restored, ensure buttons are in correct state
	if (!foundAny && house_list->GetCount() > 0) {
		SelectHouse(0);
	} else if (!foundAny) {
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
		select_position_button->Enable(false);
		house_brush_button->Enable(false);
	}
}

void HousePalettePanel::OnClickHouseBrushButton(wxCommandEvent& event) {
	SelectHouseBrush();
	g_gui.SelectBrush();
}

void HousePalettePanel::OnClickSelectExitButton(wxCommandEvent& event) {
	SelectExitBrush();
	g_gui.SelectBrush();
}

void HousePalettePanel::OnClickAddHouse(wxCommandEvent& event) {
	if (map == nullptr) {
		return;
	}

	House* new_house = newd House(*map);
	new_house->setID(map->houses.getEmptyID());

	std::ostringstream os;
	os << "Unnamed House #" << new_house->getID();
	new_house->name = os.str();
	Town* town = reinterpret_cast<Town*>(town_choice->GetClientData(town_choice->GetSelection()));

	ASSERT(town);
	new_house->townid = town->getID();

	map->houses.addHouse(new_house);
	house_list->Append(wxstr(new_house->getDescription()), new_house);
	SelectHouse(house_list->FindString(wxstr(new_house->getDescription())));
	g_gui.SelectBrush();
	refresh_timer.Start(300, true);
}

void HousePalettePanel::OnClickEditHouse(wxCommandEvent& event) {
	if (house_list->GetCount() == 0) {
		return;
	}
	if (map == nullptr) {
		return;
	}

	// Only edit if a single house is selected
	wxArrayInt selections;
	if (house_list->GetSelections(selections) != 1) {
		wxMessageBox("Please select only one house to edit.", "Edit House", wxOK | wxICON_INFORMATION);
		return;
	}

	int selection = selections[0];
	House* house = reinterpret_cast<House*>(house_list->GetClientData(selection));
	if (house) {
		wxDialog* d = newd EditHouseDialog(g_gui.root, map, house);
		int ret = d->ShowModal();
		if (ret == 1) {
			// Something changed, change name of house
			house_list->SetString(selection, wxstr(house->getDescription()));
			house_list->Sort();

			// refresh house list for town
			SelectTown(town_choice->GetSelection());
			g_gui.SelectBrush();
			refresh_timer.Start(300, true);
		}
	}
}

void HousePalettePanel::OnClickRemoveHouse(wxCommandEvent& event) {
	wxArrayInt selections;
	int count = house_list->GetSelections(selections);

	if (count == 0) {
		return;
	}

	// Ask for confirmation when removing multiple houses
	if (count > 1) {
		wxString message = wxString::Format("Are you sure you want to remove %d houses?", count);
		int response = wxMessageBox(message, "Confirm Removal", wxYES_NO | wxICON_QUESTION);
		if (response != wxYES) {
			return;
		}
	}

	// Sort selections in descending order to avoid index issues when deleting
	std::sort(selections.begin(), selections.end(), std::greater<int>());

	// Remove all selected houses
	for (size_t i = 0; i < selections.GetCount(); ++i) {
		int selection = selections[i];
		House* house = reinterpret_cast<House*>(house_list->GetClientData(selection));
		map->houses.removeHouse(house);
		house_list->Delete(selection);
	}

	refresh_timer.Start(300, true);

	// Select an appropriate remaining item if possible
	if (house_list->GetCount() > 0) {
		int new_selection = std::min(selections.back(), (int)house_list->GetCount() - 1);
		house_list->SetSelection(new_selection);
		edit_house_button->Enable(true);
		remove_house_button->Enable(true);
		select_position_button->Enable(true);
		house_brush_button->Enable(true);
	} else {
		select_position_button->Enable(false);
		select_position_button->SetValue(false);
		house_brush_button->Enable(false);
		house_brush_button->SetValue(false);
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
	}

	g_gui.SelectBrush();
	g_gui.RefreshView();
}

#ifdef __APPLE__
// On wxMac it is possible to deselect a wxListBox. (Unlike on the other platforms)
// EVT_LISTBOX is not triggered when the deselection is happening. http://trac.wxwidgets.org/ticket/15603
// Here we find out if the listbox was deselected using a normal mouse up event so we know when to disable the buttons and brushes.
void HousePalettePanel::OnListBoxClick(wxMouseEvent& event) {
	if (house_list->GetSelection() == wxNOT_FOUND) {
		select_position_button->Enable(false);
		select_position_button->SetValue(false);
		house_brush_button->Enable(false);
		house_brush_button->SetValue(false);
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
		g_gui.SelectBrush();
	}
}
#endif

// ============================================================================
// House Edit Dialog

BEGIN_EVENT_TABLE(EditHouseDialog, wxDialog)
EVT_SET_FOCUS(EditHouseDialog::OnFocusChange)
EVT_BUTTON(wxID_OK, EditHouseDialog::OnClickOK)
EVT_BUTTON(wxID_CANCEL, EditHouseDialog::OnClickCancel)
END_EVENT_TABLE()

EditHouseDialog::EditHouseDialog(wxWindow* parent, Map* map, House* house) :
	// window title
	wxDialog(parent, wxID_ANY, "House Properties", wxDefaultPosition, wxSize(250, 160)),
	map(map),
	what_house(house) {
	ASSERT(map);
	ASSERT(house);

	// main properties window box
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "House Properties");
	wxFlexGridSizer* housePropContainer = newd wxFlexGridSizer(2, 10, 10);
	housePropContainer->AddGrowableCol(1);

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	house_name = wxstr(house->name);
	house_id = i2ws(house->getID());
	house_rent = i2ws(house->rent);

	// House name
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Name:"), wxSizerFlags(0).Border(wxLEFT, 5));
	name_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(160, 20), 0, wxTextValidator(wxFILTER_ASCII, &house_name));
	subsizer->Add(name_field, wxSizerFlags(1).Expand());

	// Town selection menu
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Town:"), wxSizerFlags(0).Border(wxLEFT, 5));

	const Towns& towns = map->towns;

	town_id_field = newd wxChoice(this, wxID_ANY);
	int to_select_index = 0;
	uint32_t houseTownId = house->townid;

	if (towns.count() > 0) {
		bool found = false;
		for (TownMap::const_iterator town_iter = towns.begin(); town_iter != towns.end(); ++town_iter) {
			if (town_iter->second->getID() == houseTownId) {
				found = true;
			}
			town_id_field->Append(wxstr(town_iter->second->getName()), newd int(town_iter->second->getID()));
			if (!found) {
				++to_select_index;
			}
		}

		if (!found) {
			if (houseTownId != 0) {
				town_id_field->Append("Undefined Town (id:" + i2ws(houseTownId) + ")", newd int(houseTownId));
				++to_select_index;
			}
		}
	}
	town_id_field->SetSelection(to_select_index);
	subsizer->Add(town_id_field, wxSizerFlags(1).Expand());
	// end town selection

	// Rent price
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Rent:"), wxSizerFlags(0).Border(wxLEFT, 5));
	rent_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(160, 20), 0, wxTextValidator(wxFILTER_NUMERIC, &house_rent));
	subsizer->Add(rent_field, wxSizerFlags(1).Expand());

	// Right column
	wxFlexGridSizer* subsizerRight = newd wxFlexGridSizer(1, 10, 10);

	// house ID
	wxFlexGridSizer* houseSizer = newd wxFlexGridSizer(2, 10, 10);

	houseSizer->Add(newd wxStaticText(this, wxID_ANY, "ID:"), wxSizerFlags(0).Center());
	id_field = newd wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(40, 20), wxSP_ARROW_KEYS, 1, 0xFFFF, house->getID());
	// id_field->Enable(false);
	houseSizer->Add(id_field, wxSizerFlags(1).Expand());
	subsizerRight->Add(houseSizer, wxSizerFlags(1).Expand());

	// Guildhall checkbox
	wxSizer* checkbox_sub_sizer = newd wxBoxSizer(wxVERTICAL);
	checkbox_sub_sizer->AddSpacer(4);

	guildhall_field = newd wxCheckBox(this, wxID_ANY, "Guildhall");

	checkbox_sub_sizer->Add(guildhall_field);
	subsizerRight->Add(checkbox_sub_sizer);
	guildhall_field->SetValue(house->guildhall);

	// construct the layout
	housePropContainer->Add(subsizer, wxSizerFlags(5).Expand());
	housePropContainer->Add(subsizerRight, wxSizerFlags(5).Expand());
	boxsizer->Add(housePropContainer, wxSizerFlags(5).Expand().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxRIGHT | wxLEFT, 20));

	// OK/Cancel buttons
	wxSizer* buttonsSizer = newd wxBoxSizer(wxHORIZONTAL);
	buttonsSizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	buttonsSizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(buttonsSizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
}

EditHouseDialog::~EditHouseDialog() {
	////
}

void EditHouseDialog::OnFocusChange(wxFocusEvent& event) {
	wxWindow* win = event.GetWindow();
	if (wxSpinCtrl* spin = dynamic_cast<wxSpinCtrl*>(win)) {
		spin->SetSelection(-1, -1);
	} else if (wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(win)) {
		text->SetSelection(-1, -1);
	}
}

void EditHouseDialog::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	if (Validate() && TransferDataFromWindow()) {
		// Verify the new rent information
		long new_house_rent;
		house_rent.ToLong(&new_house_rent);
		if (new_house_rent < 0) {
			g_gui.PopupDialog(this, "Error", "House rent cannot be less than 0.", wxOK);
			return;
		}

		// Verify the new house id
		uint32_t new_house_id = id_field->GetValue();
		if (new_house_id < 1) {
			g_gui.PopupDialog(this, "Error", "House id cannot be less than 1.", wxOK);
			return;
		}

		// Verify the new house name
		if (house_name.length() == 0) {
			g_gui.PopupDialog(this, "Error", "House name cannot be empty.", wxOK);
			return;
		}

		// Verify town selection
		if (town_id_field->GetSelection() == wxNOT_FOUND) {
			g_gui.PopupDialog(this, "Error", "You must select a town for this house.", wxOK);
			return;
		}

		int* new_town_id = reinterpret_cast<int*>(town_id_field->GetClientData(town_id_field->GetSelection()));
		if (!new_town_id) {
			g_gui.PopupDialog(this, "Error", "Invalid town selection.", wxOK);
			return;
		}

		if (g_settings.getInteger(Config::WARN_FOR_DUPLICATE_ID)) {
			Houses& houses = map->houses;
			for (HouseMap::const_iterator house_iter = houses.begin(); house_iter != houses.end(); ++house_iter) {
				House* house = house_iter->second;
				ASSERT(house);

				if (house->getID() == new_house_id && new_house_id != what_house->getID()) {
					g_gui.PopupDialog(this, "Error", "This house id is already in use.", wxOK);
					return;
				}

				if (wxstr(house->name) == house_name && house->getID() != what_house->getID()) {
					int ret = g_gui.PopupDialog(this, "Warning", "This house name is already in use, are you sure you want to continue?", wxYES | wxNO);
					if (ret == wxID_NO) {
						return;
					}
				}
			}
		}

		if (new_house_id != what_house->getID()) {
			int ret = g_gui.PopupDialog(this, "Warning", "Changing existing house ids on a production server WILL HAVE DATABASE CONSEQUENCES such as potential item loss, house owner change or invalidating guest lists.\\nYou are doing it at own risk!\\n\\nAre you ABSOLUTELY sure you want to continue?", wxYES | wxNO);
			if (ret == wxID_NO) {
				return;
			}

			uint32_t old_house_id = what_house->getID();

			map->convertHouseTiles(old_house_id, new_house_id);
			map->houses.changeId(what_house, new_house_id);
		}

		// Transfer to house
		what_house->name = nstr(house_name);
		what_house->rent = new_house_rent;
		what_house->guildhall = guildhall_field->GetValue();
		what_house->townid = *new_town_id;

		EndModal(1);
	}
}

void EditHouseDialog::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	// Just close this window
	EndModal(0);
}
""",
    "wxwidgets/palette_house.h": """//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_PALETTE_HOUSE_H_
#define RME_PALETTE_HOUSE_H_

#include "palette_common.h"

class House;

class HousePalettePanel : public PalettePanel {
public:
	HousePalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	~HousePalettePanel();

	PaletteType GetType() const;

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Returns the currently selected brush size
	int GetSelectedBrushSize() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush);

	// Called sometimes?
	void OnUpdate();
	// Called when this page is about to be displayed
	void OnSwitchIn();

	void OnLayoutFixTimer(wxTimerEvent& event);

	void SetMap(Map* map);

protected:
	// Internal use
	void SaveHouse();
	void SelectTown(size_t index);
	void SelectHouse(size_t index);

	House* GetCurrentlySelectedHouse() const;

	void SelectHouseBrush();
	void SelectExitBrush();
	void RefreshHouseList();

public:
	// wxWidgets event handling
	void OnTownChange(wxCommandEvent& event);
	void OnListBoxChange(wxCommandEvent& event);
	void OnListBoxDoubleClick(wxCommandEvent& event);
	void OnClickHouseBrushButton(wxCommandEvent& event);
	void OnClickSelectExitButton(wxCommandEvent& event);
	void OnClickAddHouse(wxCommandEvent& event);
	void OnClickEditHouse(wxCommandEvent& event);
	void OnClickRemoveHouse(wxCommandEvent& event);
	void OnListBoxContextMenu(wxContextMenuEvent& event);
	void OnMoveHouseToTown(wxCommandEvent& event);

#ifdef __APPLE__
	// Used for detecting a deselect
	void OnListBoxClick(wxMouseEvent& event);
#endif

protected:
	Map* map;
	wxChoice* town_choice;
	SortableListBox* house_list;
	wxToggleButton* house_brush_button;
	wxToggleButton* select_position_button;
	wxButton* add_house_button;
	wxButton* edit_house_button;
	wxButton* remove_house_button;
	wxMenu* context_menu;

	// Used for ugly hack
	bool do_resize_on_display;
	wxTimer fix_size_timer;

	DECLARE_EVENT_TABLE()
};

class EditHouseDialog : public wxDialog {
public:
	EditHouseDialog(wxWindow* parent, Map* map, House* house);
	virtual ~EditHouseDialog();

	void OnFocusChange(wxFocusEvent&);

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	Map* map;
	House* what_house;

	wxString house_name, house_id, house_rent;

	wxTextCtrl* name_field;
	wxChoice* town_id_field;
	wxSpinCtrl* id_field;
	wxTextCtrl* rent_field;
	wxCheckBox* guildhall_field;

	DECLARE_EVENT_TABLE();
};

#endif
""",
    "wxwidgets/palette_waypoints.cpp": """//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

// ============================================================================
// Waypoint palette

#include "main.h"

#include "gui.h"
#include "palette_waypoints.h"
#include "waypoint_brush.h"
#include "map.h"

BEGIN_EVENT_TABLE(WaypointPalettePanel, PalettePanel)
EVT_BUTTON(PALETTE_WAYPOINT_ADD_WAYPOINT, WaypointPalettePanel::OnClickAddWaypoint)
EVT_BUTTON(PALETTE_WAYPOINT_REMOVE_WAYPOINT, WaypointPalettePanel::OnClickRemoveWaypoint)

EVT_LIST_BEGIN_LABEL_EDIT(PALETTE_WAYPOINT_LISTBOX, WaypointPalettePanel::OnBeginEditWaypointLabel)
EVT_LIST_END_LABEL_EDIT(PALETTE_WAYPOINT_LISTBOX, WaypointPalettePanel::OnEditWaypointLabel)
EVT_LIST_ITEM_SELECTED(PALETTE_WAYPOINT_LISTBOX, WaypointPalettePanel::OnClickWaypoint)
END_EVENT_TABLE()

WaypointPalettePanel::WaypointPalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	map(nullptr) {
	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Waypoints");

	waypoint_list = newd wxListCtrl(this, PALETTE_WAYPOINT_LISTBOX, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_EDIT_LABELS | wxLC_NO_HEADER);
	waypoint_list->InsertColumn(0, "UNNAMED", wxLIST_FORMAT_LEFT, 200);
	sidesizer->Add(waypoint_list, 1, wxEXPAND);

	wxSizer* tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(add_waypoint_button = newd wxButton(this, PALETTE_WAYPOINT_ADD_WAYPOINT, "Add", wxDefaultPosition, wxSize(50, -1)), 1, wxEXPAND);
	tmpsizer->Add(remove_waypoint_button = newd wxButton(this, PALETTE_WAYPOINT_REMOVE_WAYPOINT, "Remove", wxDefaultPosition, wxSize(70, -1)), 1, wxEXPAND);
	sidesizer->Add(tmpsizer, 0, wxEXPAND);

	SetSizerAndFit(sidesizer);
}

WaypointPalettePanel::~WaypointPalettePanel() {
	////
}

void WaypointPalettePanel::OnSwitchIn() {
	PalettePanel::OnSwitchIn();
}

void WaypointPalettePanel::OnSwitchOut() {
	PalettePanel::OnSwitchOut();
}

void WaypointPalettePanel::SetMap(Map* m) {
	map = m;
	this->Enable(m);
}

void WaypointPalettePanel::SelectFirstBrush() {
	// SelectWaypointBrush();
}

Brush* WaypointPalettePanel::GetSelectedBrush() const {
	long item = waypoint_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	g_gui.waypoint_brush->setWaypoint(
		item == -1 ? nullptr : map->waypoints.getWaypoint(nstr(waypoint_list->GetItemText(item)))
	);
	return g_gui.waypoint_brush;
}

bool WaypointPalettePanel::SelectBrush(const Brush* whatbrush) {
	ASSERT(whatbrush == g_gui.waypoint_brush);
	return false;
}

int WaypointPalettePanel::GetSelectedBrushSize() const {
	return 0;
}

PaletteType WaypointPalettePanel::GetType() const {
	return TILESET_WAYPOINT;
}

wxString WaypointPalettePanel::GetName() const {
	return "Waypoint Palette";
}

void WaypointPalettePanel::OnUpdate() {
	if (wxTextCtrl* tc = waypoint_list->GetEditControl()) {
		Waypoint* wp = map->waypoints.getWaypoint(nstr(tc->GetValue()));
		if (wp && wp->pos == Position()) {
			if (map->getTile(wp->pos)) {
				map->getTileL(wp->pos)->decreaseWaypointCount();
			}
			map->waypoints.removeWaypoint(wp->name);
		}
	}
	waypoint_list->DeleteAllItems();

	if (!map) {
		waypoint_list->Enable(false);
		add_waypoint_button->Enable(false);
		remove_waypoint_button->Enable(false);
	} else {
		waypoint_list->Enable(true);
		add_waypoint_button->Enable(true);
		remove_waypoint_button->Enable(true);

		Waypoints& waypoints = map->waypoints;

		for (WaypointMap::const_iterator iter = waypoints.begin(); iter != waypoints.end(); ++iter) {
			waypoint_list->InsertItem(0, wxstr(iter->second->name));
		}
	}
}

void WaypointPalettePanel::OnClickWaypoint(wxListEvent& event) {
	if (!map) {
		return;
	}

	std::string wpname = nstr(event.GetText());
	Waypoint* wp = map->waypoints.getWaypoint(wpname);
	if (wp) {
		g_gui.SetScreenCenterPosition(wp->pos);
		g_gui.waypoint_brush->setWaypoint(wp);
	}
}

void WaypointPalettePanel::OnBeginEditWaypointLabel(wxListEvent& event) {
	// We need to disable all hotkeys, so we can type properly
	g_gui.DisableHotkeys();
}

void WaypointPalettePanel::OnEditWaypointLabel(wxListEvent& event) {
	std::string wpname = nstr(event.GetLabel());
	std::string oldwpname = nstr(waypoint_list->GetItemText(event.GetIndex()));
	Waypoint* wp = map->waypoints.getWaypoint(oldwpname);

	if (event.IsEditCancelled()) {
		return;
	}

	if (wpname == "") {
		map->waypoints.removeWaypoint(oldwpname);
		g_gui.RefreshPalettes();
	} else if (wp) {
		if (wpname == oldwpname) {
			; // do nothing
		} else {
			if (map->waypoints.getWaypoint(wpname)) {
				// Already exists a waypoint with this name!
				g_gui.SetStatusText("There already is a waypoint with this name.");
				event.Veto();
				if (oldwpname == "") {
					map->waypoints.removeWaypoint(oldwpname);
					g_gui.RefreshPalettes();
				}
			} else {
				Waypoint* nwp = newd Waypoint(*wp);
				nwp->name = wpname;

				Waypoint* rwp = map->waypoints.getWaypoint(oldwpname);
				if (rwp) {
					if (map->getTile(rwp->pos)) {
						map->getTileL(rwp->pos)->decreaseWaypointCount();
					}
					map->waypoints.removeWaypoint(rwp->name);
				}

				map->waypoints.addWaypoint(nwp);
				g_gui.waypoint_brush->setWaypoint(nwp);

				// Refresh other palettes
				refresh_timer.Start(300, true);
			}
		}
	}

	if (event.IsAllowed()) {
		g_gui.EnableHotkeys();
	}
}

void WaypointPalettePanel::OnClickAddWaypoint(wxCommandEvent& event) {
	if (map) {
		map->waypoints.addWaypoint(newd Waypoint());
		long i = waypoint_list->InsertItem(0, "");
		waypoint_list->EditLabel(i);

		// g_gui.RefreshPalettes();
	}
}

void WaypointPalettePanel::OnClickRemoveWaypoint(wxCommandEvent& event) {
	if (!map) {
		return;
	}

	long item = waypoint_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != -1) {
		Waypoint* wp = map->waypoints.getWaypoint(nstr(waypoint_list->GetItemText(item)));
		if (wp) {
			if (map->getTile(wp->pos)) {
				map->getTileL(wp->pos)->decreaseWaypointCount();
			}
			map->waypoints.removeWaypoint(wp->name);
		}
		waypoint_list->DeleteItem(item);
		refresh_timer.Start(300, true);
	}
}
""",
    "wxwidgets/palette_waypoints.h": """//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_PALETTE_WAYPOINTS_H_
#define RME_PALETTE_WAYPOINTS_H_

#include <wx/listctrl.h>

#include "waypoints.h"
#include "palette_common.h"

class WaypointPalettePanel : public PalettePanel {
public:
	WaypointPalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	~WaypointPalettePanel();

	wxString GetName() const;
	PaletteType GetType() const;

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Returns the currently selected brush size
	int GetSelectedBrushSize() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush);

	// Called sometimes?
	void OnUpdate();
	// Called when this page is about to be displayed
	void OnSwitchIn();
	// Called when this page is hidden
	void OnSwitchOut();

public:
	// wxWidgets event handling
	void OnClickWaypoint(wxListEvent& event);
	void OnBeginEditWaypointLabel(wxListEvent& event);
	void OnEditWaypointLabel(wxListEvent& event);
	void OnClickAddWaypoint(wxCommandEvent& event);
	void OnClickRemoveWaypoint(wxCommandEvent& event);

	void SetMap(Map* map);

protected:
	Map* map;
	wxListCtrl* waypoint_list;
	wxButton* add_waypoint_button;
	wxButton* remove_waypoint_button;

	DECLARE_EVENT_TABLE()
};

#endif
"""
}

analyzed_files_data = []
file_descriptions = {
    "wxwidgets/palette_house.cpp": "Implements `HousePalettePanel` using wxChoice for towns, SortableListBox for houses, buttons for Add/Edit/Remove, and toggles for House/Exit brushes. Defines `EditHouseDialog` for house properties.",
    "wxwidgets/palette_house.h": "Header for `HousePalettePanel` and `EditHouseDialog`.",
    "wxwidgets/palette_waypoints.cpp": "Implements `WaypointPalettePanel` using wxListCtrl for displaying and editing waypoint names, with Add/Remove buttons.",
    "wxwidgets/palette_waypoints.h": "Header for `WaypointPalettePanel`."
}

for file_path, content in cpp_files_content.items():
    analyzed_files_data.append({
        "file_path": file_path,
        "description": file_descriptions.get(file_path, "N/A"),
        "md5_hash": get_md5_hash(content),
        "content_lite": get_content_lite(content)
    })

yaml_output = {
    "wbs_item_id": "UI-07",
    "name": "Port House & Waypoint Tools",
    "description": "Recreate UI components for managing houses (palette, editor dialog) and waypoints (palette).",
    "dependencies": [
        "UI-02   # House and Waypoint palettes are tabs in the main palette system.",
        "CORE-03 # For map saving/loading which includes house and waypoint data.",
        "LOGIC-04 # For Waypoint and WaypointManager data structures.",
        "LOGIC-05 # For House and Town data structures and management."
    ],
    "input_files": [
        "wxwidgets/palette_house.cpp",
        "wxwidgets/palette_house.h",
        "wxwidgets/palette_waypoints.cpp",
        "wxwidgets/palette_waypoints.h"
    ],
    "analyzed_input_files": analyzed_files_data,
    "documentation_references": [
        "QComboBox: https://doc.qt.io/qt-6/qcombobox.html",
        "QListWidget: https://doc.qt.io/qt-6/qlistwidget.html",
        "QDialog: https://doc.qt.io/qt-6/qdialog.html",
        "QRadioButton / QActionGroup for brush mode selection."
    ],
    "current_functionality_summary": """The `HousePalettePanel` provides a UI to:
- Select a town using a `wxChoice` control.
- List houses within that town in a `SortableListBox`.
- Add, Edit (via `EditHouseDialog`), and Remove houses.
- Implement a context menu for moving houses to different towns.
- Toggle between a "House Tile Brush" and a "Set House Exit" brush mode, configuring a global house brush with the selected house.
The `EditHouseDialog` allows editing a house's name, town, rent, ID, and guildhall status.

The `WaypointPalettePanel` provides a UI to:
- List all waypoints in a `wxListCtrl` (names are editable).
- Add new waypoints (name becomes editable).
- Remove selected waypoints.
- Selecting a waypoint sets a global waypoint brush.""",
    "definition_of_done": [
        "The 'House' tab in the main palette system (`UI-02`) is fully implemented:",
        "  - A `QComboBox` lists towns from the current map (plus a '(No Town)' option). Selecting a town filters the `QListWidget` of houses.",
        "  - The `QListWidget` displays houses for the selected town, showing house name, ID, and size. It supports multi-selection for removal or moving.",
        "  - An 'Add House' `QPushButton` opens the `EditHouseDialogQt` for defining a new house.",
        "  - An 'Edit House' `QPushButton` opens the `EditHouseDialogQt` for the single selected house.",
        "  - A 'Remove House' `QPushButton` removes all selected house(s) after confirmation.",
        "  - A context menu on the house list offers 'Move to Town...', which opens a dialog to select a new town for the selected house(s).",
        "  - `QRadioButton`s or a `QActionGroup` allows switching between 'Draw House Tiles' and 'Set House Exit' brush modes, configuring the global house brush accordingly.",
        "The `EditHouseDialogQt` (subclass of `QDialog`) is implemented:",
        "  - Allows editing/setting house name (`QLineEdit`), town (`QComboBox`), rent (`QSpinBox`), ID (`QSpinBox` with warnings for changes/duplicates), and guildhall status (`QCheckBox`).",
        "  - Loads data from a `House` object copy and applies validated changes back to the copy on 'OK'.",
        "The 'Waypoint' tab in the main palette system (`UI-02`) is fully implemented:",
        "  - A `QListWidget` displays all waypoints from the map. Waypoint names are editable in-place (`Qt::ItemIsEditable`).",
        "  - An 'Add Waypoint' `QPushButton` creates a new waypoint with a default unique name in the map data and list, then initiates editing of the new item's name.",
        "  - A 'Remove Waypoint' `QPushButton` removes the selected waypoint(s) from the map data and list after confirmation.",
        "Selecting a house (and brush mode) or a waypoint in their respective palettes correctly configures and activates the corresponding global brush (`HouseBrush`, `HouseExitBrush`, `WaypointBrush`) for use on the map canvas.",
        "All palette lists and controls are updated when the underlying map data (houses, towns, waypoints) changes."
    ],
    "boilerplate_coder_ai_prompt": """Your task is to implement the Qt6 UI components for managing Houses and Waypoints, specifically their palette tabs and the House editing dialog.

**Reference Files:** `wxwidgets/palette_house.cpp`, `wxwidgets/palette_house.h`, `wxwidgets/palette_waypoints.cpp`, `wxwidgets/palette_waypoints.h`.

**1. House Palette Tab (within main `paletteTabWidget` from `UI-02`):**
   - **Controls:**
     - `QComboBox* townCombo;`
     - `QListWidget* houseList;` (Set `SelectionMode` to `ExtendedSelection` for multi-delete/move)
     - `QPushButton* addHouseButton;`
     - `QPushButton* editHouseButton;`
     - `QPushButton* removeHouseButton;`
     - `QRadioButton* drawHouseTilesRadio;` (Text: "Draw House Tiles")
     - `QRadioButton* setHouseExitRadio;` (Text: "Set House Exit")
     - (Group the radio buttons)
   - **Functionality:**
     - Populate `townCombo` from `g_map.towns`. Add a "(No Town)" item.
     - When `townCombo` changes, filter `houseList` to show houses matching `selectedTownId`. Display "Name (ID: X, Size: Y sqm)". Store `House*` or ID with `QListWidgetItem::setData`.
     - `addHouseButton`: Create a new `House` object, open `EditHouseDialogQt` (pass it the new house object and map). If dialog accepted, add house to `g_map.houses`, refresh `houseList`.
     - `editHouseButton`: For selected house in `houseList`, create a copy, open `EditHouseDialogQt`. If accepted, apply changes to original house (create Undo action), refresh `houseList`. Disable if no selection or multiple selections.
     - `removeHouseButton`: Confirm and remove selected house(s) from `g_map.houses`. Refresh `houseList`. Create Undo action.
     - `houseList` Context Menu: Action "Move to Town...". Opens a `QInputDialog::getItem` to select a new town from `g_map.towns`. Update `townid` for all selected houses. Refresh `houseList`. Create Undo action.
     - Radio buttons/selection in `houseList` update the global house brush:
       - If `drawHouseTilesRadio` checked: `g_houseBrush->setHouse(selectedHouse); g_gui->setActiveBrush(g_houseBrush);`
       - If `setHouseExitRadio` checked: `g_houseExitBrush->setHouse(selectedHouse); g_gui->setActiveBrush(g_houseExitBrush);`

**2. `EditHouseDialogQt` (inherits `QDialog`):**
   - Constructor: `EditHouseDialogQt(QWidget* parent, Map* map, House* houseToEditCopy)`.
   - Controls:
     - `QLineEdit* nameEdit;`
     - `QComboBox* townCombo;` (Populate with `map->towns`, set current based on `houseToEditCopy->townid`).
     - `QSpinBox* rentSpinBox;`
     - `QSpinBox* idSpinBox;` (Range 1-65535).
     - `QCheckBox* guildhallCheck;`
   - Load initial values from `houseToEditCopy`.
   - On "OK" (`accept()`): Validate data (name not empty, ID unique if changed, rent >= 0). Update `houseToEditCopy` fields.

**3. Waypoint Palette Tab (within main `paletteTabWidget` from `UI-02`):**
   - **Controls:**
     - `QListWidget* waypointList;` (Set `Qt::ItemIsEditable` for in-place renaming).
     - `QPushButton* addWaypointButton;`
     - `QPushButton* removeWaypointButton;`
   - **Functionality:**
     - Populate `waypointList` from `g_map.waypoints`. Store `Waypoint*` or name with `QListWidgetItem::setData`.
     - `addWaypointButton`: Create new `Waypoint` (e.g., "New Waypoint 1") in `g_map.waypoints`. Add to `waypointList`. Call `waypointList->editItem()` on the new item.
     - `removeWaypointButton`: Confirm and remove selected waypoint(s) from `g_map.waypoints` and `waypointList`. Create Undo action.
     - `waypointList::itemChanged(QListWidgetItem* item)` signal:
       - Get old name (stored in item data before edit) and new name (item->text()).
       - Validate new name (not empty, unique among waypoints). If invalid, revert or show error.
       - Update the `Waypoint` object in `g_map.waypoints`. Create Undo action.
     - `waypointList::currentItemChanged`: Sets `g_waypointBrush->setWaypoint(selectedWaypoint); g_gui->setActiveBrush(g_waypointBrush);`.

**General:**
- Ensure all list/combo boxes are updated if the underlying map data (towns, houses, waypoints) changes due to other operations or file loading.
- Connect signals from UI elements to appropriate slots to implement the described functionality.
"""
}

output_filename = "enhanced_wbs_yaml_files/UI-07.yaml"
output_dir = os.path.dirname(output_filename)

if not os.path.exists(output_dir):
    os.makedirs(output_dir)

with open(output_filename, 'w') as f:
    yaml.dump(yaml_output, f, sort_keys=False, width=1000, allow_unicode=True)

print(f"Successfully generated {output_filename}")
