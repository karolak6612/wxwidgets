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
    "wxwidgets/palette_creature.cpp": """//////////////////////////////////////////////////////////////////////
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

#include "settings.h"
#include "brush.h"
#include "gui.h"
#include "palette_creature.h"
#include "creature_brush.h"
#include "spawn_brush.h"
#include "materials.h"
#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/textdlg.h>
#include "creature_sprite_manager.h"

// Define the new event ID for the Load NPCs button
#define PALETTE_LOAD_NPCS_BUTTON 1952
#define PALETTE_LOAD_MONSTERS_BUTTON 1953
#define PALETTE_PURGE_CREATURES_BUTTON 1954
#define PALETTE_SEARCH_BUTTON 1955
#define PALETTE_SEARCH_FIELD 1956
#define PALETTE_VIEW_TOGGLE_BUTTON 1957
#define PALETTE_CREATURE_LARGE_SPRITES_TOGGLE 1958
#define PALETTE_CREATURE_ZOOM_BUTTON 1959

// ============================================================================
// Creature palette

BEGIN_EVENT_TABLE(CreaturePalettePanel, PalettePanel)
EVT_CHOICE(PALETTE_CREATURE_TILESET_CHOICE, CreaturePalettePanel::OnTilesetChange)
EVT_LISTBOX(PALETTE_CREATURE_LISTBOX, CreaturePalettePanel::OnListBoxChange)
EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_LISTBOX_SELECTED, CreaturePalettePanel::OnSpriteSelected)
EVT_TOGGLEBUTTON(PALETTE_CREATURE_BRUSH_BUTTON, CreaturePalettePanel::OnClickCreatureBrushButton)
EVT_TOGGLEBUTTON(PALETTE_SPAWN_BRUSH_BUTTON, CreaturePalettePanel::OnClickSpawnBrushButton)
EVT_TOGGLEBUTTON(PALETTE_VIEW_TOGGLE_BUTTON, CreaturePalettePanel::OnClickViewToggle)
EVT_TOGGLEBUTTON(PALETTE_CREATURE_VIEW_STYLE_TOGGLE, CreaturePalettePanel::OnClickViewStyleToggle)
EVT_TOGGLEBUTTON(PALETTE_CREATURE_LARGE_SPRITES_TOGGLE, CreaturePalettePanel::OnClickLargeSpritesToggle)
EVT_BUTTON(PALETTE_CREATURE_ZOOM_BUTTON, CreaturePalettePanel::OnClickZoomButton)
EVT_BUTTON(PALETTE_LOAD_NPCS_BUTTON, CreaturePalettePanel::OnClickLoadNPCsButton)
EVT_BUTTON(PALETTE_LOAD_MONSTERS_BUTTON, CreaturePalettePanel::OnClickLoadMonstersButton)
EVT_BUTTON(PALETTE_PURGE_CREATURES_BUTTON, CreaturePalettePanel::OnClickPurgeCreaturesButton)
EVT_BUTTON(PALETTE_SEARCH_BUTTON, CreaturePalettePanel::OnClickSearchButton)
EVT_TEXT(PALETTE_SEARCH_FIELD, CreaturePalettePanel::OnSearchFieldText)
EVT_SPINCTRL(PALETTE_CREATURE_SPAWN_TIME, CreaturePalettePanel::OnChangeSpawnTime)
EVT_SPINCTRL(PALETTE_CREATURE_SPAWN_SIZE, CreaturePalettePanel::OnChangeSpawnSize)
END_EVENT_TABLE()

CreaturePalettePanel::CreaturePalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	tileset_choice(nullptr),
	creature_list(nullptr),
	sprite_panel(nullptr),
	seamless_panel(nullptr),
	view_toggle(nullptr),
	view_style_toggle(nullptr),
	large_sprites_toggle(nullptr),
	zoom_button(nullptr),
	view_sizer(nullptr),
	use_sprite_view(false),
	use_seamless_view(true), // Seamless is now the default
	use_large_sprites(false),
	zoom_factor(1),
	handling_event(false),
	search_field(nullptr),
	search_button(nullptr),
	load_npcs_button(nullptr),
	load_monsters_button(nullptr),
	purge_creatures_button(nullptr),
	creature_spawntime_spin(nullptr),
	spawn_size_spin(nullptr),
	creature_brush_button(nullptr),
	spawn_brush_button(nullptr) {

	// Create the controls
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Creatures");

	// Tileset choice
	tileset_choice = newd wxChoice(this, PALETTE_CREATURE_TILESET_CHOICE, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY);
	sidesizer->Add(tileset_choice, 0, wxEXPAND | wxALL, 5);

	// Search field
	wxBoxSizer* searchSizer = newd wxBoxSizer(wxHORIZONTAL);
	searchSizer->Add(newd wxStaticText(this, wxID_ANY, "Search:"), 0, wxCENTER | wxLEFT, 5);
	search_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	searchSizer->Add(search_field, 1, wxCENTER | wxLEFT, 5);
	search_button = newd wxButton(this, PALETTE_SEARCH_BUTTON, "Go", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	searchSizer->Add(search_button, 0, wxLEFT, 5);
	sidesizer->Add(searchSizer, 0, wxEXPAND | wxTOP, 5);

	// Connect the focus events to disable hotkeys during typing
	search_field->Connect(wxEVT_SET_FOCUS, wxFocusEventHandler(CreaturePalettePanel::OnSearchFieldFocus), nullptr, this);
	search_field->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CreaturePalettePanel::OnSearchFieldKillFocus), nullptr, this);
	// Connect key down event to handle key presses in the search field
	search_field->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(CreaturePalettePanel::OnSearchFieldKeyDown), nullptr, this);

	// Create view container that will hold both list and sprite views
	view_sizer = newd wxBoxSizer(wxVERTICAL);

	// Create both views
	creature_list = newd SortableListBox(this, PALETTE_CREATURE_LISTBOX);
	sprite_panel = newd CreatureSpritePanel(this);
	seamless_panel = newd CreatureSeamlessGridPanel(this);

	// Add views to sizer (only one will be shown at a time)
	view_sizer->Add(creature_list, 1, wxEXPAND);
	view_sizer->Add(sprite_panel, 1, wxEXPAND);
	view_sizer->Add(seamless_panel, 1, wxEXPAND);
	sprite_panel->Hide(); // Initially hide the sprite view
	seamless_panel->Hide(); // Initially hide the seamless view

	sidesizer->Add(view_sizer, 1, wxEXPAND | wxTOP, 5);

	// Add buttons for loading NPCs, monsters, and purging creatures
	wxSizer* buttonSizer = newd wxBoxSizer(wxHORIZONTAL);

	load_npcs_button = newd wxButton(this, PALETTE_LOAD_NPCS_BUTTON, "Load NPCs Folder");
	buttonSizer->Add(load_npcs_button, 1, wxEXPAND | wxRIGHT, 5);

	load_monsters_button = newd wxButton(this, PALETTE_LOAD_MONSTERS_BUTTON, "Load Monsters Folder");
	buttonSizer->Add(load_monsters_button, 1, wxEXPAND | wxLEFT, 5);

	sidesizer->Add(buttonSizer, 0, wxEXPAND | wxTOP, 5);

	purge_creatures_button = newd wxButton(this, PALETTE_PURGE_CREATURES_BUTTON, "Purge Creatures");
	sidesizer->Add(purge_creatures_button, 0, wxEXPAND | wxTOP, 5);

	// View mode toggle
	wxBoxSizer* viewModeSizer = newd wxBoxSizer(wxHORIZONTAL);
	view_toggle = newd wxToggleButton(this, PALETTE_VIEW_TOGGLE_BUTTON, "Sprite View");
	viewModeSizer->Add(view_toggle, 1, wxEXPAND);

	// Large sprites toggle
	large_sprites_toggle = newd wxToggleButton(this, PALETTE_CREATURE_LARGE_SPRITES_TOGGLE, "64x64");
	large_sprites_toggle->Enable(false); // Only enabled in sprite view
	viewModeSizer->Add(large_sprites_toggle, 1, wxEXPAND | wxLEFT, 5);

	// Zoom button
	zoom_button = newd wxButton(this, PALETTE_CREATURE_ZOOM_BUTTON, "Zoom 2x");
	zoom_button->Enable(false); // Only enabled in sprite view with large sprites
	viewModeSizer->Add(zoom_button, 1, wxEXPAND | wxLEFT, 5);

	sidesizer->Add(viewModeSizer, 0, wxEXPAND | wxTOP, 5);

	// Add brush radio buttons
	wxToggleButton* creature_radio = newd wxToggleButton(this, PALETTE_CREATURE_BRUSH_BUTTON, "Creature");
	wxToggleButton* spawn_radio = newd wxToggleButton(this, PALETTE_SPAWN_BRUSH_BUTTON, "Spawn");

	wxBoxSizer* radiosizer = newd wxBoxSizer(wxHORIZONTAL);
	radiosizer->Add(creature_radio, 1, wxEXPAND);
	radiosizer->Add(spawn_radio, 1, wxEXPAND);

	sidesizer->Add(radiosizer, 0, wxEXPAND | wxTOP, 5);

	// Store references to the radio buttons
	creature_brush_button = creature_radio;
	spawn_brush_button = spawn_radio;

	// Add spawn settings
	wxFlexGridSizer* settings_sizer = newd wxFlexGridSizer(2, 5, 5);
	settings_sizer->AddGrowableCol(1);
	settings_sizer->Add(newd wxStaticText(this, wxID_ANY, "Spawntime"));

	creature_spawntime_spin = newd wxSpinCtrl(this, PALETTE_CREATURE_SPAWN_TIME, i2ws(g_settings.getInteger(Config::DEFAULT_SPAWNTIME)),
											wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 3600, g_settings.getInteger(Config::DEFAULT_SPAWNTIME));

	settings_sizer->Add(creature_spawntime_spin, 0, wxEXPAND);
	settings_sizer->Add(newd wxStaticText(this, wxID_ANY, "Size"));

	spawn_size_spin = newd wxSpinCtrl(this, PALETTE_CREATURE_SPAWN_SIZE, i2ws(5),
									  wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, 5);

	settings_sizer->Add(spawn_size_spin, 0, wxEXPAND);

	sidesizer->Add(settings_sizer, 0, wxEXPAND | wxTOP, 5);
	topsizer->Add(sidesizer, 1, wxEXPAND | wxALL, 5);

	SetSizerAndFit(topsizer);

	// Load all creatures
	TilesetContainer tilesets;

	// Create a list of all creature tilesets
	for (TilesetContainer::const_iterator iter = g_materials.tilesets.begin(); iter != g_materials.tilesets.end(); ++iter) {
		const TilesetCategory* tc = iter->second->getCategory(TILESET_CREATURE);
		if (tc && tc->size() > 0) {
			if (tilesets.count(iter->first) == 0)
				tilesets[iter->first] = iter->second;
		}
	}

	// Add them to the choice control
	for (TilesetContainer::const_iterator iter = tilesets.begin(); iter != tilesets.end(); ++iter) {
		tileset_choice->Append(wxstr(iter->second->name), const_cast<TilesetCategory*>(iter->second->getCategory(TILESET_CREATURE)));
	}

	// Add the rest of the tilesets as before
	for (TilesetContainer::const_iterator iter = g_materials.tilesets.begin(); iter != g_materials.tilesets.end(); ++iter) {
		if (iter->first == "All Creatures") continue;  // Skip since we already added it

		const TilesetCategory* tsc = iter->second->getCategory(TILESET_CREATURE);
		if (tsc && tsc->size() > 0) {
			tileset_choice->Append(wxstr(iter->second->name), const_cast<TilesetCategory*>(tsc));
		} else if (iter->second->name == "NPCs" || iter->second->name == "Others") {
			Tileset* ts = const_cast<Tileset*>(iter->second);
			TilesetCategory* rtsc = ts->getCategory(TILESET_CREATURE);
			tileset_choice->Append(wxstr(ts->name), rtsc);
		}
	}
	SelectTileset(0);
}

CreaturePalettePanel::~CreaturePalettePanel() {
	////
}

PaletteType CreaturePalettePanel::GetType() const {
	return TILESET_CREATURE;
}

void CreaturePalettePanel::SelectFirstBrush() {
	if (use_sprite_view) {
		if (use_seamless_view) {
			// Select first creature in seamless panel
			if (!seamless_panel->creatures.empty()) {
				seamless_panel->SelectIndex(0);
			}
		} else {
			// Select first creature in sprite panel
			if (!sprite_panel->creatures.empty()) {
				sprite_panel->SelectIndex(0);
			}
		}
	} else {
		// Select first creature in list
		if (creature_list->GetCount() > 0) {
			creature_list->SetSelection(0);
		}
	}
}

Brush* CreaturePalettePanel::GetSelectedBrush() const {
	if (use_sprite_view) {
		if (use_seamless_view) {
			return seamless_panel->GetSelectedBrush();
		} else {
			return sprite_panel->GetSelectedBrush();
		}
	} else {
		if (creature_list->GetCount() > 0 && creature_list->GetSelection() != wxNOT_FOUND) {
			const Brush* brush = reinterpret_cast<const Brush*>(creature_list->GetClientData(creature_list->GetSelection()));
			if (brush) {
				if (g_gui.GetCurrentBrush() != brush) {
					g_gui.SelectBrush(const_cast<Brush*>(brush), TILESET_CREATURE);
				}
			}
			return const_cast<Brush*>(brush);
		}
		return nullptr;
	}
}

bool CreaturePalettePanel::SelectBrush(const Brush* whatbrush) {
	if (!whatbrush) {
		if (use_sprite_view) {
			if (use_seamless_view) {
				seamless_panel->SelectBrush(nullptr);
			} else {
				sprite_panel->SelectBrush(nullptr);
			}
		} else {
			creature_list->SetSelection(wxNOT_FOUND);
		}
		return true;
	}

	if (whatbrush->isCreature()) {
		if (use_sprite_view) {
			if (use_seamless_view) {
				return seamless_panel->SelectBrush(whatbrush);
			} else {
				return sprite_panel->SelectBrush(whatbrush);
			}
		} else {
			for (size_t i = 0; i < creature_list->GetCount(); ++i) {
				const Brush* tmp_brush = reinterpret_cast<const Brush*>(creature_list->GetClientData(i));
				if (tmp_brush == whatbrush) {
					creature_list->SetSelection(i);
					return true;
				}
			}
		}
	}
	return false;
}

int CreaturePalettePanel::GetSelectedBrushSize() const {
	return spawn_size_spin->GetValue();
}

void CreaturePalettePanel::OnUpdate() {
	tileset_choice->Clear();
	g_materials.createOtherTileset();

	// Create an "All Creatures" tileset that contains all creatures
	Tileset* allCreatures = nullptr;
	TilesetCategory* allCreaturesCategory = nullptr;

	// Check if the "All Creatures" tileset already exists, if not create it
	if (g_materials.tilesets.count("All Creatures") > 0) {
		allCreatures = g_materials.tilesets["All Creatures"];
		allCreaturesCategory = allCreatures->getCategory(TILESET_CREATURE);
		allCreaturesCategory->brushlist.clear();
	} else {
		allCreatures = newd Tileset(g_brushes, "All Creatures");
		g_materials.tilesets["All Creatures"] = allCreatures;
		allCreaturesCategory = allCreatures->getCategory(TILESET_CREATURE);
	}

	// Track added creatures to avoid duplicates
	std::set<std::string> addedCreatures;

	// Collect all creature brushes from all tilesets
	for (TilesetContainer::const_iterator iter = g_materials.tilesets.begin(); iter != g_materials.tilesets.end(); ++iter) {
		if (iter->first == "All Creatures") continue;  // Skip ourselves to avoid duplication

		const TilesetCategory* tsc = iter->second->getCategory(TILESET_CREATURE);
		if (tsc && tsc->size() > 0) {
			// Add all creature brushes from this category to the All Creatures category
			for (BrushVector::const_iterator brushIter = tsc->brushlist.begin(); brushIter != tsc->brushlist.end(); ++brushIter) {
				if ((*brushIter)->isCreature()) {
					// Only add if not already added (avoid duplicates)
					std::string creatureName = (*brushIter)->getName();
					if (addedCreatures.count(creatureName) == 0) {
						allCreaturesCategory->brushlist.push_back(*brushIter);
						addedCreatures.insert(creatureName);
					}
				}
			}
		}
	}

	// Add the "All Creatures" tileset first
	tileset_choice->Append(wxstr(allCreatures->name), allCreaturesCategory);

	// Add the rest of the tilesets as before
	for (TilesetContainer::const_iterator iter = g_materials.tilesets.begin(); iter != g_materials.tilesets.end(); ++iter) {
		if (iter->first == "All Creatures") continue;  // Skip since we already added it

		const TilesetCategory* tsc = iter->second->getCategory(TILESET_CREATURE);
		if (tsc && tsc->size() > 0) {
			tileset_choice->Append(wxstr(iter->second->name), const_cast<TilesetCategory*>(tsc));
		} else if (iter->second->name == "NPCs" || iter->second->name == "Others") {
			Tileset* ts = const_cast<Tileset*>(iter->second);
			TilesetCategory* rtsc = ts->getCategory(TILESET_CREATURE);
			tileset_choice->Append(wxstr(ts->name), rtsc);
		}
	}
	SelectTileset(0);
}

void CreaturePalettePanel::OnUpdateBrushSize(BrushShape shape, int size) {
	return spawn_size_spin->SetValue(size);
}

void CreaturePalettePanel::OnSwitchIn() {
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SetBrushSize(spawn_size_spin->GetValue());
}

void CreaturePalettePanel::SelectTileset(size_t index) {
	ASSERT(tileset_choice->GetCount() >= index);

	creature_list->Clear();
	sprite_panel->Clear();
	seamless_panel->Clear();

	if (tileset_choice->GetCount() == 0) {
		// No tilesets :(
		creature_brush_button->Enable(false);
	} else {
		const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));

		// Add creatures to appropriate view
		if (use_sprite_view) {
			sprite_panel->LoadCreatures(tsc->brushlist);
		} else {
			// Add creatures to list view
			for (BrushVector::const_iterator iter = tsc->brushlist.begin();
				iter != tsc->brushlist.end();
				++iter) {
				// Check if this creature has outfit colors
				CreatureBrush* cb = dynamic_cast<CreatureBrush*>(*iter);
				if (cb && cb->getType()) {
					const Outfit& outfit = cb->getType()->outfit;

					// Create name for list display
					std::string name = (*iter)->getName();

					// If this creature has custom outfit colors, add an indicator to the name
					if (outfit.lookHead > 0 || outfit.lookBody > 0 || outfit.lookLegs > 0 || outfit.lookFeet > 0) {
						name += " [outfit]";
					}

					creature_list->Append(wxstr(name), *iter);
				} else {
					// Regular creature without custom outfit
					creature_list->Append(wxstr((*iter)->getName()), *iter);
				}
			}
			creature_list->Sort();
		}

		// Apply filter if search field has text
		if (!search_field->IsEmpty()) {
			FilterCreatures(search_field->GetValue());
		} else {
			SelectCreature(0);
		}

		tileset_choice->SetSelection(index);
	}
}

void CreaturePalettePanel::SelectCreature(size_t index) {
	// Select creature by index
	if (use_sprite_view) {
		// In sprite view, select by index
		if (index < sprite_panel->creatures.size()) {
			sprite_panel->SelectIndex(index);
		}
	} else {
		// In list view, select by index
		if (creature_list->GetCount() > 0 && index < creature_list->GetCount()) {
			creature_list->SetSelection(index);
		}
	}

	SelectCreatureBrush();
}

void CreaturePalettePanel::SelectCreature(std::string name) {
	if (use_sprite_view) {
		// In sprite view, find and select brush by name
		for (size_t i = 0; i < sprite_panel->creatures.size(); ++i) {
			if (sprite_panel->creatures[i]->getName() == name) {
				sprite_panel->SelectIndex(i);
				break;
			}
		}
	} else {
		// In list view, select by name string
		if (creature_list->GetCount() > 0) {
			if (!creature_list->SetStringSelection(wxstr(name))) {
				creature_list->SetSelection(0);
			}
		}
	}

	SelectCreatureBrush();
}

void CreaturePalettePanel::SelectCreatureBrush() {
	bool has_selection = false;

	if (use_sprite_view) {
		has_selection = (sprite_panel->GetSelectedBrush() != nullptr);
	} else {
		has_selection = (creature_list->GetCount() > 0);
	}

	if (has_selection) {
		creature_brush_button->Enable(true);
		creature_brush_button->SetValue(true);
		spawn_brush_button->SetValue(false);
	} else {
		creature_brush_button->Enable(false);
		SelectSpawnBrush();
	}
}

void CreaturePalettePanel::SelectSpawnBrush() {
	// g_gui.house_exit_brush->setHouse(house);
	creature_brush_button->SetValue(false);
	spawn_brush_button->SetValue(true);
}

void CreaturePalettePanel::OnTilesetChange(wxCommandEvent& event) {
	SelectTileset(event.GetSelection());
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void CreaturePalettePanel::OnListBoxChange(wxCommandEvent& event) {
	// Get the selected brush before updating
	Brush* old_brush = g_gui.GetCurrentBrush();

	// Update selection
	SelectCreature(event.GetSelection());
	g_gui.ActivatePalette(GetParentPalette());

	// Get the newly selected brush
	Brush* new_brush = g_gui.GetCurrentBrush();

	// If we selected the same brush, first set to nullptr then reselect
	if(old_brush && new_brush && old_brush == new_brush) {
		g_gui.SelectBrush(nullptr, TILESET_CREATURE);
	}

	// Now select the brush (either for the first time or re-selecting)
	g_gui.SelectBrush();
}

void CreaturePalettePanel::OnClickCreatureBrushButton(wxCommandEvent& event) {
	SelectCreatureBrush();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void CreaturePalettePanel::OnClickSpawnBrushButton(wxCommandEvent& event) {
	SelectSpawnBrush();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void CreaturePalettePanel::OnClickLoadNPCsButton(wxCommandEvent& event) {
	wxDirDialog dlg(g_gui.root, "Select NPC folder", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK) {
		wxString folder = dlg.GetPath();
		LoadNPCsFromFolder(folder);
	}
}

void CreaturePalettePanel::OnClickLoadMonstersButton(wxCommandEvent& event) {
	wxDirDialog dlg(g_gui.root, "Select Monsters folder", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK) {
		wxString folder = dlg.GetPath();
		LoadMonstersFromFolder(folder);
	}
}

void CreaturePalettePanel::OnClickPurgeCreaturesButton(wxCommandEvent& event) {
	// Confirmation dialog
	long response = wxMessageBox("Are you sure you want to purge all creatures from the palette? This cannot be undone.",
		"Confirm Purge", wxYES_NO | wxICON_QUESTION, g_gui.root);

	if (response == wxYES) {
		PurgeCreaturePalettes();
	}
}

bool CreaturePalettePanel::LoadNPCsFromFolder(const wxString& folder) {
	// Get all .xml files in the folder
	wxArrayString files;
	wxDir::GetAllFiles(folder, &files, "*.xml", wxDIR_FILES);

	if (files.GetCount() == 0) {
		wxMessageBox("No XML files found in the selected folder.", "Error", wxOK | wxICON_INFORMATION, g_gui.root);
		return false;
	}

	wxArrayString warnings;
	int loadedCount = 0;

	for (size_t i = 0; i < files.GetCount(); ++i) {
		wxString error;
		bool ok = g_creatures.importXMLFromOT(FileName(files[i]), error, warnings);
		if (ok) {
			loadedCount++;
		} else {
			warnings.Add("Failed to load " + files[i] + ": " + error);
		}
	}

	if (!warnings.IsEmpty()) {
		g_gui.ListDialog("NPC loader messages", warnings);
	}

	if (loadedCount > 0) {
		g_gui.PopupDialog("Success", wxString::Format("Successfully loaded %d NPC files.", loadedCount), wxOK);

		// Refresh the palette
		g_gui.RefreshPalettes();

		// Refresh current tileset and creature list
		OnUpdate();

		return true;
	} else {
		wxMessageBox("No NPCs could be loaded from the selected folder.", "Error", wxOK | wxICON_INFORMATION, g_gui.root);
		return false;
	}
}

bool CreaturePalettePanel::LoadMonstersFromFolder(const wxString& folder) {
	// Get all .xml files in the folder
	wxArrayString files;
	wxDir::GetAllFiles(folder, &files, "*.xml", wxDIR_FILES);

	if (files.GetCount() == 0) {
		wxMessageBox("No XML files found in the selected folder.", "Error", wxOK | wxICON_INFORMATION, g_gui.root);
		return false;
	}

	wxArrayString warnings;
	int loadedCount = 0;

	for (size_t i = 0; i < files.GetCount(); ++i) {
		wxString error;
		bool ok = g_creatures.importXMLFromOT(FileName(files[i]), error, warnings);
		if (ok) {
			loadedCount++;
		} else {
			warnings.Add("Failed to load " + files[i] + ": " + error);
		}
	}

	if (!warnings.IsEmpty()) {
		g_gui.ListDialog("Monster loader messages", warnings);
	}

	if (loadedCount > 0) {
		g_gui.PopupDialog("Success", wxString::Format("Successfully loaded %d monster files.", loadedCount), wxOK);

		// Refresh the palette
		g_gui.RefreshPalettes();

		// Refresh current tileset and creature list
		OnUpdate();

		return true;
	} else {
		wxMessageBox("No monsters could be loaded from the selected folder.", "Error", wxOK | wxICON_INFORMATION, g_gui.root);
		return false;
	}
}

bool CreaturePalettePanel::PurgeCreaturePalettes() {
	// Track success
	bool success = false;

	// Create vectors to store brushes that need to be removed
	std::vector<Brush*> brushesToRemove;

	// Collect creature brushes from the "NPCs", "Others", and "All Creatures" tilesets
	if (g_materials.tilesets.count("All Creatures") > 0) {
		Tileset* allCreaturesTileset = g_materials.tilesets["All Creatures"];
		TilesetCategory* allCreaturesCategory = allCreaturesTileset->getCategory(TILESET_CREATURE);
		if (allCreaturesCategory) {
			allCreaturesCategory->brushlist.clear();
			success = true;
		}
	}

	if (g_materials.tilesets.count("NPCs") > 0) {
		Tileset* npcTileset = g_materials.tilesets["NPCs"];
		TilesetCategory* npcCategory = npcTileset->getCategory(TILESET_CREATURE);
		if (npcCategory) {
			for (BrushVector::iterator it = npcCategory->brushlist.begin(); it != npcCategory->brushlist.end(); ++it) {
				brushesToRemove.push_back(*it);
			}
			npcCategory->brushlist.clear();
			success = true;
		}
	}

	if (g_materials.tilesets.count("Others") > 0) {
		Tileset* othersTileset = g_materials.tilesets["Others"];
		TilesetCategory* othersCategory = othersTileset->getCategory(TILESET_CREATURE);
		if (othersCategory) {
			for (BrushVector::iterator it = othersCategory->brushlist.begin(); it != othersCategory->brushlist.end(); ++it) {
				brushesToRemove.push_back(*it);
			}
			othersCategory->brushlist.clear();
			success = true;
		}
	}

	// Remove creature brushes from g_brushes
	// We need to collect the keys to remove first to avoid modifying the map during iteration
	const BrushMap& allBrushes = g_brushes.getMap();
	std::vector<std::string> brushKeysToRemove;

	for (BrushMap::const_iterator it = allBrushes.begin(); it != allBrushes.end(); ++it) {
		if (it->second && it->second->isCreature()) {
			brushKeysToRemove.push_back(it->first);
		}
	}

	// Now remove the brushes from g_brushes
	for (std::vector<std::string>::iterator it = brushKeysToRemove.begin(); it != brushKeysToRemove.end(); ++it) {
		g_brushes.removeBrush(*it);
	}

	// Delete the brush objects to prevent memory leaks
	for (std::vector<Brush*>::iterator it = brushesToRemove.begin(); it != brushesToRemove.end(); ++it) {
		delete *it;
	}

	// Clear creature database
	g_creatures.clear();

	// Recreate empty tilesets if needed
	g_materials.createOtherTileset();

	// Refresh the palette
	g_gui.RefreshPalettes();

	// Refresh current tileset and creature list in this panel
	OnUpdate();

	if (success) {
		g_gui.PopupDialog("Success", "All creatures have been purged from the palettes.", wxOK);
	} else {
		wxMessageBox("There was a problem purging the creatures.", "Error", wxOK | wxICON_ERROR, g_gui.root);
	}

	return success;
}

void CreaturePalettePanel::OnChangeSpawnTime(wxSpinEvent& event) {
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SetSpawnTime(event.GetPosition());
}

void CreaturePalettePanel::OnChangeSpawnSize(wxSpinEvent& event) {
	if (!handling_event) {
		handling_event = true;
		g_gui.ActivatePalette(GetParentPalette());
		g_gui.SetBrushSize(event.GetPosition());
		handling_event = false;
	}
}

void CreaturePalettePanel::OnClickSearchButton(wxCommandEvent& event) {
	// Get the text from the search field and filter
	wxString searchText = search_field->GetValue();
	FilterCreatures(searchText);
}

void CreaturePalettePanel::OnSearchFieldText(wxCommandEvent& event) {
	// Filter as user types
	FilterCreatures(search_field->GetValue());
}

void CreaturePalettePanel::OnSearchFieldFocus(wxFocusEvent& event) {
	// Disable hotkeys when search field receives focus
	g_gui.DisableHotkeys();
	event.Skip();
}

void CreaturePalettePanel::OnSearchFieldKillFocus(wxFocusEvent& event) {
	// Re-enable hotkeys when search field loses focus
	g_gui.EnableHotkeys();
	event.Skip();
}

void CreaturePalettePanel::OnSearchFieldKeyDown(wxKeyEvent& event) {
	// Handle Enter key specially
	if (event.GetKeyCode() == WXK_RETURN) {
		FilterCreatures(search_field->GetValue());
	} else if (event.GetKeyCode() == WXK_ESCAPE) {
		// Clear search field and reset the list on Escape
		search_field->Clear();
		FilterCreatures(wxEmptyString);
		// Set focus back to the map
		wxWindow* mapCanvas = g_gui.root->FindWindowByName("MapCanvas");
		if (mapCanvas) {
			mapCanvas->SetFocus();
		}
	} else {
		// Process the event normally for all other keys
		event.Skip();
	}
}

void CreaturePalettePanel::FilterCreatures(const wxString& search_text) {
	if (tileset_choice->GetCount() == 0) return;

	// If search is empty, reset to show all creatures
	if (search_text.IsEmpty()) {
		int currentSelection = tileset_choice->GetSelection();
		if (currentSelection != wxNOT_FOUND) {
			SelectTileset(currentSelection);
		}
		return;
	}

	wxString searchLower = search_text.Lower();

	// Check if we're searching for a specific looktype (format: "lt:123" or "looktype:123")
	bool isLooktypeSearch = false;
	int searchLooktype = 0;

	if (searchLower.StartsWith("lt:") || searchLower.StartsWith("looktype:")) {
		wxString looktypeStr = searchLower.AfterFirst(':');
		if (looktypeStr.ToInt(&searchLooktype)) {
			isLooktypeSearch = true;
		}
	}

	// Clear current content
	BrushVector filtered_brushes;
	std::set<std::string> seenCreatures;  // To avoid duplicates in "All Creatures"

	// Get current category
	int index = tileset_choice->GetSelection();
	if (index == wxNOT_FOUND) return;

	const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));
	bool isAllCreaturesCategory = (tileset_choice->GetString(index) == "All Creatures");

	for (BrushVector::const_iterator iter = tsc->brushlist.begin(); iter != tsc->brushlist.end(); ++iter) {
		if (!(*iter)->isCreature()) continue;

		CreatureBrush* creatureBrush = dynamic_cast<CreatureBrush*>(*iter);
		if (!creatureBrush) continue;

		std::string baseCreatureName = (*iter)->getName();
		wxString name = wxstr(baseCreatureName).Lower();

		// For "All Creatures" category, don't add duplicates
		if (!isAllCreaturesCategory && seenCreatures.count(baseCreatureName) > 0) {
			continue;
		}

		bool match = false;

		// Check if this is a looktype search
		if (isLooktypeSearch) {
			// Match by looktype
			if (creatureBrush->getType() && creatureBrush->getType()->outfit.lookType == searchLooktype) {
				match = true;
			}
		} else {
			// Standard name search
			if (name.Find(searchLower) != wxNOT_FOUND) {
				match = true;
			}
		}

		if (match) {
			filtered_brushes.push_back(*iter);
			seenCreatures.insert(baseCreatureName);
		}
	}

	// Apply the filtered list to the appropriate view
	if (use_sprite_view) {
		// Update sprite view
		sprite_panel->Clear();
		sprite_panel->LoadCreatures(filtered_brushes);
	} else {
		// Update list view
		creature_list->Clear();

		for (BrushVector::const_iterator iter = filtered_brushes.begin(); iter != filtered_brushes.end(); ++iter) {
			CreatureBrush* cb = dynamic_cast<CreatureBrush*>(*iter);
			if (cb && cb->getType()) {
				const Outfit& outfit = cb->getType()->outfit;

				// Create name for list display
				std::string name = (*iter)->getName();

				// If this creature has custom outfit colors, add an indicator to the name
				if (outfit.lookHead > 0 || outfit.lookBody > 0 || outfit.lookLegs > 0 || outfit.lookFeet > 0) {
					name += " [outfit]";
				}

				creature_list->Append(wxstr(name), *iter);
			} else {
				// Regular creature without custom outfit
				creature_list->Append(wxstr((*iter)->getName()), *iter);
			}
		}

		// Sort the filtered list
		creature_list->Sort();
	}

	// Select first result if any
	if (!filtered_brushes.empty()) {
		SelectCreature(0);
		creature_brush_button->Enable(true);
	} else {
		creature_brush_button->Enable(false);
	}
}

void CreaturePalettePanel::OnSpriteSelected(wxCommandEvent& event) {
	Brush* old_brush = g_gui.GetCurrentBrush();

	// Update selection
	SelectCreatureBrush();
	g_gui.ActivatePalette(GetParentPalette());

	// Get the newly selected brush
	Brush* new_brush = g_gui.GetCurrentBrush();

	// If we selected the same brush, first set to nullptr then reselect
	if(old_brush && new_brush && old_brush == new_brush) {
		g_gui.SelectBrush(nullptr, TILESET_CREATURE);
	}

	// Now select the brush (either for the first time or re-selecting)
	g_gui.SelectBrush();
}

// New method to switch between list and sprite view
void CreaturePalettePanel::SetViewMode(bool use_sprites) {
	// Store original selection
	Brush* selected_brush = GetSelectedBrush();

	// Update mode flag
	use_sprite_view = use_sprites;

	// Update UI elements
	view_toggle->SetValue(use_sprites);
	large_sprites_toggle->Enable(use_sprites);  // Only enable large sprites toggle in sprite view mode
	zoom_button->Enable(use_sprites && use_large_sprites); // Only enable zoom button in large sprite mode

	if (use_sprites) {
		// Always use seamless view when sprite view is enabled
		use_seamless_view = true;

		// Switch to sprite view
		creature_list->Hide();
		sprite_panel->Hide();
		seamless_panel->Show();

		// Load creatures from the current category
		int index = tileset_choice->GetSelection();
		if (index != wxNOT_FOUND) {
			const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));

			// Determine base cell size
			int base_cell_size = use_large_sprites ? 128 : 32;
			int cell_size = base_cell_size;

			// Apply zoom factor to cell size if in large mode
			if (use_large_sprites && zoom_factor > 1) {
				cell_size = base_cell_size * zoom_factor;
			}

			// Pre-generate creature sprites at their natural sizes
			g_creature_sprites.clear(); // Clear cache to ensure new sprites are generated

			// Also reset sprite dimensions cache in the view panel
			if (seamless_panel) {
				seamless_panel->sprite_dimensions.clear();
			}

			// Pre-calculate and store natural sizes for all creatures
			for (size_t i = 0; i < tsc->brushlist.size(); ++i) {
				Brush* brush = tsc->brushlist[i];
				if (brush->isCreature()) {
					CreatureBrush* cb = static_cast<CreatureBrush*>(brush);
					if (cb && cb->getType()) {
						CreatureType* type = cb->getType();

						// Calculate natural size for this creature
						int natural_size = 32;
						if (seamless_panel) {
							natural_size = seamless_panel->GetCreatureNaturalSize(type);

							// Store in the dimensions map for later use
							for (size_t j = 0; j < seamless_panel->creatures.size(); ++j) {
								CreatureBrush* panel_cb = static_cast<CreatureBrush*>(seamless_panel->creatures[j]);
								if (panel_cb && panel_cb->getType() == type) {
									seamless_panel->sprite_dimensions[j] = natural_size;
									break;
								}
							}
						}

						// Generate sprite at its natural size
						const Outfit& outfit = type->outfit;
						if (outfit.lookHead || outfit.lookBody || outfit.lookLegs || outfit.lookFeet) {
							g_creature_sprites.getSpriteBitmap(outfit.lookType, outfit.lookHead, outfit.lookBody,
									outfit.lookLegs, outfit.lookFeet, natural_size, natural_size);
						} else {
							g_creature_sprites.getSpriteBitmap(outfit.lookType, natural_size, natural_size);
						}
					}
				}
			}

			// Update the cell size
			seamless_panel->sprite_size = cell_size;
			seamless_panel->need_full_redraw = true;
			seamless_panel->RecalculateGrid();
			seamless_panel->LoadCreatures(tsc->brushlist);
		}
	} else {
		// Switch to list view
		sprite_panel->Hide();
		seamless_panel->Hide();
		creature_list->Show();
	}

	// Update layout
	view_sizer->Layout();

	// Restore selection
	if (selected_brush) {
		SelectBrush(selected_brush);
	}
}

void CreaturePalettePanel::OnClickViewToggle(wxCommandEvent& event) {
	SetViewMode(view_toggle->GetValue());
}

void CreaturePalettePanel::OnClickViewStyleToggle(wxCommandEvent& event) {
	SetViewStyle(view_style_toggle->GetValue());
}

// ============================================================================
// CreatureSpritePanel - Panel to display creature sprites in a grid

BEGIN_EVENT_TABLE(CreatureSpritePanel, wxScrolledWindow)
EVT_PAINT(CreatureSpritePanel::OnPaint)
EVT_SIZE(CreatureSpritePanel::OnSize)
EVT_LEFT_DOWN(CreatureSpritePanel::OnMouseClick)
EVT_MOTION(CreatureSpritePanel::OnMouseMove)
END_EVENT_TABLE()

CreatureSpritePanel::CreatureSpritePanel(wxWindow* parent) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE | wxWANTS_CHARS),
	columns(0),
	sprite_size(40),
	padding(6),
	selected_index(-1),
	hover_index(-1),
	buffer(nullptr) {

	// Set background color
	SetBackgroundColour(wxColour(245, 245, 245));

	// Enable scrolling
	SetScrollRate(1, 10);
}

CreatureSpritePanel::~CreatureSpritePanel() {
	delete buffer;
}

void CreatureSpritePanel::Clear() {
	creatures.clear();
	selected_index = -1;
	hover_index = -1;
	Refresh();
}

void CreatureSpritePanel::LoadCreatures(const BrushVector& brushlist) {
	// Clear any existing creatures
	creatures.clear();
	selected_index = -1;
	hover_index = -1;

	// Copy valid creature brushes
	for (BrushVector::const_iterator iter = brushlist.begin(); iter != brushlist.end(); ++iter) {
		if ((*iter)->isCreature()) {
			creatures.push_back(*iter);
		}
	}

	// Select first creature if any
	if (!creatures.empty()) {
		selected_index = 0;
	}

	// Calculate layout and refresh
	RecalculateGrid();
	Refresh();
}

void CreatureSpritePanel::RecalculateGrid() {
	// Get the client size of the panel
	int panel_width, panel_height;
	GetClientSize(&panel_width, &panel_height);

	// Calculate number of columns based on available width
	columns = std::max(1, (panel_width - padding) / (sprite_size + padding));

	// Calculate number of rows
	int rows = creatures.empty() ? 0 : (creatures.size() + columns - 1) / columns;

	// Set virtual size for scrolling
	int virtual_height = rows * (sprite_size + padding) + padding;
	SetVirtualSize(panel_width, virtual_height);

	// Recreate buffer with new size if needed
	if (buffer) {
		delete buffer;
		buffer = nullptr;
	}

	if (panel_width > 0 && panel_height > 0) {
		buffer = new wxBitmap(panel_width, panel_height);
	}
}

void CreatureSpritePanel::OnPaint(wxPaintEvent& event) {
	// Use wxAutoBufferedPaintDC for flicker-free drawing
	wxAutoBufferedPaintDC dc(this);
	DoPrepareDC(dc);

	// Clear background
	dc.SetBackground(wxBrush(GetBackgroundColour()));
	dc.Clear();

	// Get visible region
	int x_start, y_start;
	GetViewStart(&x_start, &y_start);
	int ppuX, ppuY;
	GetScrollPixelsPerUnit(&ppuX, &ppuY);
	y_start *= ppuY;

	int width, height;
	GetClientSize(&width, &height);

	// Calculate first and last visible row
	int first_row = std::max(0, y_start / (sprite_size + padding));
	int last_row = std::min((int)((creatures.size() + columns - 1) / columns),
						   (y_start + height) / (sprite_size + padding) + 1);

	// Draw visible sprites
	for (int row = first_row; row < last_row; ++row) {
		for (int col = 0; col < columns; ++col) {
			int index = row * columns + col;
			if (index < (int)creatures.size()) {
				int x = padding + col * (sprite_size + padding);
				int y = padding + row * (sprite_size + padding);

				CreatureType* ctype = static_cast<CreatureBrush*>(creatures[index])->getType();
				DrawSprite(dc, x, y, ctype, index == selected_index);
			}
		}
	}
}

void CreatureSpritePanel::DrawSprite(wxDC& dc, int x, int y, CreatureType* ctype, bool selected) {
	if (!ctype) return;

	// Background
	if (selected) {
		dc.SetBrush(wxBrush(wxColour(0x80, 0x80, 0x80)));
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(x, y, sprite_size, sprite_size);
	}

	// Determine the target sprite size (64x64 for large mode, otherwise same as cell size)
	int actual_sprite_size = sprite_size > 64 ? 64 : sprite_size;

	// Get or create sprite bitmap at the correct size
	wxBitmap* bitmap = nullptr;
	if (ctype->outfit.lookType != 0) {
		if (ctype->outfit.lookHead || ctype->outfit.lookBody || ctype->outfit.lookLegs || ctype->outfit.lookFeet) {
			bitmap = g_creature_sprites.getSpriteBitmap(
				ctype->outfit.lookType,
				ctype->outfit.lookHead,
				ctype->outfit.lookBody,
				ctype->outfit.lookLegs,
				ctype->outfit.lookFeet,
				actual_sprite_size, actual_sprite_size);
		} else {
			bitmap = g_creature_sprites.getSpriteBitmap(ctype->outfit.lookType, actual_sprite_size, actual_sprite_size);
		}

		if (bitmap) {
			// Calculate position to center the sprite in the cell
			int offsetX = (sprite_size - actual_sprite_size) / 2;
			int offsetY = (sprite_size - actual_sprite_size) / 2;

			// Draw the sprite centered in the cell
			dc.DrawBitmap(*bitmap, x + offsetX, y + offsetY, true);
		}
	}
}

void CreatureSpritePanel::OnSize(wxSizeEvent& event) {
	RecalculateGrid();
	Refresh();
}

void CreatureSpritePanel::OnScroll(wxScrollWinEvent& event) {
	Refresh();
	event.Skip();
}

void CreatureSpritePanel::OnMouseClick(wxMouseEvent& event) {
	// Get the position in scroll coordinates
	int x, y;
	CalcUnscrolledPosition(event.GetX(), event.GetY(), &x, &y);

	// Find which sprite was clicked
	int index = GetSpriteIndexAt(x, y);

	if (index >= 0 && index < (int)creatures.size()) {
		SelectIndex(index);

		// Send selection event to parent
		wxCommandEvent selectionEvent(wxEVT_COMMAND_LISTBOX_SELECTED);
		selectionEvent.SetEventObject(this);
		GetParent()->GetEventHandler()->ProcessEvent(selectionEvent);
	}
}

void CreatureSpritePanel::OnMouseMove(wxMouseEvent& event) {
	// Update hover effect if needed
	int index = GetSpriteIndexAt(event.GetX(), event.GetY());
	if (index != hover_index) {
		// Only redraw the cells that changed, not the entire panel
		int old_hover = hover_index;
		hover_index = index;

		// If we had a previous hover, just redraw that cell
		if (old_hover >= 0 && old_hover < static_cast<int>(creatures.size())) {
			int old_row = old_hover / columns;
			int old_col = old_hover % columns;
			int x = padding + old_col * (sprite_size + padding);
			int y = padding + old_row * (sprite_size + padding);
			wxRect oldRect(x, y, sprite_size, sprite_size);
			RefreshRect(oldRect, false);
		}

		// If we have a new hover, just redraw that cell
		if (hover_index >= 0 && hover_index < static_cast<int>(creatures.size())) {
			int new_row = hover_index / columns;
			int new_col = hover_index % columns;
			int x = padding + new_col * (sprite_size + padding);
			int y = padding + new_row * (sprite_size + padding);
			wxRect newRect(x, y, sprite_size, sprite_size);
			RefreshRect(newRect, false);
		}
	}

	event.Skip();
}

int CreatureSpritePanel::GetSpriteIndexAt(int x, int y) const {
	// Calculate the column and row
	int col = (x - padding) / (sprite_size + padding);
	int row = (y - padding) / (sprite_size + padding);

	// Check if within sprite bounds
	int sprite_x = padding + col * (sprite_size + padding);
	int sprite_y = padding + row * (sprite_size + padding);

	if (x >= sprite_x && x < sprite_x + sprite_size &&
		y >= sprite_y && y < sprite_y + sprite_size) {

		// Convert to index
		int index = row * columns + col;
		if (index >= 0 && index < (int)creatures.size()) {
			return index;
		}
	}

	return -1;
}

void CreatureSpritePanel::SelectIndex(int index) {
	if (index >= 0 && index < (int)creatures.size() && index != selected_index) {
		selected_index = index;
		Refresh();

		// Ensure the selected creature is visible
		if (selected_index >= 0) {
			int row = selected_index / columns;
			int col = selected_index % columns;
			int x = padding + col * (sprite_size + padding);
			int y = padding + row * (sprite_size + padding);

			// Scroll to make the selected creature visible
			int client_width, client_height;
			GetClientSize(&client_width, &client_height);

			int x_scroll, y_scroll;
			GetViewStart(&x_scroll, &y_scroll);

			// Adjust vertical scroll if needed
			if (y < y_scroll) {
				Scroll(-1, y / 10); // / 10 because of scroll rate
			} else if (y + sprite_size > y_scroll + client_height) {
				Scroll(-1, (y + sprite_size - client_height) / 10 + 1);
			}
		}
	}
}

Brush* CreatureSpritePanel::GetSelectedBrush() const {
	if (selected_index >= 0 && selected_index < (int)creatures.size()) {
		return creatures[selected_index];
	}
	return nullptr;
}

bool CreatureSpritePanel::SelectBrush(const Brush* brush) {
	if (!brush || !brush->isCreature()) {
		return false;
	}

	for (size_t i = 0; i < creatures.size(); ++i) {
		if (creatures[i] == brush) {
			SelectIndex(i);
			return true;
		}
	}

	return false;
}

void CreatureSpritePanel::EnsureVisible(const Brush* brush) {
	if (!brush || !brush->isCreature()) {
		return;
	}

	for (size_t i = 0; i < creatures.size(); ++i) {
		if (creatures[i] == brush) {
			// Calculate row of the item
			int row = i / columns;
			int y = row * sprite_size;

			int client_height;
			GetClientSize(nullptr, &client_height);

			int x_scroll, y_scroll;
			GetViewStart(&x_scroll, &y_scroll);

			// Adjust vertical scroll if needed
			if (y < y_scroll) {
				Scroll(-1, y / 10); // / 10 because of scroll rate
			}
			break;
		}
	}
}

int CreatureSpritePanel::GetSpriteSize() const {
	return sprite_size;
}

// Implementation of CreatureSeamlessGridPanel
BEGIN_EVENT_TABLE(CreatureSeamlessGridPanel, wxScrolledWindow)
EVT_PAINT(CreatureSeamlessGridPanel::OnPaint)
EVT_SIZE(CreatureSeamlessGridPanel::OnSize)
EVT_LEFT_DOWN(CreatureSeamlessGridPanel::OnMouseClick)
EVT_MOTION(CreatureSeamlessGridPanel::OnMouseMove)
EVT_SCROLLWIN(CreatureSeamlessGridPanel::OnScroll)
EVT_TIMER(wxID_ANY, CreatureSeamlessGridPanel::OnTimer)
END_EVENT_TABLE()

CreatureSeamlessGridPanel::CreatureSeamlessGridPanel(wxWindow* parent) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxWANTS_CHARS),
	columns(1),
	sprite_size(32),
	selected_index(-1),
	hover_index(-1),
	buffer(nullptr),
	first_visible_row(0),
	last_visible_row(0),
	visible_rows_margin(10),
	total_rows(0),
	need_full_redraw(true),
	use_progressive_loading(true),
	is_large_tileset(false),
	loading_step(0),
	max_loading_steps(5),
	loading_timer(nullptr) {

	// Enable background erase to prevent flicker
	SetBackgroundStyle(wxBG_STYLE_PAINT);

	// Set background color
	SetBackgroundColour(wxColour(240, 240, 240));

	// Enable scrolling - pixel level for smoother scrolling
	SetScrollRate(1, 20);

	// Create loading timer for progressive loading
	loading_timer = new wxTimer(this);
}

CreatureSeamlessGridPanel::~CreatureSeamlessGridPanel() {
	if (loading_timer) {
		loading_timer->Stop();
		delete loading_timer;
	}
	delete buffer;
}

void CreatureSeamlessGridPanel::Clear() {
	creatures.clear();
	selected_index = -1;
	hover_index = -1;
	Refresh();
}

void CreatureSeamlessGridPanel::LoadCreatures(const BrushVector& brushlist) {
	// Clear any existing creatures
	creatures.clear();
	selected_index = -1;
	hover_index = -1;

	// Copy valid creature brushes
	for (BrushVector::const_iterator iter = brushlist.begin(); iter != brushlist.end(); ++iter) {
		if ((*iter)->isCreature()) {
			creatures.push_back(*iter);
		}
	}

	// Select first creature if any
	if (!creatures.empty()) {
		selected_index = 0;
	}

	// Store natural dimensions for each creature to use when drawing
	// This prevents constant recalculation
	sprite_dimensions.clear();
	for (size_t i = 0; i < creatures.size(); ++i) {
		CreatureBrush* cb = static_cast<CreatureBrush*>(creatures[i]);
		if (cb && cb->getType()) {
			// Determine natural size based on looktype
			int natural_size = GetCreatureNaturalSize(cb->getType());
			sprite_dimensions[i] = natural_size;
		}
	}

	// Calculate layout and refresh
	RecalculateGrid();
	Refresh();
}

void CreatureSeamlessGridPanel::StartProgressiveLoading() {
	if (!loading_timer) return;

	// Reset loading step
	loading_step = 0;

	// Set initial small margin for quick initial display
	visible_rows_margin = 3;

	// Force full redraw
	need_full_redraw = true;

	// Start timer for progressive loading
	loading_timer->Start(150); // 150ms interval for smooth loading

	// Force initial redraw to show progress
	Refresh();
}

void CreatureSeamlessGridPanel::OnTimer(wxTimerEvent& event) {
	// Progressively increase the loading step
	loading_step++;

	// Update viewable items with new margin
	UpdateViewableItems();

	// Force redraw to update progress
	Refresh();

	// Stop timer when we've reached max loading steps
	if (loading_step >= max_loading_steps) {
		loading_timer->Stop();
		visible_rows_margin = 20; // Set to higher value for regular scrolling
		need_full_redraw = true;
		Refresh();
	}
}

void CreatureSeamlessGridPanel::RecalculateGrid() {
	// Get the client size of the panel
	int panel_width, panel_height;
	GetClientSize(&panel_width, &panel_height);

	// Calculate number of columns based on available width
	columns = std::max(1, panel_width / sprite_size);

	// Calculate number of rows
	total_rows = creatures.empty() ? 0 : (creatures.size() + columns - 1) / columns;

	// Set virtual size for scrolling
	int virtual_height = total_rows * sprite_size;
	SetVirtualSize(panel_width, virtual_height);

	// Recreate buffer with new size if needed
	if (buffer) {
		delete buffer;
		buffer = nullptr;
	}

	if (panel_width > 0 && panel_height > 0) {
		buffer = new wxBitmap(panel_width, panel_height);
	}

	// Update viewable items
	UpdateViewableItems();
}

void CreatureSeamlessGridPanel::UpdateViewableItems() {
	int xStart, yStart;
	GetViewStart(&xStart, &yStart);
	int ppuX, ppuY;
	GetScrollPixelsPerUnit(&ppuX, &ppuY);
	yStart *= ppuY;

	int width, height;
	GetClientSize(&width, &height);

	// Calculate visible range with margins
	int new_first_row = std::max(0, (yStart / sprite_size) - visible_rows_margin);
	int new_last_row = std::min(total_rows - 1, ((yStart + height) / sprite_size) + visible_rows_margin);

	// Only trigger redraw if visible range changes
	if (new_first_row != first_visible_row || new_last_row != last_visible_row) {
		first_visible_row = new_first_row;
		last_visible_row = new_last_row;
		Refresh();
	}
}

void CreatureSeamlessGridPanel::OnScroll(wxScrollWinEvent& event) {
	// Handle scroll events to update visible items
	UpdateViewableItems();
	event.Skip();
}

void CreatureSeamlessGridPanel::DrawItemsToPanel(wxDC& dc) {
	if (creatures.empty()) return;

	// Get client area size
	int width, height;
	GetClientSize(&width, &height);

	// Draw loading progress for large datasets during initial load
	if (loading_step < max_loading_steps && is_large_tileset) {
		// Show loading progress
		wxString loadingMessage = wxString::Format("Loading creatures... %d%%",
			(loading_step * 100) / max_loading_steps);

		// Gray semi-transparent overlay with progress message
		dc.SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
		dc.SetTextForeground(wxColour(50, 50, 50));
		dc.DrawLabel(loadingMessage, wxRect(0, 0, width, height), wxALIGN_CENTER);
	}

	// Draw visible sprites in grid
	for (int row = first_visible_row; row <= last_visible_row; ++row) {
		for (int col = 0; col < columns; ++col) {
			int index = row * columns + col;
			if (index < static_cast<int>(creatures.size())) {
				int x = col * sprite_size;
				int y = row * sprite_size;

				CreatureBrush* cb = static_cast<CreatureBrush*>(creatures[index]);
				if (cb && cb->getType()) {
					DrawCreature(dc, x, y, cb->getType(), index == selected_index);
				}
			}
		}
	}
}

void CreatureSeamlessGridPanel::OnPaint(wxPaintEvent& event) {
	wxAutoBufferedPaintDC dc(this);
	DoPrepareDC(dc);  // For correct scrolling

	// Clear background
	dc.SetBackground(wxBrush(GetBackgroundColour()));
	dc.Clear();

	// Draw items
	DrawItemsToPanel(dc);
}

void CreatureSeamlessGridPanel::DrawCreature(wxDC& dc, int x, int y, CreatureType* ctype, bool selected) {
	if (!ctype) return;

	// Draw selection highlight
	if (selected) {
		dc.SetBrush(wxBrush(wxColour(0x80, 0x80, 0xFF, 0x80)));
		dc.SetPen(wxPen(wxColour(0x80, 0x80, 0xFF), 1));
		dc.DrawRectangle(x, y, sprite_size, sprite_size);
	}

	// For hover effect
	if (!selected && selected_index != -1 && hover_index != -1 && hover_index != selected_index) {
		int hover_col = hover_index % columns;
		int hover_row = hover_index / columns;
		int hover_x = hover_col * sprite_size;
		int hover_y = hover_row * sprite_size;

		if (hover_x == x && hover_y == y) {
			dc.SetBrush(wxBrush(wxColour(0xC0, 0xC0, 0xC0, 0x80)));
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.DrawRectangle(x, y, sprite_size, sprite_size);
		}
	}

	// Find the natural size of this creature
	int index = -1;
	for (size_t i = 0; i < creatures.size(); ++i) {
		CreatureBrush* cb = static_cast<CreatureBrush*>(creatures[i]);
		if (cb && cb->getType() == ctype) {
			index = i;
			break;
		}
	}

	// Get the natural size from the dimensions map if available
	int natural_size = 32;
	if (index >= 0 && sprite_dimensions.count(index) > 0) {
		natural_size = sprite_dimensions[index];
	} else {
		// Calculate natural size if not found in the map
		natural_size = GetCreatureNaturalSize(ctype);
	}

	// For zoomed view, determine the display size based on the cell size
	int display_size = natural_size;

	// Apply scaling based on cell size
	if (sprite_size < natural_size) {
		// If the cell is smaller than the natural size, scale down
		display_size = sprite_size;
	} else if (sprite_size > natural_size * 2) {
		// If the cell is more than twice the natural size, scale up by a factor
		int zoom_factor = sprite_size / natural_size;
		// Limit to reasonable zoom
		zoom_factor = std::min(zoom_factor, 4);
		display_size = natural_size * zoom_factor;
	}

	// Get or create sprite bitmap at the natural size
	wxBitmap* bitmap = nullptr;

	if (ctype->outfit.lookType != 0) {
		if (ctype->outfit.lookHead || ctype->outfit.lookBody || ctype->outfit.lookLegs || ctype->outfit.lookFeet) {
			bitmap = g_creature_sprites.getSpriteBitmap(
				ctype->outfit.lookType,
				ctype->outfit.lookHead,
				ctype->outfit.lookBody,
				ctype->outfit.lookLegs,
				ctype->outfit.lookFeet,
				natural_size, natural_size);
		} else {
			bitmap = g_creature_sprites.getSpriteBitmap(ctype->outfit.lookType, natural_size, natural_size);
		}

		if (bitmap) {
			// Calculate position to center the sprite in the grid cell
			int offsetX = (sprite_size - display_size) / 2;
			int offsetY = (sprite_size - display_size) / 2;

			// Ensure offsets are not negative
			offsetX = std::max(0, offsetX);
			offsetY = std::max(0, offsetY);

			// Scale the sprite if needed
			if (display_size != bitmap->GetWidth() || display_size != bitmap->GetHeight()) {
				// Create a temporary scaled bitmap
				wxImage original = bitmap->ConvertToImage();
				wxBitmap scaled(original.Scale(display_size, display_size, wxIMAGE_QUALITY_HIGH));

				// Draw the scaled bitmap
				dc.DrawBitmap(scaled, x + offsetX, y + offsetY, true);
			} else {
				// Draw the original bitmap
				dc.DrawBitmap(*bitmap, x + offsetX, y + offsetY, true);
			}
		}
	}

	// Draw name label below the sprite
	wxString name = wxString(ctype->name.c_str(), wxConvUTF8);
	if (!name.IsEmpty()) {
		// Set font size based on cell size
		int font_size = std::min(10, sprite_size / 12);
		font_size = std::max(7, font_size); // Make sure it's not too small

		wxFont font(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		dc.SetFont(font);
		dc.SetTextForeground(selected ? wxColour(50, 50, 120) : wxColour(80, 80, 80));

		// Position text at the bottom of the cell
		int text_y = y + sprite_size - font_size - 4;
		wxCoord text_width, text_height;
		dc.GetTextExtent(name, &text_width, &text_height);

		// Center the text and make sure it fits in the cell
		if (text_width > sprite_size - 4) {
			// Truncate the text
			wxString truncated_name;
			int chars_that_fit = 0;
			wxArrayInt partial_extents;
			dc.GetPartialTextExtents(name, partial_extents);

			for (size_t i = 0; i < name.Length(); ++i) {
				if (i < partial_extents.GetCount() && partial_extents[i] < sprite_size - 10) {
					chars_that_fit = i + 1;
				} else {
					break;
				}
			}

			if (chars_that_fit > 0) {
				truncated_name = name.Left(chars_that_fit) + "...";
				dc.DrawText(truncated_name, x + (sprite_size - dc.GetTextExtent(truncated_name).GetWidth()) / 2, text_y);
			}
		} else {
			// Text fits, center it
			dc.DrawText(name, x + (sprite_size - text_width) / 2, text_y);
		}
	}
}

void CreatureSeamlessGridPanel::OnSize(wxSizeEvent& event) {
	RecalculateGrid();
	event.Skip();
}

int CreatureSeamlessGridPanel::GetSpriteIndexAt(int x, int y) const {
	// Convert mouse position to logical position (accounting for scrolling)
	int logX, logY;
	CalcUnscrolledPosition(x, y, &logX, &logY);

	// Calculate row and column
	int col = logX / sprite_size;
	int row = logY / sprite_size;

	// Calculate index
	int index = row * columns + col;

	// Check if this is a valid index
	if (index >= 0 && index < static_cast<int>(creatures.size()) &&
		col >= 0 && col < columns) {
		return index;
	}

	return -1;
}

void CreatureSeamlessGridPanel::OnMouseClick(wxMouseEvent& event) {
	int index = GetSpriteIndexAt(event.GetX(), event.GetY());
	if (index != -1) {
		selected_index = index;
		Refresh();

		// Notify parent of selection
		wxCommandEvent selectionEvent(wxEVT_COMMAND_LISTBOX_SELECTED);
		wxPostEvent(GetParent(), selectionEvent);
	}

	event.Skip();
}

void CreatureSeamlessGridPanel::OnMouseMove(wxMouseEvent& event) {
	// Update hover effect if needed
	int index = GetSpriteIndexAt(event.GetX(), event.GetY());
	if (index != hover_index) {
		// Only redraw the cells that changed, not the entire panel
		int old_hover = hover_index;
		hover_index = index;

		// If we had a previous hover, just redraw that cell
		if (old_hover >= 0 && old_hover < static_cast<int>(creatures.size())) {
			int old_row = old_hover / columns;
			int old_col = old_hover % columns;
			wxRect oldRect(old_col * sprite_size, old_row * sprite_size, sprite_size, sprite_size);
			RefreshRect(oldRect, false);
		}

		// If we have a new hover, just redraw that cell
		if (hover_index >= 0 && hover_index < static_cast<int>(creatures.size())) {
			int new_row = hover_index / columns;
			int new_col = hover_index % columns;
			wxRect newRect(new_col * sprite_size, new_row * sprite_size, sprite_size, sprite_size);
			RefreshRect(newRect, false);
		}
	}

	event.Skip();
}

Brush* CreatureSeamlessGridPanel::GetSelectedBrush() const {
	if (selected_index >= 0 && selected_index < static_cast<int>(creatures.size())) {
		return creatures[selected_index];
	}
	return nullptr;
}

bool CreatureSeamlessGridPanel::SelectBrush(const Brush* whatbrush) {
	if (!whatbrush) return false;

	for (size_t i = 0; i < creatures.size(); ++i) {
		if (creatures[i] == whatbrush) {
			SelectIndex(i);
			return true;
		}
	}
	return false;
}

void CreatureSeamlessGridPanel::SelectIndex(int index) {
	if (index >= 0 && index < static_cast<int>(creatures.size())) {
		// Store the old selection
		int old_selection = selected_index;
		selected_index = index;

		// Only redraw if selection changed
		if (old_selection != selected_index) {
			Refresh();
		}

		// Ensure the selected item is visible
		EnsureVisible(creatures[index]);
	}
}

void CreatureSeamlessGridPanel::EnsureVisible(const Brush* brush) {
	for (size_t i = 0; i < creatures.size(); ++i) {
		if (creatures[i] == brush) {
			// Calculate row of the item
			int row = i / columns;
			int y = row * sprite_size;

			// Get the visible area
			int xStart, yStart;
			GetViewStart(&xStart, &yStart);
			int ppuX, ppuY;
			GetScrollPixelsPerUnit(&ppuX, &ppuY);
			yStart *= ppuY;

			int clientHeight;
			GetClientSize(nullptr, &clientHeight);

			// Scroll if necessary
			if (y < yStart) {
				Scroll(-1, y / ppuY);
			} else if (y + sprite_size > yStart + clientHeight) {
				Scroll(-1, (y - clientHeight + sprite_size) / ppuY);
			}

			// Update which items are visible after scrolling
			UpdateViewableItems();
			break;
		}
	}
}

// Update CreaturePalettePanel constructor
void CreaturePalettePanel::SetViewStyle(bool use_seamless) {
	// Store original selection
	Brush* selected_brush = GetSelectedBrush();

	// Update mode flag
	use_seamless_view = use_seamless;

	// Update UI elements
	if (use_sprite_view) {
		if (use_seamless_view) {
			// Switch to seamless grid view
			sprite_panel->Hide();
			seamless_panel->Show();

			// Load creatures from the current category
			int index = tileset_choice->GetSelection();
			if (index != wxNOT_FOUND) {
				const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));
				// Pre-generate creature sprites
				int sprite_size = seamless_panel->GetSpriteSize();
				g_creature_sprites.generateCreatureSprites(tsc->brushlist, sprite_size, sprite_size);
				seamless_panel->LoadCreatures(tsc->brushlist);
			}
		} else {
			// Switch to regular grid view
			seamless_panel->Hide();
			sprite_panel->Show();

			// Load creatures from the current category
			int index = tileset_choice->GetSelection();
			if (index != wxNOT_FOUND) {
				const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));
				// Pre-generate creature sprites
				int sprite_size = sprite_panel->GetSpriteSize();
				g_creature_sprites.generateCreatureSprites(tsc->brushlist, sprite_size, sprite_size);
				sprite_panel->LoadCreatures(tsc->brushlist);
			}
		}

		// Update layout
		view_sizer->Layout();
	}

	// Restore selection
	if (selected_brush) {
		SelectBrush(selected_brush);
	}
}

void CreaturePalettePanel::SetLargeSpriteMode(bool use_large) {
	if (use_large_sprites != use_large) {
		use_large_sprites = use_large;
		large_sprites_toggle->SetValue(use_large);

		// Update zoom button state - only enable when in large sprite mode
		zoom_button->Enable(use_large);

		// Reset zoom factor when changing sprite size mode
		if (!use_large) {
			zoom_factor = 1;
			zoom_button->SetLabel("Zoom 2x");
		}

		// Store the currently selected brush
		Brush* old_brush = GetSelectedBrush();

		// Get current tileset
		int index = tileset_choice->GetSelection();
		if (index != wxNOT_FOUND) {
			const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));

			// Determine base sprite size and cell size
			int base_sprite_size = use_large ? 64 : 32;
			int base_cell_size = use_large ? 128 : 32;

			// Apply zoom factor to cell size if needed
			int cell_size = base_cell_size;
			if (use_large && zoom_factor > 1) {
				cell_size = base_cell_size * zoom_factor;
			}

			// Force regeneration of creature sprites with new size
			g_creature_sprites.clear(); // Clear cache to ensure new sprites are generated
			g_creature_sprites.generateCreatureSprites(tsc->brushlist, base_sprite_size, base_sprite_size);

			// Update panel settings
			if (use_seamless_view) {
				// Update seamless panel - use cell_size for grid cell dimensions
				seamless_panel->sprite_size = cell_size;
				seamless_panel->need_full_redraw = true;
				seamless_panel->RecalculateGrid();
				seamless_panel->Refresh();
			} else {
				// Update regular panel - use cell_size for grid cell dimensions
				sprite_panel->sprite_size = cell_size;
				sprite_panel->RecalculateGrid();
				sprite_panel->Refresh();
			}

			// Reselect the brush that was selected before
			if (old_brush) {
				SelectBrush(old_brush);
			}
		}
	}
}

void CreaturePalettePanel::SetZoomLevel(int new_zoom_factor) {
	if (zoom_factor != new_zoom_factor) {
		zoom_factor = new_zoom_factor;

		// Update button label
		zoom_button->SetLabel(wxString::Format("Zoom %dx", new_zoom_factor));

		// Only apply zoom when in large sprite mode
		if (use_large_sprites) {
			// Store the currently selected brush
			Brush* old_brush = GetSelectedBrush();

			// Get current tileset
			int index = tileset_choice->GetSelection();
			if (index != wxNOT_FOUND) {
				const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));

				// Base cell size is 128x128 in large mode
				int base_cell_size = 128;
				int cell_size = base_cell_size * zoom_factor;

				// Pre-generate creature sprites at their natural sizes
				g_creature_sprites.clear(); // Clear cache to ensure new sprites are generated

				// Also reset sprite dimensions cache in the view panel
				if (seamless_panel) {
					seamless_panel->sprite_dimensions.clear();
				}

				// Pre-generate sprites at their natural sizes
				for (Brush* brush : tsc->brushlist) {
					if (brush->isCreature()) {
						CreatureBrush* cb = static_cast<CreatureBrush*>(brush);
						if (cb && cb->getType()) {
							CreatureType* type = cb->getType();

							// Get natural size
							int natural_size = 32;
							if (seamless_panel) {
								natural_size = seamless_panel->GetCreatureNaturalSize(type);
							}

							// Generate sprite at its natural size
							const Outfit& outfit = type->outfit;
							if (outfit.lookHead || outfit.lookBody || outfit.lookLegs || outfit.lookFeet) {
								g_creature_sprites.getSpriteBitmap(outfit.lookType, outfit.lookHead, outfit.lookBody,
										outfit.lookLegs, outfit.lookFeet, natural_size, natural_size);
							} else {
								g_creature_sprites.getSpriteBitmap(outfit.lookType, natural_size, natural_size);
							}
						}
					}
				}

				// Update panel settings
				if (use_seamless_view) {
					// Update seamless panel
					seamless_panel->sprite_size = cell_size;
					seamless_panel->need_full_redraw = true;
					seamless_panel->RecalculateGrid();
					seamless_panel->Refresh();
				} else {
					// Update regular panel
					sprite_panel->sprite_size = cell_size;
					sprite_panel->RecalculateGrid();
					sprite_panel->Refresh();
				}

				// Reselect the brush that was selected before
				if (old_brush) {
					SelectBrush(old_brush);
				}
			}
		}
	}
}

void CreaturePalettePanel::OnClickZoomButton(wxCommandEvent& event) {
	// Toggle between zoom levels (1x, 2x, 3x, back to 1x)
	int new_zoom_factor = (zoom_factor % 3) + 1;
	SetZoomLevel(new_zoom_factor);
}

void CreaturePalettePanel::OnClickLargeSpritesToggle(wxCommandEvent& event) {
	SetLargeSpriteMode(event.IsChecked());
}

int CreatureSeamlessGridPanel::GetCreatureNaturalSize(CreatureType* ctype) const {
	if (!ctype) return 32;

	// Get sprite from graphics system to check dimensions
	GameSprite* spr = g_gui.gfx.getCreatureSprite(ctype->outfit.lookType);
	if (!spr) return 32;

	// Get natural dimensions from sprite
	int natural_width = spr->width > 0 ? spr->width : 32;
	int natural_height = spr->height > 0 ? spr->height : 32;

	// Calculate natural size as the maximum of width and height
	int natural_size = std::max(natural_width, natural_height);

	// Round up to nearest standard size (32, 64, 96, 128)
	if (natural_size <= 32) {
		natural_size = 32;
	} else if (natural_size <= 64) {
		natural_size = 64;
	} else if (natural_size <= 96) {
		natural_size = 96;
	} else if (natural_size <= 128) {
		natural_size = 128;
	} else {
		natural_size = ((natural_size + 31) / 32) * 32; // Round up to nearest multiple of 32
	}

	// Fallback based on looktype for sprites without proper dimensions
	if (natural_size == 32 && ctype->outfit.lookType >= 800) {
		natural_size = 64; // Many higher looktype monsters are larger
	}

	// Fallback for very high looktypes
	if (ctype->outfit.lookType >= 1200 && natural_size < 96) {
		natural_size = 96; // Some newer monsters can be much larger
	}

	return natural_size;
}
""",
    "wxwidgets/palette_creature.h": """//////////////////////////////////////////////////////////////////////
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

#ifndef RME_TILESET_CREATURE_H_
#define RME_TILESET_CREATURE_H_

#include "palette_common.h"

// New class for seamless grid view of creature sprites with direct rendering
class CreatureSeamlessGridPanel : public wxScrolledWindow {
public:
	CreatureSeamlessGridPanel(wxWindow* parent);
	virtual ~CreatureSeamlessGridPanel();

	void Clear();
	void LoadCreatures(const BrushVector& brushlist);
	Brush* GetSelectedBrush() const;
	bool SelectBrush(const Brush* brush);
	void EnsureVisible(const Brush* brush);
	void SelectIndex(int index);

	// Getter for sprite size
	int GetSpriteSize() const { return sprite_size; }

	// Drawing handlers
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnMouseClick(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnTimer(wxTimerEvent& event);

	// Creature brushes in this panel
	BrushVector creatures;

	// Friend class declaration to allow access to protected members
	friend class CreaturePalettePanel;

protected:
	void RecalculateGrid();
	int GetSpriteIndexAt(int x, int y) const;
	int GetCreatureNaturalSize(CreatureType* ctype) const;
	void DrawCreature(wxDC& dc, int x, int y, CreatureType* ctype, bool selected = false);
	void DrawItemsToPanel(wxDC& dc);
	void UpdateViewableItems();
	void StartProgressiveLoading();

	int columns;
	int sprite_size;
	int selected_index;
	int hover_index;
	wxBitmap* buffer;
	std::map<size_t, int> sprite_dimensions; // Maps creature index to natural size

	// Viewport and loading management
	int first_visible_row;
	int last_visible_row;
	int visible_rows_margin;
	int total_rows;
	bool need_full_redraw;

	// Progressive loading properties
	bool use_progressive_loading;
	bool is_large_tileset;
	int loading_step;
	int max_loading_steps;
	wxTimer* loading_timer;
	static const int LARGE_TILESET_THRESHOLD = 200;

	DECLARE_EVENT_TABLE();
};

// Original class for grid view of creature sprites with padding
class CreatureSpritePanel : public wxScrolledWindow {
public:
	CreatureSpritePanel(wxWindow* parent);
	virtual ~CreatureSpritePanel();

	void Clear();
	void LoadCreatures(const BrushVector& brushlist);
	Brush* GetSelectedBrush() const;
	bool SelectBrush(const Brush* brush);
	void EnsureVisible(const Brush* brush);
	void SelectIndex(int index);

	// Getter for sprite size
	int GetSpriteSize() const;

	// Drawing handlers
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnMouseClick(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);

	// Creature brushes in this panel
	BrushVector creatures;

	// Friend class declaration to allow access to protected members
	friend class CreaturePalettePanel;

protected:
	void RecalculateGrid();
	int GetSpriteIndexAt(int x, int y) const;
	void DrawSprite(wxDC& dc, int x, int y, CreatureType* ctype, bool selected = false);

	int columns;
	int sprite_size;
	int padding;
	int selected_index;
	int hover_index;
	wxBitmap* buffer;

	DECLARE_EVENT_TABLE();
};

class CreaturePalettePanel : public PalettePanel {
public:
	CreaturePalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	virtual ~CreaturePalettePanel();

	PaletteType GetType() const;

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Returns the currently selected brush size
	int GetSelectedBrushSize() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush);

	// Updates the palette window to use the current brush size
	void OnUpdateBrushSize(BrushShape shape, int size);
	// Called when this page is displayed
	void OnSwitchIn();
	// Called sometimes?
	void OnUpdate();

protected:
	void SelectTileset(size_t index);
	void SelectCreature(size_t index);
	void SelectCreature(std::string name);
	// Switch between list view and sprite view modes
	void SetViewMode(bool use_sprites);
	// Set view style (regular grid vs seamless grid)
	void SetViewStyle(bool use_seamless);
	// Set large sprite mode (64x64 vs 32x32)
	void SetLargeSpriteMode(bool use_large);
	// Set zoom level for sprites
	void SetZoomLevel(int zoom_factor);

public:
	// Event handling
	void OnChangeSpawnTime(wxSpinEvent& event);
	void OnChangeSpawnSize(wxSpinEvent& event);

	void OnTilesetChange(wxCommandEvent& event);
	void OnListBoxChange(wxCommandEvent& event);
	void OnClickCreatureBrushButton(wxCommandEvent& event);
	void OnClickSpawnBrushButton(wxCommandEvent& event);
	void OnClickLoadNPCsButton(wxCommandEvent& event);
	void OnClickLoadMonstersButton(wxCommandEvent& event);
	void OnClickPurgeCreaturesButton(wxCommandEvent& event);
	void OnClickSearchButton(wxCommandEvent& event);
	void OnSearchFieldText(wxCommandEvent& event);
	void OnSearchFieldFocus(wxFocusEvent& event);
	void OnSearchFieldKillFocus(wxFocusEvent& event);
	void OnSearchFieldKeyDown(wxKeyEvent& event);
	void OnClickViewToggle(wxCommandEvent& event);
	void OnClickViewStyleToggle(wxCommandEvent& event);
	void OnClickLargeSpritesToggle(wxCommandEvent& event);
	void OnClickZoomButton(wxCommandEvent& event);
	void OnSpriteSelected(wxCommandEvent& event);

protected:
	void SelectCreatureBrush();
	void SelectSpawnBrush();
	bool LoadNPCsFromFolder(const wxString& folder);
	bool LoadMonstersFromFolder(const wxString& folder);
	bool PurgeCreaturePalettes();
	void FilterCreatures(const wxString& search_text);

	wxChoice* tileset_choice;
	SortableListBox* creature_list;
	CreatureSpritePanel* sprite_panel;
	CreatureSeamlessGridPanel* seamless_panel;
	wxToggleButton* view_toggle;
	wxToggleButton* view_style_toggle;
	wxToggleButton* large_sprites_toggle;
	wxButton* zoom_button;
	wxSizer* view_sizer;
	bool use_sprite_view;
	bool use_seamless_view;
	bool use_large_sprites;
	int zoom_factor;
	bool handling_event;

	wxTextCtrl* search_field;
	wxButton* search_button;
	wxButton* load_npcs_button;
	wxButton* load_monsters_button;
	wxButton* purge_creatures_button;

	wxSpinCtrl* creature_spawntime_spin;
	wxSpinCtrl* spawn_size_spin;
	wxToggleButton* creature_brush_button;
	wxToggleButton* spawn_brush_button;

	DECLARE_EVENT_TABLE();
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
"""
}

analyzed_files_data = []
file_descriptions = {
    "wxwidgets/palette_creature.cpp": "Implements `CreaturePalettePanel`, which includes a wxChoice for creature tilesets, a SortableListBox or CreatureSpritePanel for display, search functionality, spawn time/size controls, creature/spawn brush toggles, and buttons for loading NPC/Monster XML folders.",
    "wxwidgets/palette_creature.h": "Header for `CreaturePalettePanel` and its helper classes `CreatureSpritePanel` and `CreatureSeamlessGridPanel`.",
    "wxwidgets/old_properties_window.cpp": "(Partially relevant) Contains a constructor variant for `OldPropertiesWindow(Creature* creature)` allowing editing of creature's spawn interval and direction. Does not provide full outfit editing for map creatures directly.",
    "wxwidgets/old_properties_window.h": "Header for `OldPropertiesWindow`."
}

for file_path, content in cpp_files_content.items():
    analyzed_files_data.append({
        "file_path": file_path,
        "description": file_descriptions.get(file_path, "N/A"), # Get description or default
        "md5_hash": get_md5_hash(content),
        "content_lite": get_content_lite(content)
    })

yaml_output = {
    "wbs_item_id": "UI-06",
    "name": "Port Creature Editor & Tools",
    "description": "Recreate UI for creature palette (selection, spawn settings, search, loading) and a dialog for editing properties of creatures placed on the map.",
    "dependencies": [
        "UI-02 # Creature palette is a tab in the main palette system",
        "CORE-02 # For CreatureDatabase and CreatureType information",
        "# XML files like creatures.xml, creature_palette.xml, tilesets.xml are data sources."
    ],
    "input_files": [
        "wxwidgets/palette_creature.cpp",
        "wxwidgets/palette_creature.h",
        "wxwidgets/old_properties_window.cpp",
        "wxwidgets/old_properties_window.h"
    ],
    "analyzed_input_files": analyzed_files_data,
    "documentation_references": [
        "QComboBox: https://doc.qt.io/qt-6/qcombobox.html",
        "QLineEdit: https://doc.qt.io/qt-6/qlineedit.html",
        "QListView / QListWidget: https://doc.qt.io/qt-6/qlistwidget.html",
        "QSpinBox: https://doc.qt.io/qt-6/qspinbox.html",
        "QRadioButton / QActionGroup: https://doc.qt.io/qt-6/qradiobutton.html",
        "QFileDialog::getExistingDirectory: https://doc.qt.io/qt-6/qfiledialog.html#getExistingDirectory"
    ],
    "current_functionality_summary": """The creature tooling in wxWidgets is primarily centered around the `CreaturePalettePanel`. This panel allows:
- Selection of creature types from different tilesets/categories (loaded from XML).
- Searching for creatures by name.
- Displaying creatures in a list or a sprite grid.
- Toggling between a "Creature Brush" (for placing single instances) and a "Spawn Brush".
- When "Spawn Brush" is active, controls for "Spawn Time" and "Spawn Radius" become relevant.
- Buttons to load creatures from external NPC/Monster XML definition folders.
- A button to purge all loaded creature definitions.
Separately, the `OldPropertiesWindow` has a mode for editing an existing `Creature*` on the map, which allows changing its spawn interval (if part of a spawn) and its facing direction. Full outfit editing for individual map creatures is not a standard feature here (outfits are usually type-defined, except for podiums).""",
    "definition_of_done": [
        "The 'Creature' tab within the main palette system (`UI-02`) is implemented.",
        "This palette tab includes:",
        "  - A `QComboBox` for selecting creature categories/tilesets (e.g., 'All Creatures', 'NPCs', 'Monsters').",
        "  - A `QLineEdit` for searching/filtering creatures by name or by 'lt:<looktype_id>'.",
        "  - A `QListView` (with custom delegate for icon+name) or a custom sprite grid widget to display the filtered list of creatures.",
        "  - `QSpinBox` controls for 'Spawn Time (seconds)' and 'Spawn Radius'.",
        "  - `QRadioButton`s (or a `QActionGroup`) to toggle between 'Place Single Creature' mode and 'Place Spawn Area' mode.",
        "  - `QPushButton`s for 'Load NPCs Folder', 'Load Monsters Folder', and 'Purge All Creatures'. These interact with the core creature data management (`Creatures` class from `CORE-02`).",
        "Selecting a creature in the palette updates the application's current brush to either a `CreatureBrush` or a `SpawnBrush` based on the mode toggle.",
        "When 'Place Spawn Area' mode is active and used on the map, the tile's spawn data is configured with the selected creature, and the current Spawn Time and Radius from the palette.",
        "A new `EditMapCreatureDialog` (subclass of `QDialog`) is created.",
        "  - This dialog is used to edit properties of an existing creature instance on the map.",
        "  - It displays the creature's name (read-only).",
        "  - It provides a `QSpinBox` to edit the 'Spawn Interval' (if the creature's tile is a spawn point for this creature).",
        "  - It provides a `QComboBox` to edit the creature's 'Direction'.",
        "  - (Optional, if decided by core data structure design): If individual creatures on the map can have their outfits customized (beyond their type definition), controls for `lookType`, `head`, `body`, `legs`, `feet`, `addons` are included.",
        "The `EditMapCreatureDialog` loads data from a copy of the `Creature` object and applies changes back to the copy upon 'OK', with the caller managing undo actions."
    ],
    "boilerplate_coder_ai_prompt": """Your task is to implement the UI components for creature management in Qt6. This includes the 'Creature' tab within the main application palette and a dialog for editing properties of creature instances already placed on the map.

**Reference Files:** `wxwidgets/palette_creature.cpp`, `wxwidgets/palette_creature.h`, and relevant sections of `wxwidgets/old_properties_window.cpp` (for editing creature instances).

**1. Creature Palette Tab (within `paletteTabWidget` from `UI-02`):**
   - **Layout:**
     - `QComboBox` (`creatureTilesetCombo`): For selecting creature categories (e.g., "All Creatures", "NPCs", "Forest Monsters"). Populate from `g_materials.tilesets` where category is `TILESET_CREATURE`.
     - `QLineEdit` (`creatureSearchEdit`): For filtering creatures by name or "lt:<looktype_id>".
     - Display Area: `QListView` (`creatureListView`) with a custom model/delegate to show creature sprites and names, or a custom `CreatureSpriteGridWidget`.
     - `QSpinBox` (`spawnTimeSpin`): Label "Spawn Time (s):", range e.g., 1-3600.
     - `QSpinBox` (`spawnRadiusSpin`): Label "Spawn Radius:", range e.g., 1-15.
     - `QRadioButton` (`placeSingleCreatureRadio`): Text "Place Single Creature" (checked by default).
     - `QRadioButton` (`placeSpawnAreaRadio`): Text "Place Spawn Area". Group these two.
     - `QPushButton` (`loadNpcsButton`): Text "Load NPCs Folder...".
     - `QPushButton` (`loadMonstersButton`): Text "Load Monsters Folder...".
     - `QPushButton` (`purgeCreaturesButton`): Text "Purge All Creatures".
     - (Optional: Toggles for "Sprite View" vs "List View", "64x64 Sprites", "Zoom" if implementing advanced sprite views from `CreaturePalettePanel`).
   - **Functionality:**
     - Populate `creatureTilesetCombo`. Selecting a tileset filters the `creatureListView`.
     - `creatureSearchEdit` text changes filter `creatureListView` (case-insensitive name search, or looktype search if "lt:" prefix).
     - Selecting a creature in `creatureListView` sets it as the active creature for the brush.
     - If `placeSingleCreatureRadio` is checked, the active brush becomes a `CreatureBrush` for the selected creature.
     - If `placeSpawnAreaRadio` is checked, the active brush becomes a `SpawnBrush`. When applied to a tile, it uses the selected creature, `spawnTimeSpin->value()`, and `spawnRadiusSpin->value()` to configure the tile's spawn data.
     - "Load NPCs/Monsters" buttons: Open `QFileDialog::getExistingDirectory`. Iterate XML files, call `g_creatures.importXMLFromOT()`. Refresh palette and current list.
     - "Purge All Creatures": Confirm with `QMessageBox`. Clear `g_creatures`, clear relevant `TilesetCategory` entries in `g_materials`, refresh palettes.

**2. `EditMapCreatureDialog` (inherits `QDialog`):**
   - Constructor: `EditMapCreatureDialog(QWidget* parent, Map* map, Creature* creatureToEditCopy)`.
   - **Layout:**
     - `QLabel` for Creature Name (read-only, from `creatureToEditCopy->getName()`).
     - `QSpinBox` (`spawnIntervalEdit`): Label "Spawn Interval (s):". Load from `creatureToEditCopy->getSpawnTime()`.
     - `QComboBox` (`directionCombo`): Label "Direction:". Populate with "North", "East", "South", "West", etc. Set current from `creatureToEditCopy->getDirection()`.
     - **Outfit Section (Conditional - only if core `Creature` object on map stores its own outfit overrides):**
       - `QSpinBox` `lookTypeEdit`, `lookHeadEdit`, `lookBodyEdit`, `lookLegsEdit`, `lookFeetEdit`, `lookAddonsEdit`.
   - **Functionality:**
     - On "OK": Validate inputs. Update `creatureToEditCopy` with new spawn interval and direction (and outfit if applicable). `accept()` the dialog.

**Data Interaction:**
- Creature definitions are managed by `g_creatures` (see `CORE-02`).
- Palettes display `CreatureBrush` objects, which reference `CreatureType` from `g_creatures`.
- When placing a spawn, the `Tile` object on the map stores the creature name (or list of names), spawn radius, and spawn time.
- The `EditMapCreatureDialog` modifies an existing `Creature` object (a copy for undo purposes).
"""
}

output_filename = "enhanced_wbs_yaml_files/UI-06.yaml"
output_dir = os.path.dirname(output_filename)

if not os.path.exists(output_dir):
    os.makedirs(output_dir)

with open(output_filename, 'w') as f:
    yaml.dump(yaml_output, f, sort_keys=False, width=1000, allow_unicode=True)

print(f"Successfully generated {output_filename}")
