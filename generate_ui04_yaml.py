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

# Content of the C++ files (obtained from a previous step)
cpp_files_content = {
    "wxwidgets/properties_window.cpp": """//////////////////////////////////////////////////////////////////////
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

#include "properties_window.h"

#include "gui_ids.h"
#include "complexitem.h"
#include "container_properties_window.h"

#include <wx/grid.h>

BEGIN_EVENT_TABLE(PropertiesWindow, wxDialog)
EVT_BUTTON(wxID_OK, PropertiesWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, PropertiesWindow::OnClickCancel)

EVT_BUTTON(ITEM_PROPERTIES_ADD_ATTRIBUTE, PropertiesWindow::OnClickAddAttribute)
EVT_BUTTON(ITEM_PROPERTIES_REMOVE_ATTRIBUTE, PropertiesWindow::OnClickRemoveAttribute)

EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, PropertiesWindow::OnNotebookPageChanged)

EVT_GRID_CELL_CHANGED(PropertiesWindow::OnGridValueChanged)
END_EVENT_TABLE()

PropertiesWindow::PropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(parent, "Item Properties", map, tile_parent, item, pos),
	currentPanel(nullptr) {
	ASSERT(edit_item);
	notebook = newd wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(600, 300));

	notebook->AddPage(createGeneralPanel(notebook), "Simple", true);
	if (dynamic_cast<Container*>(item)) {
		notebook->AddPage(createContainerPanel(notebook), "Contents");
	}
	notebook->AddPage(createAttributesPanel(notebook), "Advanced");

	wxSizer* topSizer = newd wxBoxSizer(wxVERTICAL);
	topSizer->Add(notebook, wxSizerFlags(1).DoubleBorder());

	wxSizer* optSizer = newd wxBoxSizer(wxHORIZONTAL);
	optSizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(0).Center());
	optSizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(0).Center());
	topSizer->Add(optSizer, wxSizerFlags(0).Center().DoubleBorder());

	SetSizerAndFit(topSizer);
	Centre(wxBOTH);
}

PropertiesWindow::~PropertiesWindow() {
	;
}

void PropertiesWindow::Update() {
	Container* container = dynamic_cast<Container*>(edit_item);
	if (container) {
		for (uint32_t i = 0; i < container->getVolume(); ++i) {
			container_items[i]->setItem(container->getItem(i));
		}
	}
	wxDialog::Update();
}

wxWindow* PropertiesWindow::createGeneralPanel(wxWindow* parent) {
	wxPanel* panel = newd wxPanel(parent, ITEM_PROPERTIES_GENERAL_TAB);
	wxFlexGridSizer* gridsizer = newd wxFlexGridSizer(2, 10, 10);
	gridsizer->AddGrowableCol(1);

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "ID " + i2ws(edit_item->getID())));
	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "\"" + wxstr(edit_item->getName()) + "\""));

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "Action ID"));
	wxSpinCtrl* action_id_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
	gridsizer->Add(action_id_field, wxSizerFlags(1).Expand());

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "Unique ID"));
	wxSpinCtrl* unique_id_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
	gridsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

	panel->SetSizerAndFit(gridsizer);

	return panel;
}

wxWindow* PropertiesWindow::createContainerPanel(wxWindow* parent) {
	Container* container = (Container*)edit_item;
	wxPanel* panel = newd wxPanel(parent, ITEM_PROPERTIES_CONTAINER_TAB);
	wxSizer* topSizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* gridSizer = newd wxGridSizer(6, 5, 5);

	bool use_large_sprites = g_settings.getBoolean(Config::USE_LARGE_CONTAINER_ICONS);
	for (uint32_t i = 0; i < container->getVolume(); ++i) {
		Item* item = container->getItem(i);
		ContainerItemButton* containerItemButton = newd ContainerItemButton(panel, use_large_sprites, i, edit_map, item);

		container_items.push_back(containerItemButton);
		gridSizer->Add(containerItemButton, wxSizerFlags(0));
	}

	topSizer->Add(gridSizer, wxSizerFlags(1).Expand());

	/*
	wxSizer* optSizer = newd wxBoxSizer(wxHORIZONTAL);
	optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_ADD_ATTRIBUTE, "Add Item"), wxSizerFlags(0).Center());
	// optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_REMOVE_ATTRIBUTE, "Remove Attribute"), wxSizerFlags(0).Center());
	topSizer->Add(optSizer, wxSizerFlags(0).Center().DoubleBorder());
	*/

	panel->SetSizer(topSizer);
	return panel;
}

wxWindow* PropertiesWindow::createAttributesPanel(wxWindow* parent) {
	wxPanel* panel = newd wxPanel(parent, wxID_ANY);
	wxSizer* topSizer = newd wxBoxSizer(wxVERTICAL);

	attributesGrid = newd wxGrid(panel, ITEM_PROPERTIES_ADVANCED_TAB, wxDefaultPosition, wxSize(-1, 160));
	topSizer->Add(attributesGrid, wxSizerFlags(1).Expand());

	wxFont time_font(*wxSWISS_FONT);
	attributesGrid->SetDefaultCellFont(time_font);
	attributesGrid->CreateGrid(0, 3);
	attributesGrid->DisableDragRowSize();
	attributesGrid->DisableDragColSize();
	attributesGrid->SetSelectionMode(wxGrid::wxGridSelectRows);
	attributesGrid->SetRowLabelSize(0);
	// log->SetColLabelSize(0);
	// log->EnableGridLines(false);
	attributesGrid->EnableEditing(true);

	attributesGrid->SetColLabelValue(0, "Key");
	attributesGrid->SetColSize(0, 100);
	attributesGrid->SetColLabelValue(1, "Type");
	attributesGrid->SetColSize(1, 80);
	attributesGrid->SetColLabelValue(2, "Value");
	attributesGrid->SetColSize(2, 410);

	// contents
	ItemAttributeMap attrs = edit_item->getAttributes();
	attributesGrid->AppendRows(attrs.size());
	int i = 0;
	for (ItemAttributeMap::iterator aiter = attrs.begin(); aiter != attrs.end(); ++aiter, ++i) {
		SetGridValue(attributesGrid, i, aiter->first, aiter->second);
	}

	wxSizer* optSizer = newd wxBoxSizer(wxHORIZONTAL);
	optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_ADD_ATTRIBUTE, "Add Attribute"), wxSizerFlags(0).Center());
	optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_REMOVE_ATTRIBUTE, "Remove Attribute"), wxSizerFlags(0).Center());
	topSizer->Add(optSizer, wxSizerFlags(0).Center().DoubleBorder());

	panel->SetSizer(topSizer);

	return panel;
}

void PropertiesWindow::SetGridValue(wxGrid* grid, int rowIndex, std::string label, const ItemAttribute& attr) {
	wxArrayString types;
	types.Add("Number");
	types.Add("Float");
	types.Add("Boolean");
	types.Add("String");

	grid->SetCellValue(rowIndex, 0, label);
	switch (attr.type) {
		case ItemAttribute::STRING: {
			grid->SetCellValue(rowIndex, 1, "String");
			grid->SetCellValue(rowIndex, 2, wxstr(*attr.getString()));
			break;
		}
		case ItemAttribute::INTEGER: {
			grid->SetCellValue(rowIndex, 1, "Number");
			grid->SetCellValue(rowIndex, 2, i2ws(*attr.getInteger()));
			grid->SetCellEditor(rowIndex, 2, new wxGridCellNumberEditor);
			break;
		}
		case ItemAttribute::DOUBLE:
		case ItemAttribute::FLOAT: {
			grid->SetCellValue(rowIndex, 1, "Float");
			wxString f;
			f << *attr.getFloat();
			grid->SetCellValue(rowIndex, 2, f);
			grid->SetCellEditor(rowIndex, 2, new wxGridCellFloatEditor);
			break;
		}
		case ItemAttribute::BOOLEAN: {
			grid->SetCellValue(rowIndex, 1, "Boolean");
			grid->SetCellValue(rowIndex, 2, *attr.getBoolean() ? "1" : "");
			grid->SetCellRenderer(rowIndex, 2, new wxGridCellBoolRenderer);
			grid->SetCellEditor(rowIndex, 2, new wxGridCellBoolEditor);
			break;
		}
		default: {
			grid->SetCellValue(rowIndex, 1, "Unknown");
			grid->SetCellBackgroundColour(rowIndex, 1, *wxLIGHT_GREY);
			grid->SetCellBackgroundColour(rowIndex, 2, *wxLIGHT_GREY);
			grid->SetReadOnly(rowIndex, 1, true);
			grid->SetReadOnly(rowIndex, 2, true);
			break;
		}
	}
	grid->SetCellEditor(rowIndex, 1, new wxGridCellChoiceEditor(types));
}

void PropertiesWindow::OnResize(wxSizeEvent& evt) {
	/*
	if(wxGrid* grid = (wxGrid*)currentPanel->FindWindowByName("AdvancedGrid")) {
		int tWidth = 0;
		for(int i = 0; i < 3; ++i)
			tWidth += grid->GetColumnWidth(i);

		int wWidth = grid->GetParent()->GetSize().GetWidth();

		grid->SetColumnWidth(2, wWidth - 100 - 80);
	}
	*/
}

void PropertiesWindow::OnNotebookPageChanged(wxNotebookEvent& evt) {
	wxWindow* page = notebook->GetCurrentPage();

	// TODO: Save

	switch (page->GetId()) {
		case ITEM_PROPERTIES_GENERAL_TAB: {
			// currentPanel = createGeneralPanel(page);
			break;
		}
		case ITEM_PROPERTIES_ADVANCED_TAB: {
			// currentPanel = createAttributesPanel(page);
			break;
		}
		default:
			break;
	}
}

void PropertiesWindow::saveGeneralPanel() {
	////
}

void PropertiesWindow::saveContainerPanel() {
	////
}

void PropertiesWindow::saveAttributesPanel() {
	edit_item->clearAllAttributes();
	for (int32_t rowIndex = 0; rowIndex < attributesGrid->GetNumberRows(); ++rowIndex) {
		ItemAttribute attr;
		wxString type = attributesGrid->GetCellValue(rowIndex, 1);
		if (type == "String") {
			attr.set(nstr(attributesGrid->GetCellValue(rowIndex, 2)));
		} else if (type == "Float") {
			double value;
			if (attributesGrid->GetCellValue(rowIndex, 2).ToDouble(&value)) {
				attr.set(value);
			}
		} else if (type == "Number") {
			long value;
			if (attributesGrid->GetCellValue(rowIndex, 2).ToLong(&value)) {
				attr.set(static_cast<int32_t>(value));
			}
		} else if (type == "Boolean") {
			attr.set(attributesGrid->GetCellValue(rowIndex, 2) == "1");
		} else {
			continue;
		}
		edit_item->setAttribute(nstr(attributesGrid->GetCellValue(rowIndex, 0)), attr);
	}
}

void PropertiesWindow::OnGridValueChanged(wxGridEvent& event) {
	if (event.GetCol() == 1) {
		wxString newType = attributesGrid->GetCellValue(event.GetRow(), 1);
		if (newType == event.GetString()) {
			return;
		}

		ItemAttribute attr;
		if (newType == "String") {
			attr.set("");
		} else if (newType == "Float") {
			attr.set(0.0f);
		} else if (newType == "Number") {
			attr.set(0);
		} else if (newType == "Boolean") {
			attr.set(false);
		}
		SetGridValue(attributesGrid, event.GetRow(), nstr(attributesGrid->GetCellValue(event.GetRow(), 0)), attr);
	}
}

void PropertiesWindow::OnClickOK(wxCommandEvent&) {
	saveAttributesPanel();
	EndModal(1);
}

void PropertiesWindow::OnClickAddAttribute(wxCommandEvent&) {
	attributesGrid->AppendRows(1);
	ItemAttribute attr(0);
	SetGridValue(attributesGrid, attributesGrid->GetNumberRows() - 1, "", attr);
}

void PropertiesWindow::OnClickRemoveAttribute(wxCommandEvent&) {
	wxArrayInt rowIndexes = attributesGrid->GetSelectedRows();
	if (rowIndexes.Count() != 1) {
		return;
	}

	int rowIndex = rowIndexes[0];
	attributesGrid->DeleteRows(rowIndex, 1);
}

void PropertiesWindow::OnClickCancel(wxCommandEvent&) {
	EndModal(0);
}
""",
    "wxwidgets/properties_window.h": """//////////////////////////////////////////////////////////////////////
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

#ifndef RME_PROPERTIES_WINDOW_H_
#define RME_PROPERTIES_WINDOW_H_

#include "main.h"

#include "common_windows.h"

class ContainerItemButton;
class ContainerItemPopupMenu;
class ItemAttribute;

class PropertiesWindow : public ObjectPropertiesWindowBase {
public:
	PropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint position = wxDefaultPosition);
	~PropertiesWindow();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);
	void OnClickAddAttribute(wxCommandEvent&);
	void OnClickRemoveAttribute(wxCommandEvent&);

	void OnResize(wxSizeEvent&);
	void OnNotebookPageChanged(wxNotebookEvent&);
	void OnGridValueChanged(wxGridEvent&);

	void Update();

protected:
	// Simple pane
	wxWindow* createGeneralPanel(wxWindow* parent);
	void saveGeneralPanel();

	// Container pane
	std::vector<ContainerItemButton*> container_items;
	wxWindow* createContainerPanel(wxWindow* parent);
	void saveContainerPanel();

	// Advanced pane
	wxGrid* attributesGrid;
	wxWindow* createAttributesPanel(wxWindow* parent);
	void saveAttributesPanel();
	void SetGridValue(wxGrid* grid, int rowIndex, std::string name, const ItemAttribute& attr);

protected:
	wxNotebook* notebook;
	wxWindow* currentPanel;

	DECLARE_EVENT_TABLE()
};

#endif
""",
    "wxwidgets/old_properties_window.cpp": """//////////////////////////////////////////////////////////////////////
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

#include <wx/grid.h>

#include "tile.h"
#include "item.h"
#include "complexitem.h"
#include "town.h"
#include "house.h"
#include "map.h"
#include "editor.h"
#include "creature.h"

#include "gui.h"
#include "application.h"
#include "old_properties_window.h"
#include "container_properties_window.h"

// ============================================================================
// Old Properties Window


/*
Current Task - Old Properties Window Improvements:

1. Main Goal:
- Make the properties window non-blocking like replace_items_window.cpp
- Allow brush selection while properties window is open
- Fix crashes related to container and item handling (when we achieve it as a non main window)

2. Issues Being Addressed:
- Access violation when checking canHoldText() on null items
- Container handling crashes
- Creature direction field not updating properly
- Modal dialog blocking brush selection

3. Required Changes:
- Add proper null checks for item/creature/spawn pointers
- Improve container handling safety
- Fix creature direction field updates
- Make window non-modal while maintaining functionality all the bugs above are when we try to implement a non modal window
- Ensure proper cleanup of resources

4. Implementation Strategy:
- Add comprehensive null checks before accessing properties
- Improve error handling for container operations
- Fix creature direction field saving mechanism
- Modify window behavior to allow background interaction
- Maintain data integrity during property updates

5. Note:
NEVER REMOVE ANY PREVIOUS FUNCTIONALITY UNLESS SPECIFICALLY REQUESTED!
Keep all existing features while adding improvements.
*/

BEGIN_EVENT_TABLE(OldPropertiesWindow, wxDialog)
EVT_SET_FOCUS(OldPropertiesWindow::OnFocusChange)
EVT_BUTTON(wxID_OK, OldPropertiesWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, OldPropertiesWindow::OnClickCancel)
END_EVENT_TABLE()

static constexpr int OUTFIT_COLOR_MAX = 133;

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Item Properties", map, tile_parent, item, pos),
	count_field(nullptr),
	direction_field(nullptr),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	door_id_field(nullptr),
	tier_field(nullptr),
	depot_id_field(nullptr),
	splash_type_field(nullptr),
	text_field(nullptr),
	description_field(nullptr) {
	ASSERT(edit_item);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	if (Container* container = dynamic_cast<Container*>(edit_item)) {
		// Container
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Container Properties");

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
		action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
		subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
		unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
		subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

		boxsizer->Add(subsizer, wxSizerFlags(0).Expand());

		// Now we add the subitems!
		wxSizer* contents_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Contents");

		bool use_large_sprites = g_settings.getBoolean(Config::USE_LARGE_CONTAINER_ICONS);
		wxSizer* horizontal_sizer = nullptr;
		const int additional_height_increment = (use_large_sprites ? 40 : 24);
		int additional_height = 0;

		int32_t maxColumns;
		if (use_large_sprites) {
			maxColumns = 6;
		} else {
			maxColumns = 12;
		}

		for (uint32_t index = 0; index < container->getVolume(); ++index) {
			if (!horizontal_sizer) {
				horizontal_sizer = newd wxBoxSizer(wxHORIZONTAL);
			}

			Item* item = container->getItem(index);
			ContainerItemButton* containerItemButton = newd ContainerItemButton(this, use_large_sprites, index, map, item);

			container_items.push_back(containerItemButton);
			horizontal_sizer->Add(containerItemButton);

			if (((index + 1) % maxColumns) == 0) {
				contents_sizer->Add(horizontal_sizer);
				horizontal_sizer = nullptr;
				additional_height += additional_height_increment;
			}
		}

		if (horizontal_sizer != nullptr) {
			contents_sizer->Add(horizontal_sizer);
			additional_height += additional_height_increment;
		}

		boxsizer->Add(contents_sizer, wxSizerFlags(2).Expand());

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

		// SetSize(260, 150 + additional_height);
	} else if (edit_item->canHoldText() || edit_item->canHoldDescription()) {
		// Book
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Writeable Properties");

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
		action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
		action_id_field->SetSelection(-1, -1);
		subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
		unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
		subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

		boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

		wxSizer* textsizer = newd wxBoxSizer(wxVERTICAL);
		textsizer->Add(newd wxStaticText(this, wxID_ANY, "Text"), wxSizerFlags(1).Center());
		text_field = newd wxTextCtrl(this, wxID_ANY, wxstr(item->getText()), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
		textsizer->Add(text_field, wxSizerFlags(7).Expand());

		boxsizer->Add(textsizer, wxSizerFlags(2).Expand());

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

		// SetSize(220, 310);
	} else if (edit_item->isSplash() || edit_item->isFluidContainer()) {
		// Splash
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Splash Properties");

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Type"));

		// Splash types
		splash_type_field = newd wxChoice(this, wxID_ANY);
		if (edit_item->isFluidContainer()) {
			splash_type_field->Append(wxstr(Item::LiquidID2Name(LIQUID_NONE)), newd int32_t(LIQUID_NONE));
		}

		for (SplashType splashType = LIQUID_FIRST; splashType != LIQUID_LAST; ++splashType) {
			splash_type_field->Append(wxstr(Item::LiquidID2Name(splashType)), newd int32_t(splashType));
		}

		if (item->getSubtype()) {
			const std::string& what = Item::LiquidID2Name(item->getSubtype());
			if (what == "Unknown") {
				splash_type_field->Append(wxstr(Item::LiquidID2Name(LIQUID_NONE)), newd int32_t(LIQUID_NONE));
			}
			splash_type_field->SetStringSelection(wxstr(what));
		} else {
			splash_type_field->SetSelection(0);
		}

		subsizer->Add(splash_type_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
		action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
		subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
		unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
		subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

		boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

		// SetSize(220, 190);
	} else if (Depot* depot = dynamic_cast<Depot*>(edit_item)) {
		// Depot
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Depot Properties");
		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);

		subsizer->AddGrowableCol(1);
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		const Towns& towns = map->towns;
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Depot ID"));
		depot_id_field = newd wxChoice(this, wxID_ANY);
		int to_select_index = 0;
		if (towns.count() > 0) {
			bool found = false;
			for (TownMap::const_iterator town_iter = towns.begin();
				 town_iter != towns.end();
				 ++town_iter) {
				if (town_iter->second->getID() == depot->getDepotID()) {
					found = true;
				}
				depot_id_field->Append(wxstr(town_iter->second->getName()), newd int(town_iter->second->getID()));
				if (!found) {
					++to_select_index;
				}
			}
			if (!found) {
				if (depot->getDepotID() != 0) {
					depot_id_field->Append("Undefined Town (id:" + i2ws(depot->getDepotID()) + ")", newd int(depot->getDepotID()));
				}
			}
		}
		depot_id_field->Append("No Town", newd int(0));
		if (depot->getDepotID() == 0) {
			to_select_index = depot_id_field->GetCount() - 1;
		}
		depot_id_field->SetSelection(to_select_index);

		subsizer->Add(depot_id_field, wxSizerFlags(1).Expand());

		boxsizer->Add(subsizer, wxSizerFlags(5).Expand());
		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));
		// SetSize(220, 140);
	} else {
		// Normal item
		Door* door = dynamic_cast<Door*>(edit_item);
		Teleport* teleport = dynamic_cast<Teleport*>(edit_item);
		Podium* podium = dynamic_cast<Podium*>(edit_item);

		wxString description;
		if (door) {
			ASSERT(tile_parent);
			description = "Door Properties";
		} else if (teleport) {
			description = "Teleport Properties";
		} else if (podium) {
			description = "Podium Properties";
		} else {
			description = "Item Properties";
		}

		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, description);

		// unused(?)
		/*
		int num_items = 4;
		//if(item->canHoldDescription()) num_items += 1;
		if(door) num_items += 1;
		if(teleport) num_items += 1;
		*/

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, (item->isCharged() ? "Charges" : "Count")));
		int max_count = 100;
		if (item->isClientCharged()) {
			max_count = 250;
		}
		if (item->isExtraCharged()) {
			max_count = 65500;
		}
		count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getCount()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, max_count, edit_item->getCount());
		if (!item->isStackable() && !item->isCharged()) {
			count_field->Enable(false);
		}
		subsizer->Add(count_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
		action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
		subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
		unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
		subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

		// item classification (12.81+)
		if (g_items.MajorVersion >= 3 && g_items.MinorVersion >= 60 && (edit_item->getClassification() > 0 || edit_item->isWeapon() || edit_item->isWearableEquipment())) {
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Classification"));
			subsizer->Add(newd wxStaticText(this, wxID_ANY, i2ws(item->getClassification())));

			// item iter
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Tier"));
			tier_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getTier()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFF, edit_item->getTier());
			subsizer->Add(tier_field, wxSizerFlags(1).Expand());
		}

		/*
		if(item->canHoldDescription()) {
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Description"));
			description_field = newd wxTextCtrl(this, wxID_ANY, edit_item->getText(), wxDefaultPosition, wxSize(-1, 20));
			subsizer->Add(description_field, wxSizerFlags(1).Expand());
		}
		*/

		if (door) {
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Door ID"));
			door_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(door->getDoorID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFF, door->getDoorID());
			if (!edit_tile || !edit_tile->isHouseTile() || !door->isRealDoor()) {
				door_id_field->Disable();
			}
			subsizer->Add(door_id_field, wxSizerFlags(1).Expand());
		}

		if (teleport) {
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Destination"));

			wxSizer* possizer = newd wxBoxSizer(wxHORIZONTAL);
			x_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getX()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, map->getWidth(), teleport->getX());
			x_field->Bind(wxEVT_CHAR, &OldPropertiesWindow::OnChar, this);
			possizer->Add(x_field, wxSizerFlags(3).Expand());
			y_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getY()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, map->getHeight(), teleport->getY());
			y_field->Bind(wxEVT_CHAR, &OldPropertiesWindow::OnChar, this);
			possizer->Add(y_field, wxSizerFlags(3).Expand());
			z_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getZ()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, MAP_MAX_LAYER, teleport->getZ());
			z_field->Bind(wxEVT_CHAR, &OldPropertiesWindow::OnChar, this);
			possizer->Add(z_field, wxSizerFlags(2).Expand());

			subsizer->Add(possizer, wxSizerFlags(1).Expand());
		}

		if (podium) {
			// direction
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Direction"));
			direction_field = newd wxChoice(this, wxID_ANY);

			for (Direction dir = DIRECTION_FIRST; dir <= DIRECTION_LAST; ++dir) {
				direction_field->Append(wxstr(Creature::DirID2Name(dir)), newd int32_t(dir));
			}
			direction_field->SetSelection(static_cast<Direction>(podium->getDirection()));
			subsizer->Add(direction_field, wxSizerFlags(1).Expand());

			// checkboxes
			show_outfit = newd wxCheckBox(this, wxID_ANY, "Show outfit");
			show_outfit->SetValue(podium->hasShowOutfit());
			show_outfit->SetToolTip("Display outfit on the podium.");
			subsizer->Add(show_outfit, 0, wxLEFT | wxTOP, 5);
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "")); // filler for checkboxes

			show_mount = newd wxCheckBox(this, wxID_ANY, "Show mount");
			show_mount->SetValue(podium->hasShowMount());
			show_mount->SetToolTip("Display mount on the podium.");
			subsizer->Add(show_mount, 0, wxLEFT | wxTOP, 5);
			subsizer->Add(newd wxStaticText(this, wxID_ANY, ""));

			show_platform = newd wxCheckBox(this, wxID_ANY, "Show platform");
			show_platform->SetValue(podium->hasShowPlatform());
			show_platform->SetToolTip("Display the podium platform.");
			subsizer->Add(show_platform, 0, wxLEFT | wxTOP, 5);
			subsizer->Add(newd wxStaticText(this, wxID_ANY, ""));

			// outfit container
			wxFlexGridSizer* outfitContainer = newd wxFlexGridSizer(2, 10, 10);
			const Outfit& outfit = podium->getOutfit();

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Outfit"));
			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, ""));

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "LookType"));
			look_type = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookType), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, std::numeric_limits<uint16_t>::max(), outfit.lookType);
			outfitContainer->Add(look_type, wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Head"));
			look_head = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookHead), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookHead);
			outfitContainer->Add(look_head, wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Body"));
			look_body = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookBody), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookBody);
			outfitContainer->Add(look_body, wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Legs"));
			look_legs = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookLegs), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookLegs);
			outfitContainer->Add(look_legs, wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Feet"));
			look_feet = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookFeet), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookFeet);
			outfitContainer->Add(look_feet, wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Addons"));
			look_addon = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookAddon), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 3, outfit.lookAddon);
			outfitContainer->Add(look_addon, wxSizerFlags(3).Expand());

			// mount container
			wxFlexGridSizer* mountContainer = newd wxFlexGridSizer(2, 10, 10);

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Mount"));
			mountContainer->Add(newd wxStaticText(this, wxID_ANY, ""));

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "LookMount"));
			look_mount = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMount), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, std::numeric_limits<uint16_t>::max(), outfit.lookMount);
			mountContainer->Add(look_mount, wxSizerFlags(3).Expand());

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Head"));
			look_mounthead = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountHead), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountHead);
			mountContainer->Add(look_mounthead, wxSizerFlags(3).Expand());

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Body"));
			look_mountbody = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountBody), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountBody);
			mountContainer->Add(look_mountbody, wxSizerFlags(3).Expand());

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Legs"));
			look_mountlegs = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountLegs), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountLegs);
			mountContainer->Add(look_mountlegs, wxSizerFlags(3).Expand());

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Feet"));
			look_mountfeet = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountFeet), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountFeet);
			mountContainer->Add(look_mountfeet, wxSizerFlags(3).Expand());

			wxFlexGridSizer* propertiesContainer = newd wxFlexGridSizer(3, 10, 10);
			propertiesContainer->Add(subsizer, wxSizerFlags(1).Expand());
			propertiesContainer->Add(outfitContainer, wxSizerFlags(1).Expand());
			propertiesContainer->Add(mountContainer, wxSizerFlags(1).Expand());
			boxsizer->Add(propertiesContainer, wxSizerFlags(1).Expand());
		} else {
			boxsizer->Add(subsizer, wxSizerFlags(1).Expand());
		}

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 20));
	}

	// Others attributes
	// should have an option to turn it off
	/*
	const ItemType& type = g_items.getItemType(edit_item->getID());
	wxStaticBoxSizer* others_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Others");
	wxFlexGridSizer* others_subsizer = newd wxFlexGridSizer(2, 5, 10);
	others_subsizer->AddGrowableCol(1);
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Stackable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.stackable)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Movable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.moveable)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Pickupable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.pickupable)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Hangable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.isHangable)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Block Missiles"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.blockMissiles)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Block Pathfinder"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.blockPathfinder)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Has Elevation"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.hasElevation)));
	others_sizer->Add(others_subsizer, wxSizerFlags(1).Expand());
	topsizer->Add(others_sizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT | wxBOTTOM, 20));
	*/

	wxSizer* subsizer = newd wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	subsizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(subsizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Creature* creature, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Creature Properties", map, tile_parent, creature, pos),
	count_field(nullptr),
	direction_field(nullptr),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	door_id_field(nullptr),
	tier_field(nullptr),
	depot_id_field(nullptr),
	splash_type_field(nullptr),
	text_field(nullptr),
	description_field(nullptr) {
	ASSERT(edit_creature);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Creature Properties");

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Creature "));
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(edit_creature->getName()) + "\""), wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Spawn interval"));
	count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_creature->getSpawnTime()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 86400, edit_creature->getSpawnTime());
	// count_field->SetSelection(-1, -1);
	subsizer->Add(count_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Direction"));
	direction_field = newd wxChoice(this, wxID_ANY);

	for (Direction dir = DIRECTION_FIRST; dir <= DIRECTION_LAST; ++dir) {
		direction_field->Append(wxstr(Creature::DirID2Name(dir)), newd int32_t(dir));
	}
	direction_field->SetSelection(edit_creature->getDirection());
	subsizer->Add(direction_field, wxSizerFlags(1).Expand());

	boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(3).Expand().Border(wxALL, 20));
	// SetSize(220, 0);

	wxSizer* std_sizer = newd wxBoxSizer(wxHORIZONTAL);
	std_sizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	std_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(std_sizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Spawn* spawn, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Spawn Properties", map, tile_parent, spawn, pos),
	count_field(nullptr),
	direction_field(nullptr),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	door_id_field(nullptr),
	tier_field(nullptr),
	depot_id_field(nullptr),
	splash_type_field(nullptr),
	text_field(nullptr),
	description_field(nullptr) {
	ASSERT(edit_spawn);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Spawn Properties");

	// if(item->canHoldDescription()) num_items += 1;

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Spawn size"));
	count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_spawn->getSize()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, g_settings.getInteger(Config::MAX_SPAWN_RADIUS), edit_spawn->getSize());
	// count_field->SetSelection(-1, -1);
	subsizer->Add(count_field, wxSizerFlags(1).Expand());

	boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(3).Expand().Border(wxALL, 20));

	wxSizer* std_sizer = newd wxBoxSizer(wxHORIZONTAL);
	std_sizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	std_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(std_sizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

OldPropertiesWindow::~OldPropertiesWindow() {
	// Warning: edit_item may no longer be valid, DONT USE IT!
	if (splash_type_field) {
		for (uint32_t i = 0; i < splash_type_field->GetCount(); ++i) {
			delete reinterpret_cast<int*>(splash_type_field->GetClientData(i));
		}
	}
	if (direction_field) {
		for (uint32_t i = 0; i < direction_field->GetCount(); ++i) {
			delete reinterpret_cast<int*>(direction_field->GetClientData(i));
		}
	}
	if (depot_id_field) {
		for (uint32_t i = 0; i < depot_id_field->GetCount(); ++i) {
			delete reinterpret_cast<int*>(depot_id_field->GetClientData(i));
		}
	}
}

void OldPropertiesWindow::OnFocusChange(wxFocusEvent& event) {
	wxWindow* win = event.GetWindow();
	if (wxSpinCtrl* spin = dynamic_cast<wxSpinCtrl*>(win)) {
		spin->SetSelection(-1, -1);
	} else if (wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(win)) {
		text->SetSelection(-1, -1);
	}
}

void OldPropertiesWindow::OnChar(wxKeyEvent& evt) {
	if (evt.GetKeyCode() == WXK_CONTROL_V) {
		Position position;
		const Editor* const editor = g_gui.GetCurrentEditor();
		if (posFromClipboard(position, editor->getMapWidth(), editor->getMapHeight())) {
			x_field->SetValue(position.x);
			y_field->SetValue(position.y);
			z_field->SetValue(position.z);
			return;
		}
	}

	evt.Skip();
}

void OldPropertiesWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	if (edit_item) {
		if (dynamic_cast<Container*>(edit_item)) {
			// Container
			int new_uid = unique_id_field->GetValue();
			int new_aid = action_id_field->GetValue();

			if ((new_uid < 1000 || new_uid > 0xFFFF) && new_uid != 0) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
				return;
			}
			if (/* there is no item with the same UID */ false) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be unique, this UID is already taken.", wxOK);
				return;
			}
			if ((new_aid < 100 || new_aid > 0xFFFF) && new_aid != 0) {
				g_gui.PopupDialog(this, "Error", "Action ID must be between 100 and 65535.", wxOK);
				return;
			}

			edit_item->setUniqueID(new_uid);
			edit_item->setActionID(new_aid);
		} else if (edit_item->canHoldText() || edit_item->canHoldDescription()) {
			// Book
			int new_uid = unique_id_field->GetValue();
			int new_aid = action_id_field->GetValue();
			std::string text = nstr(text_field->GetValue());

			if ((new_uid < 1000 || new_uid > 0xFFFF) && new_uid != 0) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
				return;
			}
			if (/* there is no item with the same UID */ false) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be unique, this UID is already taken.", wxOK);
				return;
			}
			if ((new_aid < 100 || new_aid > 0xFFFF) && new_aid != 0) {
				g_gui.PopupDialog(this, "Error", "Action ID must be between 100 and 65535.", wxOK);
				return;
			}
			if (text.length() >= 0xFFFF) {
				g_gui.PopupDialog(this, "Error", "Text is longer than 65535 characters, this is not supported by OpenTibia. Reduce the length of the text.", wxOK);
				return;
			}
			if (edit_item->canHoldText() && text.length() > edit_item->getMaxWriteLength()) {
				int ret = g_gui.PopupDialog(this, "Error", "Text is longer than the maximum supported length of this book type, do you still want to change it?", wxYES | wxNO);
				if (ret != wxID_YES) {
					return;
				}
			}

			edit_item->setUniqueID(new_uid);
			edit_item->setActionID(new_aid);
			edit_item->setText(text);
		} else if (edit_item->isSplash() || edit_item->isFluidContainer()) {
			// Splash
			int new_uid = unique_id_field->GetValue();
			int new_aid = action_id_field->GetValue();
			int* new_type = reinterpret_cast<int*>(splash_type_field->GetClientData(splash_type_field->GetSelection()));

			if ((new_uid < 1000 || new_uid > 0xFFFF) && new_uid != 0) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
				return;
			}
			if (/* there is no item with the same UID */ false) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be unique, this UID is already taken.", wxOK);
				return;
			}
			if ((new_aid < 100 || new_aid > 0xFFFF) && new_aid != 0) {
				g_gui.PopupDialog(this, "Error", "Action ID must be between 100 and 65535.", wxOK);
				return;
			}
			if (new_type) {
				edit_item->setSubtype(*new_type);
			}
			edit_item->setUniqueID(new_uid);
			edit_item->setActionID(new_aid);

			// Clean up client data
		} else if (Depot* depot = dynamic_cast<Depot*>(edit_item)) {
			// Depot
			int* new_depotid = reinterpret_cast<int*>(depot_id_field->GetClientData(depot_id_field->GetSelection()));

			depot->setDepotID(*new_depotid);
		} else {
			// Normal item
			Door* door = dynamic_cast<Door*>(edit_item);
			Teleport* teleport = dynamic_cast<Teleport*>(edit_item);
			Podium* podium = dynamic_cast<Podium*>(edit_item);

			int new_uid = unique_id_field->GetValue();
			int new_aid = action_id_field->GetValue();
			int new_count = count_field ? count_field->GetValue() : 1;
			int new_tier = tier_field ? tier_field->GetValue() : 0;

			std::string new_desc;
			if (edit_item->canHoldDescription() && description_field) {
				description_field->GetValue();
			}
			Position new_dest;
			if (teleport) {
				new_dest = Position(x_field->GetValue(), y_field->GetValue(), z_field->GetValue());
			}
			uint8_t new_door_id = 0;
			if (door) {
				new_door_id = door_id_field->GetValue();
			}

			if ((new_uid < 1000 || new_uid > 0xFFFF) && new_uid != 0) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
				return;
			}
			if (/* there is no item with the same UID */ false) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be unique, this UID is already taken.", wxOK);
				return;
			}
			if ((new_aid < 100 || new_aid > 0xFFFF) && new_aid != 0) {
				g_gui.PopupDialog(this, "Error", "Action ID must be between 100 and 65535.", wxOK);
				return;
			}
			if (new_tier < 0 || new_tier > 0xFF) {
				g_gui.PopupDialog(this, "Error", "Item tier must be between 0 and 255.", wxOK);
				return;
			}

			/*
			if(edit_item->canHoldDescription()) {
				if(new_desc.length() > 127) {
					g_gui.PopupDialog("Error", "Description must be shorter than 127 characters.", wxOK);
					return;
				}
			}
			*/

			if (door && g_settings.getInteger(Config::WARN_FOR_DUPLICATE_ID)) {
				if (edit_tile && edit_tile->isHouseTile()) {
					const House* house = edit_map->houses.getHouse(edit_tile->getHouseID());
					if (house) {
						Position pos = house->getDoorPositionByID(new_door_id);
						if (pos == Position()) {
							// Do nothing
						} else if (pos != edit_tile->getPosition()) {
							int ret = g_gui.PopupDialog(this, "Warning", "This doorid conflicts with another one in this house, are you sure you want to continue?", wxYES | wxNO);
							if (ret == wxID_NO) {
								return;
							}
						}
					}
				}
			}

			if (teleport) {
				if (edit_map->getTile(new_dest) == nullptr || edit_map->getTile(new_dest)->isBlocking()) {
					int ret = g_gui.PopupDialog(this, "Warning", "This teleport leads nowhere, or to an invalid location. Do you want to change the destination?", wxYES | wxNO);
					if (ret == wxID_YES) {
						return;
					}
				}
			}

			Outfit& newOutfit = Outfit();
			if (podium) {
				int newLookType = look_type->GetValue();
				int newMount = look_mount->GetValue();

				if (newLookType < 0 || newLookType > 0xFFFF || newMount < 0 || newMount > 0xFFFF) {
					g_gui.PopupDialog(this, "Error", "LookType and Mount must be between 0 and 65535.", wxOK);
					return;
				}

				int newHead = look_head->GetValue();
				int newBody = look_body->GetValue();
				int newLegs = look_legs->GetValue();
				int newFeet = look_feet->GetValue();
				int newMountHead = look_mounthead->GetValue();
				int newMountBody = look_mountbody->GetValue();
				int newMountLegs = look_mountlegs->GetValue();
				int newMountFeet = look_mountfeet->GetValue();

				if (newHead < 0 || newHead > OUTFIT_COLOR_MAX || newBody < 0 || newBody > OUTFIT_COLOR_MAX || newLegs < 0 || newLegs > OUTFIT_COLOR_MAX || newFeet < 0 || newFeet > OUTFIT_COLOR_MAX || newMountHead < 0 || newMountHead > OUTFIT_COLOR_MAX || newMountBody < 0 || newMountBody > OUTFIT_COLOR_MAX || newMountLegs < 0 || newMountLegs > OUTFIT_COLOR_MAX || newMountFeet < 0 || newMountFeet > OUTFIT_COLOR_MAX) {
					wxString response = "Outfit and mount colors must be between 0 and ";
					response << i2ws(OUTFIT_COLOR_MAX) << ".";
					g_gui.PopupDialog(this, "Error", response, wxOK);
					return;
				}

				int newAddon = look_addon->GetValue();
				if (newAddon < 0 || newAddon > 3) {
					g_gui.PopupDialog(this, "Error", "Addons value must be between 0 and 3.", wxOK);
					return;
				}

				newOutfit.lookType = newLookType;
				newOutfit.lookHead = newHead;
				newOutfit.lookBody = newBody;
				newOutfit.lookLegs = newLegs;
				newOutfit.lookFeet = newFeet;
				newOutfit.lookAddon = newAddon;
				newOutfit.lookMount = newMount;
				newOutfit.lookMountHead = newMountHead;
				newOutfit.lookMountBody = newMountBody;
				newOutfit.lookMountLegs = newMountLegs;
				newOutfit.lookMountFeet = newMountFeet;
			}

			// Done validating, set the values.
			if (edit_item->canHoldDescription()) {
				edit_item->setText(new_desc);
			}
			if (edit_item->isStackable() || edit_item->isCharged()) {
				edit_item->setSubtype(new_count);
			}
			if (door) {
				door->setDoorID(new_door_id);
			}
			if (teleport) {
				teleport->setDestination(new_dest);
			}
			if (podium) {
				podium->setShowOutfit(show_outfit->GetValue());
				podium->setShowMount(show_mount->GetValue());
				podium->setShowPlatform(show_platform->GetValue());

				int* new_dir = reinterpret_cast<int*>(direction_field->GetClientData(direction_field->GetSelection()));
				if (new_dir) {
					podium->setDirection((Direction)*new_dir);
				}

				podium->setOutfit(newOutfit);
			}
			edit_item->setUniqueID(new_uid);
			edit_item->setActionID(new_aid);
			edit_item->setTier(new_tier);
		}
	} else if (edit_creature) {
		int new_spawntime = count_field->GetValue();
		edit_creature->setSpawnTime(new_spawntime);

		int* new_dir = reinterpret_cast<int*>(direction_field->GetClientData(
			direction_field->GetSelection()
		));

		if (new_dir) {
			edit_creature->setDirection((Direction)*new_dir);
		}
	} else if (edit_spawn) {
		int new_spawnsize = count_field->GetValue();
		edit_spawn->setSize(new_spawnsize);
	}
	EndModal(1);
}

void OldPropertiesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	// Just close this window
	EndModal(0);
}

void OldPropertiesWindow::Update() {
	Container* container = dynamic_cast<Container*>(edit_item);
	if (container) {
		for (uint32_t i = 0; i < container->getVolume(); ++i) {
			container_items[i]->setItem(container->getItem(i));
		}
	}
	wxDialog::Update();
}
""",
    "wxwidgets/old_properties_window.h": """//////////////////////////////////////////////////////////////////////
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

#ifndef RME_OLD_PROPERTIES_WINDOW_H_
#define RME_OLD_PROPERTIES_WINDOW_H_

#include "main.h"

#include "common_windows.h"

class ContainerItemButton;
class ContainerItemPopupMenu;
class MapWindow;

class OldPropertiesWindow : public ObjectPropertiesWindowBase {
public:
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint = wxDefaultPosition);
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Creature* creature, wxPoint = wxDefaultPosition);
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Spawn* spawn, wxPoint = wxDefaultPosition);
	virtual ~OldPropertiesWindow();

	void OnFocusChange(wxFocusEvent&);
	void OnChar(wxKeyEvent& evt);
	void OnKeyDown(wxKeyEvent& evt);
	void OnTextEnter(wxCommandEvent& evt);

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);
	void OnClose(wxCloseEvent& evt);

	void Update();

	// Non-modal methods
	static OldPropertiesWindow* getInstance() { return instance; }
	static void destroyInstance() {
		if (instance) {
			instance->Destroy();
			instance = nullptr;
		}
	}
	static bool isActive() { return instance != nullptr; }

	void CommitChanges();

protected:
	// Singleton instance for non-modal use
	static OldPropertiesWindow* instance;

	// item
	wxSpinCtrl* count_field;
	wxSpinCtrl* action_id_field;
	wxSpinCtrl* unique_id_field;
	wxSpinCtrl* door_id_field;
	wxSpinCtrl* tier_field;
	wxChoice* depot_id_field;
	wxChoice* splash_type_field;
	wxTextCtrl* text_field;
	wxTextCtrl* description_field;

	// teleport
	wxSpinCtrl* x_field;
	wxSpinCtrl* y_field;
	wxSpinCtrl* z_field;

	// podium
	wxCheckBox* show_outfit;
	wxCheckBox* show_mount;
	wxCheckBox* show_platform;
	wxSpinCtrl* look_type;
	wxSpinCtrl* look_head;
	wxSpinCtrl* look_body;
	wxSpinCtrl* look_legs;
	wxSpinCtrl* look_feet;
	wxSpinCtrl* look_addon;
	wxSpinCtrl* look_mount;
	wxSpinCtrl* look_mounthead;
	wxSpinCtrl* look_mountbody;
	wxSpinCtrl* look_mountlegs;
	wxSpinCtrl* look_mountfeet;

	// podium and creature
	wxChoice* direction_field;

	// container
	std::vector<ContainerItemButton*> container_items;

	friend class ContainerItemButton;
	friend class ContainerItemPopupMenu;

	DECLARE_EVENT_TABLE();
};

#endif
""",
    "wxwidgets/container_properties_window.cpp": """//////////////////////////////////////////////////////////////////////
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

#include "container_properties_window.h"

#include "old_properties_window.h"
#include "properties_window.h"
#include "find_item_window.h"
#include "gui.h"
#include "complexitem.h"
#include "map.h"

// ============================================================================
// Container Item Button
// Displayed in the container object properties menu, needs some
// custom event handling for the right-click menu etcetera so we
// need to define a custom class for it.

std::unique_ptr<ContainerItemPopupMenu> ContainerItemButton::popup_menu;

BEGIN_EVENT_TABLE(ContainerItemButton, ItemButton)
EVT_LEFT_DOWN(ContainerItemButton::OnMouseDoubleLeftClick)
EVT_RIGHT_UP(ContainerItemButton::OnMouseRightRelease)

EVT_MENU(CONTAINER_POPUP_MENU_ADD, ContainerItemButton::OnAddItem)
EVT_MENU(CONTAINER_POPUP_MENU_EDIT, ContainerItemButton::OnEditItem)
EVT_MENU(CONTAINER_POPUP_MENU_REMOVE, ContainerItemButton::OnRemoveItem)
END_EVENT_TABLE()

ContainerItemButton::ContainerItemButton(wxWindow* parent, bool large, int _index, const Map* map, Item* item) :
	ItemButton(parent, (large ? RENDER_SIZE_32x32 : RENDER_SIZE_16x16), (item ? item->getClientID() : 0)),
	edit_map(map),
	edit_item(item),
	index(_index) {
	////
}

ContainerItemButton::~ContainerItemButton() {
	////
}

void ContainerItemButton::OnMouseDoubleLeftClick(wxMouseEvent& WXUNUSED(event)) {
	wxCommandEvent dummy;

	if (edit_item) {
		OnEditItem(dummy);
		return;
	}

	Container* container = getParentContainer();
	if (container->getVolume() > container->getItemCount()) {
		OnAddItem(dummy);
	}
}

void ContainerItemButton::OnMouseRightRelease(wxMouseEvent& WXUNUSED(event)) {
	if (!popup_menu) {
		popup_menu.reset(newd ContainerItemPopupMenu);
	}

	popup_menu->Update(this);
	PopupMenu(popup_menu.get());
}

void ContainerItemButton::OnAddItem(wxCommandEvent& WXUNUSED(event)) {
	FindItemDialog dialog(GetParent(), "Choose Item to add", true);

	if (dialog.ShowModal() == wxID_OK) {
		Container* container = getParentContainer();
		ItemVector& itemVector = container->getVector();

		Item* item = Item::Create(dialog.getResultID());
		if (index < itemVector.size()) {
			itemVector.insert(itemVector.begin() + index, item);
		} else {
			itemVector.push_back(item);
		}

		ObjectPropertiesWindowBase* propertyWindow = getParentContainerWindow();
		if (propertyWindow) {
			propertyWindow->Update();
		}
	}
	dialog.Destroy();
}

void ContainerItemButton::OnEditItem(wxCommandEvent& WXUNUSED(event)) {
	ASSERT(edit_item);

	wxPoint newDialogAt;
	wxWindow* w = this;
	while ((w = w->GetParent())) {
		if (ObjectPropertiesWindowBase* o = dynamic_cast<ObjectPropertiesWindowBase*>(w)) {
			newDialogAt = o->GetPosition();
			break;
		}
	}

	newDialogAt += wxPoint(20, 20);

	wxDialog* d;

	if (edit_map->getVersion().otbm >= MAP_OTBM_4) {
		d = newd PropertiesWindow(this, edit_map, nullptr, edit_item, newDialogAt);
	} else {
		d = newd OldPropertiesWindow(this, edit_map, nullptr, edit_item, newDialogAt);
	}

	d->ShowModal();
	d->Destroy();
}

void ContainerItemButton::OnRemoveItem(wxCommandEvent& WXUNUSED(event)) {
	ASSERT(edit_item);
	int32_t ret = g_gui.PopupDialog(GetParent(), "Remove Item", "Are you sure you want to remove this item from the container?", wxYES | wxNO);

	if (ret != wxID_YES) {
		return;
	}

	Container* container = getParentContainer();
	ItemVector& itemVector = container->getVector();

	auto it = itemVector.begin();
	for (; it != itemVector.end(); ++it) {
		if (*it == edit_item) {
			break;
		}
	}

	ASSERT(it != itemVector.end());

	itemVector.erase(it);
	delete edit_item;

	ObjectPropertiesWindowBase* propertyWindow = getParentContainerWindow();
	if (propertyWindow) {
		propertyWindow->Update();
	}
}

void ContainerItemButton::setItem(Item* item) {
	edit_item = item;
	if (edit_item) {
		SetSprite(edit_item->getClientID());
	} else {
		SetSprite(0);
	}
}

ObjectPropertiesWindowBase* ContainerItemButton::getParentContainerWindow() {
	for (wxWindow* window = GetParent(); window != nullptr; window = window->GetParent()) {
		ObjectPropertiesWindowBase* propertyWindow = dynamic_cast<ObjectPropertiesWindowBase*>(window);
		if (propertyWindow) {
			return propertyWindow;
		}
	}
	return nullptr;
}

Container* ContainerItemButton::getParentContainer() {
	ObjectPropertiesWindowBase* propertyWindow = getParentContainerWindow();
	if (propertyWindow) {
		return dynamic_cast<Container*>(propertyWindow->getItemBeingEdited());
	}
	return nullptr;
}

// ContainerItemPopupMenu
ContainerItemPopupMenu::ContainerItemPopupMenu() :
	wxMenu("") {
	////
}

ContainerItemPopupMenu::~ContainerItemPopupMenu() {
	////
}

void ContainerItemPopupMenu::Update(ContainerItemButton* btn) {
	// Clear the menu of all items
	while (GetMenuItemCount() != 0) {
		wxMenuItem* m_item = FindItemByPosition(0);
		// If you add a submenu, this won't delete it.
		Delete(m_item);
	}

	wxMenuItem* addItem = nullptr;
	if (btn->edit_item) {
		Append(CONTAINER_POPUP_MENU_EDIT, "&Edit Item", "Open the properties menu for this item");
		addItem = Append(CONTAINER_POPUP_MENU_ADD, "&Add Item", "Add a newd item to the container");
		Append(CONTAINER_POPUP_MENU_REMOVE, "&Remove Item", "Remove this item from the container");
	} else {
		addItem = Append(CONTAINER_POPUP_MENU_ADD, "&Add Item", "Add a newd item to the container");
	}

	Container* parentContainer = btn->getParentContainer();
	if (parentContainer->getVolume() <= (int)parentContainer->getVector().size()) {
		addItem->Enable(false);
	}
}
""",
    "wxwidgets/container_properties_window.h": """//////////////////////////////////////////////////////////////////////
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

#ifndef _RME_CONTAINER_PROPS_H_
#define _RME_CONTAINER_PROPS_H_

#include "common_windows.h"

class Container;
class ContainerItemButton;

// Right-click popup menu
class ContainerItemPopupMenu : public wxMenu {
public:
	ContainerItemPopupMenu();
	virtual ~ContainerItemPopupMenu();

	void Update(ContainerItemButton* what);
};

// Container Item Button
class ContainerItemButton : public ItemButton {
	DECLARE_EVENT_TABLE()
public:
	ContainerItemButton(wxWindow* parent, bool large, int index, const Map* map, Item* item);
	~ContainerItemButton();

	void OnMouseDoubleLeftClick(wxMouseEvent& event);
	void OnMouseRightRelease(wxMouseEvent& event);

	void OnAddItem(wxCommandEvent& event);
	void OnEditItem(wxCommandEvent& event);
	void OnRemoveItem(wxCommandEvent& event);

	ObjectPropertiesWindowBase* getParentContainerWindow();
	Container* getParentContainer();

	void setItem(Item* item);

private:
	static std::unique_ptr<ContainerItemPopupMenu> popup_menu;

	const Map* edit_map;
	Item* edit_item;

	size_t index;

	friend class ContainerItemPopupMenu;
};

#endif
"""
}

analyzed_files_data = []
for file_path, content in cpp_files_content.items():
    analyzed_files_data.append({
        "file_path": file_path,
        "description": "Header for PropertiesWindow." if ".h" in file_path and "properties_window" in file_path
                       else "Implements PropertiesWindow, a wxDialog with a wxNotebook for editing item properties (General, Contents for containers, Advanced custom attributes using wxGrid)." if ".cpp" in file_path and "properties_window" in file_path
                       else "Header for OldPropertiesWindow." if ".h" in file_path and "old_properties_window" in file_path
                       else "Implements OldPropertiesWindow, a wxDialog that handles property editing for various types including generic items, containers, writeables, splashes, depots, doors, teleports, podiums, creatures, and spawns, using wxFlexGridSizer for layout." if ".cpp" in file_path and "old_properties_window" in file_path
                       else "Header for ContainerItemButton and ContainerItemPopupMenu." if ".h" in file_path and "container_properties_window" in file_path
                       else "Implements ContainerItemButton (custom ItemButton for display in container properties) and ContainerItemPopupMenu for context menu actions on container items." if ".cpp" in file_path and "container_properties_window" in file_path
                       else "Unknown C++ file.",
        "md5_hash": get_md5_hash(content),
        "content_lite": get_content_lite(content)
    })

yaml_output = {
    "wbs_item_id": "UI-04",
    "name": "Port Item Properties & Editors",
    "description": "Recreate UI dialogs for viewing and editing properties of map items (general, container contents, custom attributes), creatures (spawn time, direction), and spawns (radius) using Qt6.",
    "dependencies": [
        "CORE-02 # For ItemDatabase to get item type information and attributes.",
        "UI-06   # Item finder dialog might be used by container editor."
        "# Implicitly depends on Map, Tile, Item, Creature, Spawn data structures."
    ],
    "input_files": [
        "wxwidgets/properties_window.cpp",
        "wxwidgets/properties_window.h",
        "wxwidgets/old_properties_window.cpp",
        "wxwidgets/old_properties_window.h",
        "wxwidgets/container_properties_window.cpp",
        "wxwidgets/container_properties_window.h"
    ],
    "analyzed_input_files": analyzed_files_data,
    "documentation_references": [
        "QDialog: https://doc.qt.io/qt-6/qdialog.html",
        "QTabWidget: https://doc.qt.io/qt-6/qtabwidget.html",
        "QSpinBox, QLineEdit, QTextEdit, QComboBox, QCheckBox for input fields.",
        "QTableWidget (for custom attributes): https://doc.qt.io/qt-6/qtablewidget.html",
        "QStyledItemDelegate (for custom editors in QTableWidget/QTreeView): https://doc.qt.io/qt-6/qstyleditemdelegate.html",
        "QListView with IconMode: https://doc.qt.io/qt-6/qlistview.html#viewMode-prop"
    ],
    "current_functionality_summary": """The wxWidgets version provides two main item properties dialogs:
1.  `PropertiesWindow` (newer): Uses a notebook with tabs for "Simple" (ID, Name, ActionID, UniqueID), "Contents" (for containers, displays items in a grid of `ContainerItemButton`s with context menus), and "Advanced" (a `wxGrid` for arbitrary key-type-value attributes).
2.  `OldPropertiesWindow`: A more direct dialog that customizes its layout based on the item type (generic, container, writeable, splash, depot, door, teleport, podium) or if editing a Creature or Spawn. It uses various `wxSpinCtrl`, `wxTextCtrl`, and `wxChoice` controls.
Both dialogs allow modification of common attributes like ActionID and UniqueID, and specialized attributes based on type. Container editing involves adding, removing, or editing items within the container.""",
    "definition_of_done": [
        "A Qt6 `ItemPropertiesDialog` (subclass of `QDialog`) is implemented, using a `QTabWidget` for 'General', 'Contents', and 'Advanced Attributes' sections.",
        "**General Tab:**",
        "  - Displays read-only Item ID and Name.",
        "  - Provides `QSpinBox` controls for Action ID and Unique ID.",
        "  - Dynamically displays and allows editing of type-specific attributes based on the item being edited (e.g., count/subtype for stackables/fluids, text for writeables, destination for teleports, depot ID for depots, door ID, podium outfit details) using appropriate Qt widgets (`QSpinBox`, `QLineEdit`, `QTextEdit`, `QComboBox`, `QCheckBox`).",
        "**Contents Tab (for Container Items):**",
        "  - This tab is only visible if the edited item is a container.",
        "  - Displays contained items in a grid view (e.g., `QListView` in `IconMode` or a custom grid of `QPushButton`-like widgets displaying item sprites).",
        "  - A context menu on container slots/items allows 'Add Item' (opens item finder from UI-06), 'Edit Item Properties' (opens a new `ItemPropertiesDialog`), and 'Remove Item'.",
        "**Advanced Attributes Tab:**",
        "  - A `QTableWidget` (or `QTreeView` with a model) allows viewing, adding, removing, and editing custom key-value attributes.",
        "  - The 'Key' is a string. The 'Type' is selectable from a `QComboBox` (String, Integer, Float, Boolean). The 'Value' cell uses an appropriate editor based on the selected type.",
        "Separate, simpler `QDialog` subclasses are created for:",
        "  - `CreaturePropertiesDialog`: Edits spawn interval (`QSpinBox`) and direction (`QComboBox`).",
        "  - `SpawnPropertiesDialog`: Edits spawn radius (`QSpinBox`).",
        "All dialogs correctly load properties from the passed object (Item, Creature, Spawn) on showing and apply validated changes back to a *copy* of the object when 'OK' is clicked. The calling code handles the actual map modification and undo action.",
        "Input validation is performed for relevant fields (e.g., numeric ranges, string lengths)."
    ],
    "boilerplate_coder_ai_prompt": """Your task is to port the item, creature, and spawn properties editing dialogs from wxWidgets to Qt6. The main item properties dialog will be tabbed for general properties, container contents, and custom key-value attributes.

**Reference Files:** `wxwidgets/properties_window.*`, `wxwidgets/old_properties_window.*`, `wxwidgets/container_properties_window.*`.

**1. `ItemPropertiesDialog` (Main Dialog, inherits `QDialog`):**
   - Use a `QTabWidget` for the main sections.

   **a. 'General' Tab:**
      - `QLabel` for Item ID (read-only).
      - `QLabel` for Item Name (read-only).
      - `QSpinBox` for Action ID (range 0-65535).
      - `QSpinBox` for Unique ID (range 0-65535, typically 1000-65535 if not 0).
      - **Dynamic Section (based on ItemType):**
          - If stackable/charged: `QSpinBox` for Count/Charges. Max value depends on item type.
          - If writeable: `QTextEdit` for Text.
          - If splash/fluid: `QComboBox` for Liquid Type (populate with known types).
          - If depot: `QComboBox` for Depot Town ID (populate with towns from map).
          - If door: `QSpinBox` for Door ID (relevant if on a house tile).
          - If teleport: `QSpinBox`es for Dest X, Y, Z.
          - If podium: `QComboBox` for Direction; `QCheckBox`es for Show Outfit, Mount, Platform; `QSpinBox`es for LookType, Head, Body, Legs, Feet, Addons, LookMount, and mount colors.
          - If tiered item (version >= 12.81): `QSpinBox` for Tier (0-255).

   **b. 'Contents' Tab (Visible only for Containers):**
      - Use a `QListView` in `QListView::IconMode` with a custom model, or a `QGridLayout` of custom `ItemButtonWidget` (subclass `QPushButton` or `QToolButton`) to display items in the container. Each button shows item sprite. Store the slot index.
      - Implement `customContextMenuRequested` on the view/buttons to show a `QMenu` with "Add Item...", "Edit Item...", "Remove Item".
      - "Add Item...": Opens an item finder dialog (from `UI-06`). Places selected item in an empty slot or appends.
      - "Edit Item...": Opens a new `ItemPropertiesDialog` for the item in that slot.
      - "Remove Item": Clears the item from that slot.

   **c. 'Advanced Attributes' Tab:**
      - Use `QTableWidget` with columns: "Key" (QString), "Type" (QComboBox: String, Integer, Float, Boolean), "Value".
      - Buttons: "Add Attribute", "Remove Selected Attribute".
      - When "Type" `QComboBox` changes, set an appropriate editor for the "Value" cell (e.g., `QLineEdit` for String, `QSpinBox` for Integer, `QDoubleSpinBox` for Float, `QCheckBox` for Boolean). Use `QTableWidget::setCellWidget` or a `QStyledItemDelegate`.

   **Dialog Logic:**
      - Constructor takes `(QWidget* parent, const Map* map, const Tile* tile, Item* itemToEditCopy)`.
      - Populate fields from `itemToEditCopy` when shown.
      - On "OK" click: Validate inputs. Update `itemToEditCopy` with new values. `accept()` the dialog. The caller is responsible for creating an undo action with this modified copy.

**2. `CreaturePropertiesDialog` (inherits `QDialog`):**
   - `QLabel` for Creature Name (read-only).
   - `QSpinBox` for Spawn Interval (seconds, e.g., 10-3600).
   - `QComboBox` for Direction (North, East, South, West, etc.).
   - Populate from `Creature*` copy, save back to copy on OK.

**3. `SpawnPropertiesDialog` (inherits `QDialog`):**
   - `QSpinBox` for Spawn Radius (e.g., 1-15).
   - Populate from `Spawn*` (or `SpawnProperties*`) copy, save back to copy on OK."""
}

output_filename = "enhanced_wbs_yaml_files/UI-04.yaml"
output_dir = os.path.dirname(output_filename)

if not os.path.exists(output_dir):
    os.makedirs(output_dir)

with open(output_filename, 'w') as f:
    yaml.dump(yaml_output, f, sort_keys=False, width=1000, allow_unicode=True)

print(f"Successfully generated {output_filename}")
