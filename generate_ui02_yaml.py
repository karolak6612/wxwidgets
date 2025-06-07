import yaml
import hashlib

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

file_contents = {
    "wxwidgets/main_toolbar.cpp": """\
//////////////////////////////////////////////////////////////////////
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
#include "main_toolbar.h"
#include "gui.h"
#include "editor.h"
#include "settings.h"
#include "brush.h"
#include "pngfiles.h"
#include "artprovider.h"
#include <wx/artprov.h>
#include <wx/mstream.h>
#include "hotkey_manager.h"
#include <pugixml.hpp>

const wxString MainToolBar::STANDARD_BAR_NAME = "standard_toolbar";
const wxString MainToolBar::BRUSHES_BAR_NAME = "brushes_toolbar";
const wxString MainToolBar::POSITION_BAR_NAME = "position_toolbar";
const wxString MainToolBar::SIZES_BAR_NAME = "sizes_toolbar";

#define loadPNGFile(name) _wxGetBitmapFromMemory(name, sizeof(name))
inline wxBitmap* _wxGetBitmapFromMemory(const unsigned char* data, int length) {
	wxMemoryInputStream is(data, length);
	wxImage img(is, "image/png");
	if (!img.IsOk()) {
		return nullptr;
	}
	return newd wxBitmap(img, -1);
}

MainToolBar::MainToolBar(wxWindow* parent, wxAuiManager* manager) {
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));
	wxBitmap new_bitmap = wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR, icon_size);
	wxBitmap open_bitmap = wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, icon_size);
	wxBitmap save_bitmap = wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, icon_size);
	wxBitmap saveas_bitmap = wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_TOOLBAR, icon_size);
	wxBitmap undo_bitmap = wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR, icon_size);
	wxBitmap redo_bitmap = wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR, icon_size);
	wxBitmap cut_bitmap = wxArtProvider::GetBitmap(wxART_CUT, wxART_TOOLBAR, icon_size);
	wxBitmap copy_bitmap = wxArtProvider::GetBitmap(wxART_COPY, wxART_TOOLBAR, icon_size);
	wxBitmap paste_bitmap = wxArtProvider::GetBitmap(wxART_PASTE, wxART_TOOLBAR, icon_size);

	standard_toolbar = newd wxAuiToolBar(parent, TOOLBAR_STANDARD, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	standard_toolbar->SetToolBitmapSize(icon_size);
	standard_toolbar->AddTool(wxID_NEW, wxEmptyString, new_bitmap, wxNullBitmap, wxITEM_NORMAL, "New Map", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_OPEN, wxEmptyString, open_bitmap, wxNullBitmap, wxITEM_NORMAL, "Open Map", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_SAVE, wxEmptyString, save_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_SAVEAS, wxEmptyString, saveas_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map As...", wxEmptyString, NULL);
	standard_toolbar->AddSeparator();
	standard_toolbar->AddTool(wxID_UNDO, wxEmptyString, undo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Undo", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_REDO, wxEmptyString, redo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Redo", wxEmptyString, NULL);
	standard_toolbar->AddSeparator();
	standard_toolbar->AddTool(wxID_CUT, wxEmptyString, cut_bitmap, wxNullBitmap, wxITEM_NORMAL, "Cut", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_COPY, wxEmptyString, copy_bitmap, wxNullBitmap, wxITEM_NORMAL, "Copy", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_PASTE, wxEmptyString, paste_bitmap, wxNullBitmap, wxITEM_NORMAL, "Paste", wxEmptyString, NULL);
	standard_toolbar->Realize();

	wxBitmap* border_bitmap = loadPNGFile(optional_border_small_png);
	wxBitmap* eraser_bitmap = loadPNGFile(eraser_small_png);
	wxBitmap pz_bitmap = wxArtProvider::GetBitmap(ART_PZ_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap nopvp_bitmap = wxArtProvider::GetBitmap(ART_NOPVP_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap nologout_bitmap = wxArtProvider::GetBitmap(ART_NOLOOUT_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap pvp_bitmap = wxArtProvider::GetBitmap(ART_PVP_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap normal_bitmap = wxArtProvider::GetBitmap(ART_DOOR_NORMAL_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap locked_bitmap = wxArtProvider::GetBitmap(ART_DOOR_LOCKED_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap magic_bitmap = wxArtProvider::GetBitmap(ART_DOOR_MAGIC_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap quest_bitmap = wxArtProvider::GetBitmap(ART_DOOR_QUEST_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap normal_alt_bitmap = wxArtProvider::GetBitmap(ART_DOOR_NORMAL_ALT_SMALL, wxART_TOOLBAR, icon_size);
	wxBitmap archway_bitmap = wxArtProvider::GetBitmap(ART_DOOR_ARCHWAY_SMALL, wxART_TOOLBAR, icon_size);

	wxBitmap* hatch_bitmap = loadPNGFile(window_hatch_small_png);
	wxBitmap* window_bitmap = loadPNGFile(window_normal_small_png);

	brushes_toolbar = newd wxAuiToolBar(parent, TOOLBAR_BRUSHES, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	brushes_toolbar->SetToolBitmapSize(icon_size);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, wxEmptyString, *border_bitmap, wxNullBitmap, wxITEM_CHECK, "Border", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_ERASER, wxEmptyString, *eraser_bitmap, wxNullBitmap, wxITEM_CHECK, "Eraser", wxEmptyString, NULL);
	brushes_toolbar->AddSeparator();
	brushes_toolbar->AddTool(PALETTE_TERRAIN_PZ_TOOL, wxEmptyString, pz_bitmap, wxNullBitmap, wxITEM_CHECK, "Protected Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_NOPVP_TOOL, wxEmptyString, nopvp_bitmap, wxNullBitmap, wxITEM_CHECK, "No PvP Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, wxEmptyString, nologout_bitmap, wxNullBitmap, wxITEM_CHECK, "No Logout Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_PVPZONE_TOOL, wxEmptyString, pvp_bitmap, wxNullBitmap, wxITEM_CHECK, "PvP Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_ZONE_BRUSH, wxEmptyString, normal_bitmap, wxNullBitmap, wxITEM_CHECK, "Zone Brush", wxEmptyString, NULL);
	brushes_toolbar->AddSeparator();

	brushes_toolbar->AddTool(PALETTE_TERRAIN_NORMAL_DOOR, wxEmptyString, normal_bitmap, wxNullBitmap, wxITEM_CHECK, "Normal Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_LOCKED_DOOR, wxEmptyString, locked_bitmap, wxNullBitmap, wxITEM_CHECK, "Locked Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_MAGIC_DOOR, wxEmptyString, magic_bitmap, wxNullBitmap, wxITEM_CHECK, "Magic Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_QUEST_DOOR, wxEmptyString, quest_bitmap, wxNullBitmap, wxITEM_CHECK, "Quest Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_NORMAL_ALT_DOOR, wxEmptyString, normal_alt_bitmap, wxNullBitmap, wxITEM_CHECK, "Normal Door (alt)", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_ARCHWAY_DOOR, wxEmptyString, archway_bitmap, wxNullBitmap, wxITEM_CHECK, "Archway", wxEmptyString, NULL);
	brushes_toolbar->AddSeparator();
	brushes_toolbar->AddTool(PALETTE_TERRAIN_HATCH_DOOR, wxEmptyString, *hatch_bitmap, wxNullBitmap, wxITEM_CHECK, "Hatch Window", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_WINDOW_DOOR, wxEmptyString, *window_bitmap, wxNullBitmap, wxITEM_CHECK, "Window", wxEmptyString, NULL);
	brushes_toolbar->Realize();

	wxBitmap go_bitmap = wxArtProvider::GetBitmap(ART_POSITION_GO, wxART_TOOLBAR, icon_size);

	position_toolbar = newd wxAuiToolBar(parent, TOOLBAR_POSITION, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_HORZ_TEXT);
	position_toolbar->SetToolBitmapSize(icon_size);
	x_control = newd NumberTextCtrl(position_toolbar, wxID_ANY, 0, 0, MAP_MAX_WIDTH, wxTE_PROCESS_ENTER, "X", wxDefaultPosition, FROM_DIP(parent, wxSize(60, 20)));
	x_control->SetToolTip("X Coordinate");
	y_control = newd NumberTextCtrl(position_toolbar, wxID_ANY, 0, 0, MAP_MAX_HEIGHT, wxTE_PROCESS_ENTER, "Y", wxDefaultPosition, FROM_DIP(parent, wxSize(60, 20)));
	y_control->SetToolTip("Y Coordinate");
	z_control = newd NumberTextCtrl(position_toolbar, wxID_ANY, 0, 0, MAP_MAX_LAYER, wxTE_PROCESS_ENTER, "Z", wxDefaultPosition, FROM_DIP(parent, wxSize(35, 20)));
	z_control->SetToolTip("Z Coordinate");
	go_button = newd wxButton(position_toolbar, TOOLBAR_POSITION_GO, wxEmptyString, wxDefaultPosition, parent->FromDIP(wxSize(22, 20)));
	go_button->SetBitmap(go_bitmap);
	go_button->SetToolTip("Go To Position");
	position_toolbar->AddControl(x_control);
	position_toolbar->AddControl(y_control);
	position_toolbar->AddControl(z_control);
	position_toolbar->AddControl(go_button);
	position_toolbar->Realize();

	wxBitmap circular_bitmap = wxArtProvider::GetBitmap(ART_CIRCULAR, wxART_TOOLBAR, icon_size);
	wxBitmap rectangular_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR, wxART_TOOLBAR, icon_size);
	wxBitmap size1_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_1, wxART_TOOLBAR, icon_size);
	wxBitmap size2_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_2, wxART_TOOLBAR, icon_size);
	wxBitmap size3_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_3, wxART_TOOLBAR, icon_size);
	wxBitmap size4_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_4, wxART_TOOLBAR, icon_size);
	wxBitmap size5_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_5, wxART_TOOLBAR, icon_size);
	wxBitmap size6_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_6, wxART_TOOLBAR, icon_size);
	wxBitmap size7_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_7, wxART_TOOLBAR, icon_size);

	sizes_toolbar = newd wxAuiToolBar(parent, TOOLBAR_SIZES, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	sizes_toolbar->SetToolBitmapSize(icon_size);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_RECTANGULAR, wxEmptyString, rectangular_bitmap, wxNullBitmap, wxITEM_CHECK, "Rectangular Brush", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_CIRCULAR, wxEmptyString, circular_bitmap, wxNullBitmap, wxITEM_CHECK, "Circular Brush", wxEmptyString, NULL);
	sizes_toolbar->AddSeparator();
	sizes_toolbar->AddTool(TOOLBAR_SIZES_1, wxEmptyString, size1_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 1", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_2, wxEmptyString, size2_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 2", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_3, wxEmptyString, size3_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 3", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_4, wxEmptyString, size4_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 4", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_5, wxEmptyString, size5_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 5", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_6, wxEmptyString, size6_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 6", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_7, wxEmptyString, size7_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 7", wxEmptyString, NULL);
	sizes_toolbar->Realize();
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_RECTANGULAR, true);
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_1, true);

	manager->AddPane(standard_toolbar, wxAuiPaneInfo().Name(STANDARD_BAR_NAME).ToolbarPane().Top().Row(1).Position(1).Floatable(false));
	manager->AddPane(brushes_toolbar, wxAuiPaneInfo().Name(BRUSHES_BAR_NAME).ToolbarPane().Top().Row(1).Position(2).Floatable(false));
	manager->AddPane(position_toolbar, wxAuiPaneInfo().Name(POSITION_BAR_NAME).ToolbarPane().Top().Row(1).Position(4).Floatable(false));
	manager->AddPane(sizes_toolbar, wxAuiPaneInfo().Name(SIZES_BAR_NAME).ToolbarPane().Top().Row(1).Position(3).Floatable(false));

	standard_toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnStandardButtonClick, this);
	brushes_toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnBrushesButtonClick, this);
	x_control->Bind(wxEVT_TEXT_PASTE, &MainToolBar::OnPastePositionText, this);
	x_control->Bind(wxEVT_KEY_UP, &MainToolBar::OnPositionKeyUp, this);
	y_control->Bind(wxEVT_TEXT_PASTE, &MainToolBar::OnPastePositionText, this);
	y_control->Bind(wxEVT_KEY_UP, &MainToolBar::OnPositionKeyUp, this);
	z_control->Bind(wxEVT_TEXT_PASTE, &MainToolBar::OnPastePositionText, this);
	z_control->Bind(wxEVT_KEY_UP, &MainToolBar::OnPositionKeyUp, this);
	go_button->Bind(wxEVT_BUTTON, &MainToolBar::OnPositionButtonClick, this);
	sizes_toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnSizesButtonClick, this);

	HideAll();
}
""",
    "wxwidgets/main_toolbar.h": """\
//////////////////////////////////////////////////////////////////////
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

#ifndef RME_MAINTOOLBAR_H_
#define RME_MAINTOOLBAR_H_

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibar.h>

#include "gui_ids.h"
#include "numbertextctrl.h"

class MainToolBar : public wxEvtHandler {
public:
	MainToolBar(wxWindow* parent, wxAuiManager* manager);
	~MainToolBar();

	wxAuiPaneInfo& GetPane(ToolBarID id);
	void UpdateButtons();
	void UpdateBrushButtons();
	void UpdateBrushSize(BrushShape shape, int size);
	void Show(ToolBarID id, bool show);
	void HideAll(bool update = true);
	void LoadPerspective();
	void SavePerspective();
	void RegisterHotkeys();

	void OnStandardButtonClick(wxCommandEvent& event);
	void OnBrushesButtonClick(wxCommandEvent& event);
	void OnPositionButtonClick(wxCommandEvent& event);
	void OnPositionKeyUp(wxKeyEvent& event);
	void OnPastePositionText(wxClipboardTextEvent& event);
	void OnSizesButtonClick(wxCommandEvent& event);

private:
	static const wxString STANDARD_BAR_NAME;
	static const wxString BRUSHES_BAR_NAME;
	static const wxString POSITION_BAR_NAME;
	static const wxString SIZES_BAR_NAME;

	wxAuiToolBar* standard_toolbar;
	wxAuiToolBar* brushes_toolbar;
	wxAuiToolBar* position_toolbar;
	NumberTextCtrl* x_control;
	NumberTextCtrl* y_control;
	NumberTextCtrl* z_control;
	wxButton* go_button;
	wxAuiToolBar* sizes_toolbar;
};

#endif // RME_MAINTOOLBAR_H_
""",
    "wxwidgets/palette_window.cpp": """\
//////////////////////////////////////////////////////////////////////
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
#include "gui.h"
#include "brush.h"
#include "map_display.h"

#include "palette_window.h"
#include "palette_brushlist.h"
#include "palette_house.h"
#include "palette_creature.h"
#include "palette_waypoints.h"

#include "house_brush.h"
#include "map.h"

// ============================================================================
// Palette window

BEGIN_EVENT_TABLE(PaletteWindow, wxPanel)
EVT_CHOICEBOOK_PAGE_CHANGING(PALETTE_CHOICEBOOK, PaletteWindow::OnSwitchingPage)
EVT_CHOICEBOOK_PAGE_CHANGED(PALETTE_CHOICEBOOK, PaletteWindow::OnPageChanged)
EVT_CLOSE(PaletteWindow::OnClose)

EVT_KEY_DOWN(PaletteWindow::OnKey)
EVT_TEXT(wxID_ANY, PaletteWindow::OnActionIDChange)
EVT_CHECKBOX(wxID_ANY, PaletteWindow::OnActionIDToggle)
END_EVENT_TABLE()

PaletteWindow::PaletteWindow(wxWindow* parent, const TilesetContainer& tilesets) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(230, 250)),
	choicebook(nullptr),
	terrain_palette(nullptr),
	doodad_palette(nullptr),
	item_palette(nullptr),
	collection_palette(nullptr),
	creature_palette(nullptr),
	house_palette(nullptr),
	waypoint_palette(nullptr),
	raw_palette(nullptr),
	action_id_input(nullptr),
	action_id_checkbox(nullptr),
	action_id(0),
	action_id_enabled(false) {

	// Allow resizing but maintain minimum size
	SetMinSize(wxSize(225, 250));

	// Create main sizer
	wxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	// Create action ID controls
	wxSizer* action_id_sizer = new wxBoxSizer(wxHORIZONTAL);
	action_id_input = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(60, -1));
	action_id_input->SetToolTip("Enter action ID (0-65535)");
	action_id_checkbox = new wxCheckBox(this, wxID_ANY, "Enable Action ID");
	action_id_checkbox->SetToolTip("When enabled, placed items will have this action ID");

	action_id_sizer->Add(action_id_input, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	action_id_sizer->Add(action_id_checkbox, 0, wxALIGN_CENTER_VERTICAL);

	main_sizer->Add(action_id_sizer, 0, wxEXPAND | wxALL, 5);

	// Create choicebook with EXPAND flag to fill available space
	choicebook = new wxChoicebook(this, PALETTE_CHOICEBOOK, wxDefaultPosition, wxDefaultSize);

	terrain_palette = static_cast<BrushPalettePanel*>(CreateTerrainPalette(choicebook, tilesets));
	choicebook->AddPage(terrain_palette, terrain_palette->GetName());

	doodad_palette = static_cast<BrushPalettePanel*>(CreateDoodadPalette(choicebook, tilesets));
	choicebook->AddPage(doodad_palette, doodad_palette->GetName());

	collection_palette = static_cast<BrushPalettePanel*>(CreateCollectionPalette(choicebook, tilesets));
	choicebook->AddPage(collection_palette, collection_palette->GetName());

	item_palette = static_cast<BrushPalettePanel*>(CreateItemPalette(choicebook, tilesets));
	choicebook->AddPage(item_palette, item_palette->GetName());

	house_palette = static_cast<HousePalettePanel*>(CreateHousePalette(choicebook, tilesets));
	choicebook->AddPage(house_palette, house_palette->GetName());

	waypoint_palette = static_cast<WaypointPalettePanel*>(CreateWaypointPalette(choicebook, tilesets));
	choicebook->AddPage(waypoint_palette, waypoint_palette->GetName());

	creature_palette = static_cast<CreaturePalettePanel*>(CreateCreaturePalette(choicebook, tilesets));
	choicebook->AddPage(creature_palette, creature_palette->GetName());

	raw_palette = static_cast<BrushPalettePanel*>(CreateRAWPalette(choicebook, tilesets));
	choicebook->AddPage(raw_palette, raw_palette->GetName());

	// Add choicebook to main sizer
	main_sizer->Add(choicebook, 1, wxEXPAND | wxALL, 2);
	SetSizer(main_sizer);

	// Load first page
	LoadCurrentContents();
}

PaletteWindow::~PaletteWindow() {
	// Clean up all brushes and caches in each palette panel
	if (choicebook) {
		// Ensure each panel type is properly cleaned up
		if (terrain_palette) {
			terrain_palette->DestroyAllCaches();
		}
		if (doodad_palette) {
			doodad_palette->DestroyAllCaches();
		}
		if (item_palette) {
			item_palette->DestroyAllCaches();
		}
		if (collection_palette) {
			collection_palette->DestroyAllCaches();
		}
		if (raw_palette) {
			raw_palette->DestroyAllCaches();
		}

		// Other palette types may need specific cleanup
		for (size_t iz = 0; iz < choicebook->GetPageCount(); ++iz) {
			PalettePanel* panel = dynamic_cast<PalettePanel*>(choicebook->GetPage(iz));
			if (panel) {
				panel->InvalidateContents();
			}
		}
	}
}

PalettePanel* PaletteWindow::CreateTerrainPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_TERRAIN);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_TERRAIN_STYLE)));

	BrushToolPanel* tool_panel = newd BrushToolPanel(panel);
	tool_panel->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_TERRAIN_TOOLBAR));
	panel->AddToolPanel(tool_panel);

	BrushSizePanel* size_panel = newd BrushSizePanel(panel);
	size_panel->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_TERRAIN_TOOLBAR));
	panel->AddToolPanel(size_panel);

	ZoneBrushPanel* zone_brush_panel = newd ZoneBrushPanel(panel);
	zone_brush_panel->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_TERRAIN_TOOLBAR));
	panel->AddToolPanel(zone_brush_panel);

	return panel;
}

PalettePanel* PaletteWindow::CreateCollectionPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_COLLECTION);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_COLLECTION_STYLE)));

	// terrain palette tools
	BrushToolPanel* tool_panel = newd BrushToolPanel(panel);
	tool_panel->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_COLLECTION_TOOLBAR));
	panel->AddToolPanel(tool_panel);

	// brush thickness panel
	panel->AddToolPanel(newd BrushThicknessPanel(panel));

	// brush size tools
	BrushSizePanel* size_panel = newd BrushSizePanel(panel);
	size_panel->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_COLLECTION_TOOLBAR));
	panel->AddToolPanel(size_panel);

	return panel;
}

PalettePanel* PaletteWindow::CreateDoodadPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_DOODAD);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_DOODAD_STYLE)));

	panel->AddToolPanel(newd BrushThicknessPanel(panel));

	BrushSizePanel* size_panel = newd BrushSizePanel(panel);
	size_panel->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_DOODAD_SIZEBAR));
	panel->AddToolPanel(size_panel);

	return panel;
}

PalettePanel* PaletteWindow::CreateItemPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_ITEM);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_ITEM_STYLE)));

	BrushSizePanel* size_panel = newd BrushSizePanel(panel);
	size_panel->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_ITEM_SIZEBAR));
	panel->AddToolPanel(size_panel);
	return panel;
}

PalettePanel* PaletteWindow::CreateHousePalette(wxWindow* parent, const TilesetContainer& tilesets) {
	HousePalettePanel* panel = newd HousePalettePanel(parent);

	BrushSizePanel* size_panel = newd BrushSizePanel(panel);
	size_panel->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_HOUSE_SIZEBAR));
	panel->AddToolPanel(size_panel);
	return panel;
}

PalettePanel* PaletteWindow::CreateWaypointPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	WaypointPalettePanel* panel = newd WaypointPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteWindow::CreateCreaturePalette(wxWindow* parent, const TilesetContainer& tilesets) {
	CreaturePalettePanel* panel = newd CreaturePalettePanel(parent);
	return panel;
}

PalettePanel* PaletteWindow::CreateRAWPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_RAW);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_RAW_STYLE)));

	BrushSizePanel* size_panel = newd BrushSizePanel(panel);
	size_panel->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_RAW_SIZEBAR));
	panel->AddToolPanel(size_panel);

	return panel;
}

void PaletteWindow::ReloadSettings(Map* map) {
	if (terrain_palette) {
		terrain_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_TERRAIN_STYLE)));
		terrain_palette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_TERRAIN_TOOLBAR));
	}
	if (doodad_palette) {
		doodad_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_DOODAD_STYLE)));
		doodad_palette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_DOODAD_SIZEBAR));
	}
	if (house_palette) {
		house_palette->SetMap(map);
		house_palette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_HOUSE_SIZEBAR));
	}
	if (waypoint_palette) {
		waypoint_palette->SetMap(map);
	}
	if (item_palette) {
		item_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_ITEM_STYLE)));
		item_palette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_ITEM_SIZEBAR));
	}
	if (collection_palette) {
		collection_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_COLLECTION_STYLE)));
		collection_palette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_COLLECTION_TOOLBAR));
	}
	if (raw_palette) {
		raw_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_RAW_STYLE)));
		raw_palette->SetToolbarIconSize(g_settings.getBoolean(Config::USE_LARGE_RAW_SIZEBAR));
	}
	InvalidateContents();
}
""",
    "wxwidgets/palette_window.h": """\
//////////////////////////////////////////////////////////////////////
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

#ifndef RME_PALETTE_H_
#define RME_PALETTE_H_

#include "palette_common.h"

class BrushPalettePanel;
class CreaturePalettePanel;
class HousePalettePanel;
class WaypointPalettePanel;

class PaletteWindow : public wxPanel {
public:
	PaletteWindow(wxWindow* parent, const TilesetContainer& tilesets);
	~PaletteWindow();

	// Interface
	// Reloads layout g_settings from g_settings (and using map)
	void ReloadSettings(Map* from);
	// Flushes all pages and forces them to be reloaded from the palette data again
	void InvalidateContents();
	// (Re)Loads all currently displayed data, called from InvalidateContents implicitly
	void LoadCurrentContents();
	// Goes to the selected page and selects any brush there
	void SelectPage(PaletteType palette);
	// The currently selected brush in this palette
	Brush* GetSelectedBrush() const;
	// The currently selected brush size in this palette
	int GetSelectedBrushSize() const;
	// The currently selected page (terrain, doodad...)
	PaletteType GetSelectedPage() const;

	// Get the current action ID value
	uint16_t GetActionID() const { return action_id; }
	// Check if action ID is enabled
	bool IsActionIDEnabled() const { return action_id_enabled; }

	// Custom Event handlers (something has changed?)
	// Finds the brush pointed to by whatbrush and selects it as the current brush (also changes page)
	// Returns if the brush was found in this palette
	virtual bool OnSelectBrush(const Brush* whatbrush, PaletteType primary = TILESET_UNKNOWN);
	// Updates the palette window to use the current brush size
	virtual void OnUpdateBrushSize(BrushShape shape, int size);
	// Updates the content of the palette (eg. houses, creatures)
	virtual void OnUpdate(Map* map);

	// wxWidgets Event Handlers
	void OnSwitchingPage(wxChoicebookEvent& event);
	void OnPageChanged(wxChoicebookEvent& event);
	// Forward key events to the parent window (The Map Window)
	void OnKey(wxKeyEvent& event);
	void OnClose(wxCloseEvent&);
	void OnActionIDChange(wxCommandEvent& event);
	void OnActionIDToggle(wxCommandEvent& event);

protected:
	static PalettePanel* CreateTerrainPalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateDoodadPalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateItemPalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateCollectionPalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateCreaturePalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateHousePalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateWaypointPalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateRAWPalette(wxWindow* parent, const TilesetContainer& tilesets);

	wxChoicebook* choicebook;

	BrushPalettePanel* terrain_palette;
	BrushPalettePanel* doodad_palette;
	BrushPalettePanel* item_palette;
	BrushPalettePanel* collection_palette;
	CreaturePalettePanel* creature_palette;
	HousePalettePanel* house_palette;
	WaypointPalettePanel* waypoint_palette;
	BrushPalettePanel* raw_palette;

	// Action ID controls
	wxTextCtrl* action_id_input;
	wxCheckBox* action_id_checkbox;
	uint16_t action_id;
	bool action_id_enabled;

	DECLARE_EVENT_TABLE()
};

#endif
""",
    "wxwidgets/palette_brushlist.cpp": """\
//////////////////////////////////////////////////////////////////////
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

#include <set>
#include <algorithm> // Add this for std::min
#include "palette_brushlist.h"
#include "gui.h"
#include "brush.h"
#include "raw_brush.h"
#include "add_tileset_window.h"
#include "add_item_window.h"
#include "materials.h"
#include "border_editor_window.h"

// Define BrushPanelState class at the top of the file
class BrushPanelState {
public:
	BrushBoxInterface* grid_view;
	BrushBoxInterface* list_view;
	wxBoxSizer* zoom_sizer;
	wxStaticText* zoom_value_label;
	bool grid_view_shown;

	BrushPanelState() : grid_view(nullptr), list_view(nullptr), zoom_sizer(nullptr),
					   zoom_value_label(nullptr), grid_view_shown(false) {}

	~BrushPanelState() {}
};

// Keep a static map of constructed brush panels by tileset
std::map<const TilesetCategory*, BrushPanelState> g_brush_panel_cache;

// ============================================================================
// Brush Palette Panel
// A common class for terrain/doodad/item/raw palette

BEGIN_EVENT_TABLE(BrushPalettePanel, PalettePanel)
	EVT_BUTTON(wxID_ADD, BrushPalettePanel::OnClickAddItemTileset)
	EVT_BUTTON(wxID_NEW, BrushPalettePanel::OnClickAddTileset)
	EVT_BUTTON(BUTTON_QUICK_ADD_ITEM, BrushPalettePanel::OnClickQuickAddItemTileset)
	EVT_BUTTON(BUTTON_ADD_BORDER, BrushPalettePanel::OnClickCreateBorder)
	EVT_CHOICEBOOK_PAGE_CHANGING(wxID_ANY, BrushPalettePanel::OnSwitchingPage)
	EVT_CHOICEBOOK_PAGE_CHANGED(wxID_ANY, BrushPalettePanel::OnPageChanged)
END_EVENT_TABLE()

BrushPalettePanel::BrushPalettePanel(wxWindow* parent, const TilesetContainer& tilesets, TilesetCategoryType category, wxWindowID id) :
	PalettePanel(parent, id),
	palette_type(category),
	choicebook(nullptr),
	size_panel(nullptr),
	quick_add_button(nullptr),
	last_tileset_name("") {
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	// Create the tileset panel
	wxSizer* ts_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Tileset");
	wxChoicebook* tmp_choicebook = newd wxChoicebook(this, wxID_ANY, wxDefaultPosition, wxSize(180, 250));
	ts_sizer->Add(tmp_choicebook, 1, wxEXPAND);
	topsizer->Add(ts_sizer, 1, wxEXPAND);

	if (g_settings.getBoolean(Config::SHOW_TILESET_EDITOR)) {
		// Create a vertical sizer to hold the two rows of buttons
		wxSizer* buttonSizer = newd wxBoxSizer(wxVERTICAL);

		// First row - Add Tileset and Add Item
		wxSizer* firstRowSizer = newd wxBoxSizer(wxHORIZONTAL);
		wxButton* buttonAddTileset = newd wxButton(this, wxID_NEW, "Add new Tileset");
		firstRowSizer->Add(buttonAddTileset, wxSizerFlags(1).Expand());

		wxButton* buttonAddItemToTileset = newd wxButton(this, wxID_ADD, "Add new Item");
		firstRowSizer->Add(buttonAddItemToTileset, wxSizerFlags(1).Expand());

		// Add first row to the button sizer
		buttonSizer->Add(firstRowSizer, wxSizerFlags(0).Expand());

		// Add a small space between rows
		buttonSizer->AddSpacer(5);

		// Second row - Quick Add Item and Create Border
		wxSizer* secondRowSizer = newd wxBoxSizer(wxHORIZONTAL);

		// Create the Quick Add Item button, initially disabled
		quick_add_button = newd wxButton(this, BUTTON_QUICK_ADD_ITEM, "Quick Add Item");
		quick_add_button->SetToolTip("Quickly add the currently selected brush to the last used tileset");
		quick_add_button->Enable(false); // Disabled until a tileset is added
		secondRowSizer->Add(quick_add_button, wxSizerFlags(1).Expand());

		// Create Border button
		wxButton* buttonCreateBorder = newd wxButton(this, BUTTON_ADD_BORDER, "Create Border");
		buttonCreateBorder->SetToolTip("Open the Border Editor to create or edit auto-borders");
		secondRowSizer->Add(buttonCreateBorder, wxSizerFlags(1).Expand());

		// Add second row to the button sizer
		buttonSizer->Add(secondRowSizer, wxSizerFlags(0).Expand());

		// Add the complete button sizer to the top sizer
		topsizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
	}

	for (TilesetContainer::const_iterator iter = tilesets.begin(); iter != tilesets.end(); ++iter) {
		const TilesetCategory* tcg = iter->second->getCategory(category);
		if (tcg && tcg->size() > 0) {
			BrushPanel* panel = newd BrushPanel(tmp_choicebook);
			panel->AssignTileset(tcg);
			tmp_choicebook->AddPage(panel, wxstr(iter->second->name));
		}
	}

	SetSizerAndFit(topsizer);

	choicebook = tmp_choicebook;
}

BrushPalettePanel::~BrushPalettePanel() {
	// Ensure all caches are destroyed
	DestroyAllCaches();
}

void BrushPalettePanel::DestroyAllCaches() {
	// Force cleanup of all panels to prevent memory leaks when application exits
	if (choicebook) {
		for (size_t iz = 0; iz < choicebook->GetPageCount(); ++iz) {
			BrushPanel* panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(iz));
			if (panel) {
				panel->InvalidateContents();
			}
		}
	}

	// Clear remembered brushes
	remembered_brushes.clear();
}

void BrushPalettePanel::InvalidateContents() {
	for (size_t iz = 0; iz < choicebook->GetPageCount(); ++iz) {
		BrushPanel* panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(iz));
		panel->InvalidateContents();
	}
	PalettePanel::InvalidateContents();
}

void BrushPalettePanel::LoadCurrentContents() {
	wxWindow* page = choicebook->GetCurrentPage();
	BrushPanel* panel = dynamic_cast<BrushPanel*>(page);
	if (panel) {
		panel->OnSwitchIn();
	}
	PalettePanel::LoadCurrentContents();
}

void BrushPalettePanel::LoadAllContents() {
	for (size_t iz = 0; iz < choicebook->GetPageCount(); ++iz) {
		BrushPanel* panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(iz));
		panel->LoadContents();
	}
	PalettePanel::LoadAllContents();
}

PaletteType BrushPalettePanel::GetType() const {
	return palette_type;
}

void BrushPalettePanel::SetListType(wxString ltype) {
	if (!choicebook) {
		return;
	}
	for (size_t iz = 0; iz < choicebook->GetPageCount(); ++iz) {
		BrushPanel* panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(iz));
		panel->SetListType(ltype);
	}
}

Brush* BrushPalettePanel::GetSelectedBrush() const {
	if (!choicebook) {
		return nullptr;
	}
	wxWindow* page = choicebook->GetCurrentPage();
	BrushPanel* panel = dynamic_cast<BrushPanel*>(page);
	Brush* res = nullptr;
	if (panel) {
		for (ToolBarList::const_iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
			res = (*iter)->GetSelectedBrush();
			if (res) {
				return res;
			}
		}
		res = panel->GetSelectedBrush();
	}
	return res;
}

void BrushPalettePanel::SelectFirstBrush() {
	if (!choicebook) {
		return;
	}
	wxWindow* page = choicebook->GetCurrentPage();
	BrushPanel* panel = dynamic_cast<BrushPanel*>(page);
	panel->SelectFirstBrush();
}

bool BrushPalettePanel::SelectBrush(const Brush* whatbrush) {
	if (!choicebook) {
		return false;
	}

	BrushPanel* panel = dynamic_cast<BrushPanel*>(choicebook->GetCurrentPage());
	if (!panel) {
		return false;
	}

	for (PalettePanel* toolBar : tool_bars) {
		if (toolBar->SelectBrush(whatbrush)) {
			panel->SelectBrush(nullptr);
			return true;
		}
	}

	for (PalettePanel* toolBar : tool_bars) {
		toolBar->DeselectAll();
	}

	if (panel->SelectBrush(whatbrush)) {
		for (PalettePanel* toolBar : tool_bars) {
			toolBar->SelectBrush(nullptr);
		}
		return true;
	}

	for (size_t iz = 0; iz < choicebook->GetPageCount(); ++iz) {
		if ((int)iz == choicebook->GetSelection()) {
			continue;
		}

		panel = dynamic_cast<BrushPanel*>(choicebook->GetPage(iz));
		if (panel && panel->SelectBrush(whatbrush)) {
			choicebook->ChangeSelection(iz);
			for (PalettePanel* toolBar : tool_bars) {
				toolBar->SelectBrush(nullptr);
			}
			return true;
		}
	}
	return false;
}

void BrushPalettePanel::OnSwitchingPage(wxChoicebookEvent& event) {
	event.Skip();
	if (!choicebook) {
		return;
	}

	// Get the old panel and clean it up
	BrushPanel* old_panel = dynamic_cast<BrushPanel*>(choicebook->GetCurrentPage());
	if (old_panel) {
		old_panel->OnSwitchOut();

		// Store selected brushes for later restoration
		for (ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
			Brush* tmp = (*iter)->GetSelectedBrush();
			if (tmp) {
				remembered_brushes[old_panel] = tmp;
			}
		}
	}

	// Get the new panel and prepare it
	wxWindow* page = choicebook->GetPage(event.GetSelection());
	BrushPanel* panel = dynamic_cast<BrushPanel*>(page);
	if (panel) {
		panel->OnSwitchIn();

		// Restore remembered brush selection if any
		for (ToolBarList::iterator iter = tool_bars.begin(); iter != tool_bars.end(); ++iter) {
			(*iter)->SelectBrush(remembered_brushes[panel]);
		}
	}
}
""",
    "wxwidgets/palette_brushlist.h": """\
//////////////////////////////////////////////////////////////////////
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

#ifndef RME_PALETTE_BRUSHLIST_H_
#define RME_PALETTE_BRUSHLIST_H_

#include <wx/panel.h>
#include <wx/listctrl.h>
#include <wx/vscroll.h>
#include <wx/timer.h>
#include <wx/dcbuffer.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/choicebk.h>

#include <map>
#include <set>
#include <vector>

#include "tileset.h"
#include "palette_common.h"

// Forward declarations
class ItemPalettePanel;
class BrushPalettePanel;
class BrushButton;
class BrushPanel;
class BrushBoxInterface;

enum BrushListType {
	BRUSHLIST_LARGE_ICONS,
	BRUSHLIST_SMALL_ICONS,
	BRUSHLIST_LISTBOX,
	BRUSHLIST_TEXT_LISTBOX,
	BRUSHLIST_GRID,
	BRUSHLIST_DIRECT_DRAW,
	BRUSHLIST_SEAMLESS_GRID
};

enum {
	PALETTE_LAYOUT_STYLE_BORDER,
	PALETTE_LAYOUT_STYLE_LARGE,
	PALETTE_LAYOUT_STYLE_LISTBOX,
	PALETTE_LAYOUT_STYLE_NEWUI,
	BUTTON_QUICK_ADD_ITEM,
	BUTTON_ADD_BORDER
};

// Custom ID for Quick Add button
#define BUTTON_QUICK_ADD_ITEM 1001
#define BUTTON_ADD_BORDER 1002

class BrushBoxInterface {
public:
	BrushBoxInterface(const TilesetCategory* _tileset) :
		tileset(_tileset), loaded(false) {
		ASSERT(tileset);
	}
	virtual ~BrushBoxInterface() { }

	virtual wxWindow* GetSelfWindow() = 0;

	// Select the first brush
	virtual void SelectFirstBrush() = 0;
	// Returns the currently selected brush (First brush if panel is not loaded)
	virtual Brush* GetSelectedBrush() const = 0;
	// Select the brush in the parameter, this only changes the look of the panel
	virtual bool SelectBrush(const Brush* brush) = 0;

protected:
	const TilesetCategory* tileset;
	bool loaded;
};

// New class for direct drawing of sprites in grid
class DirectDrawBrushPanel : public wxScrolledWindow, public BrushBoxInterface {
public:
	DirectDrawBrushPanel(wxWindow* parent, const TilesetCategory* _tileset);
	~DirectDrawBrushPanel();

	wxWindow* GetSelfWindow() { return this; }

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter
	bool SelectBrush(const Brush* brush);

	// Event handling
	void OnMouseClick(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnTimer(wxTimerEvent& event);

	// Make timer accessible
	wxTimer* loading_timer;

protected:
	void RecalculateGrid();
	void DrawItemsToPanel(wxDC& dc);
	void UpdateViewableItems(); // Method for lazy loading visible items
	void StartProgressiveLoading(); // Method for progressive loading with visual feedback

protected:
	int columns;
	int item_width;
	int item_height;
	int selected_index;
	wxBitmap* buffer;

	// Lazy loading properties
	int first_visible_row;
	int last_visible_row;
	int visible_rows_margin; // Extra rows to load above/below viewport
	int total_rows;
	bool need_full_redraw;

	// Progressive loading properties
	bool use_progressive_loading;
	bool is_large_tileset;
	int loading_step;
	int max_loading_steps;
	// Moved loading_timer to public section

	static const int LARGE_TILESET_THRESHOLD = 1000; // Number of items considered "large"

	DECLARE_EVENT_TABLE();
};

class BrushListBox : public wxVListBox, public BrushBoxInterface {
public:
	BrushListBox(wxWindow* parent, const TilesetCategory* _tileset);
	~BrushListBox();

	wxWindow* GetSelfWindow() {
		return this;
	}

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (First brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* brush);

	// Event handlers
	virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
	virtual wxCoord OnMeasureItem(size_t n) const;

	void OnKey(wxKeyEvent& event);

	DECLARE_EVENT_TABLE();
};

class BrushIconBox : public wxScrolledWindow, public BrushBoxInterface {
public:
	BrushIconBox(wxWindow* parent, const TilesetCategory* _tileset, RenderSize rsz);
	~BrushIconBox();

	wxWindow* GetSelfWindow() {
		return this;
	}

	// Scrolls the window to the position of the named brush button
	void EnsureVisible(BrushButton* btn);
	void EnsureVisible(size_t n);

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (First brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* brush);

	// Event handling...
	void OnClickBrushButton(wxCommandEvent& event);

protected:
	// Used internally to deselect all buttons before selecting a newd one.
	void DeselectAll();

protected:
	std::vector<BrushButton*> brush_buttons;
	RenderSize icon_size;

	DECLARE_EVENT_TABLE();
};

class BrushGridBox : public wxScrolledWindow, public BrushBoxInterface {
public:
	BrushGridBox(wxWindow* parent, const TilesetCategory* _tileset);
	~BrushGridBox();

	wxWindow* GetSelfWindow() { return this; }

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter
	bool SelectBrush(const Brush* brush);

	// Event handling
	void OnClickBrushButton(wxCommandEvent& event);
	void OnSize(wxSizeEvent& event);

protected:
	void RecalculateGrid();
	void DeselectAll();

protected:
	std::vector<BrushButton*> brush_buttons;
	wxFlexGridSizer* grid_sizer;
	int columns;

	DECLARE_EVENT_TABLE();
};

// New class for seamless sprite grid with direct rendering
class SeamlessGridPanel : public wxScrolledWindow, public BrushBoxInterface {
public:
	SeamlessGridPanel(wxWindow* parent, const TilesetCategory* _tileset);
	~SeamlessGridPanel();

	wxWindow* GetSelfWindow() { return this; }

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter
	bool SelectBrush(const Brush* brush);

	// Set whether to display item IDs
	void SetShowItemIDs(bool show) { show_item_ids = show; Refresh(); }
	bool IsShowingItemIDs() const { return show_item_ids; }

	// Zoom level control methods
	int GetZoomLevel() const { return zoom_level; }
	int IncrementZoom();
	int DecrementZoom();
	void SetZoomLevel(int level);

	// Move to public section
	void ClearSpriteCache();

	// Make timer accessible
	wxTimer* loading_timer;

	// Event handling
	void OnMouseClick(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnKeyDown(wxKeyEvent& event); // New keyboard handler

protected:
	void RecalculateGrid();
	void DrawItemsToPanel(wxDC& dc);
	void DrawSpriteAt(wxDC& dc, int x, int y, int index);
	void UpdateViewableItems();
	void StartProgressiveLoading();
	void UpdateGridSize();
	// Moved ClearSpriteCache to public section
	void ManageSpriteCache();
	int GetSpriteIndexAt(int x, int y) const;
	void SelectIndex(int index);
	void CreateNavigationPanel(wxWindow* parent);
	void UpdateNavigationPanel();
	void OnNavigationButtonClicked(wxCommandEvent& event);

private:
	int columns;
	int sprite_size;
	int zoom_level; // Zoom level (1=32px, 2=64px, etc)
	int selected_index;
	int hover_index;
	wxBitmap* buffer;
	bool show_item_ids;

	// Rendering optimization
	int first_visible_row;
	int last_visible_row;
	int visible_rows_margin;
	int total_rows;
	bool need_full_redraw;

	// Progressive loading for large tilesets
	bool use_progressive_loading;
	bool is_large_tileset;
	int loading_step;
	int max_loading_steps;
	// Moved loading_timer to public section

	// Chunking properties
	int chunk_size;       // Number of items per chunk
	int current_chunk;    // Current chunk being displayed
	int total_chunks;     // Total number of chunks
	wxRect prev_chunk_button; // Rectangle for previous chunk button
	wxRect next_chunk_button; // Rectangle for next chunk button
	wxPanel* navigation_panel; // Panel for navigation buttons

	// Constants
	static const int LARGE_TILESET_THRESHOLD = 1000; // Number of items considered "large"

	// Cache structure for sprite rendering
	struct CachedSprite {
		wxBitmap bitmap;
		int zoom_level;
		bool is_valid;

		CachedSprite() : zoom_level(1), is_valid(false) {}
	};

	std::map<int, CachedSprite> sprite_cache;

	DECLARE_EVENT_TABLE();
};

// A panel capable of displaying a collection of brushes
class BrushPanel : public wxPanel {
public:
	BrushPanel(wxWindow* parent);
	~BrushPanel();

	// Interface
	void InvalidateContents();
	void LoadContents();

	// Sets the display type (list or icons)
	void SetListType(BrushListType ltype);
	void SetListType(wxString ltype);
	// Assigns a tileset to this list
	void AssignTileset(const TilesetCategory* tileset);

	// Set whether to display item IDs
	void SetShowItemIDs(bool show);

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter
	bool SelectBrush(const Brush* whatbrush);

	// Called when the window is about to be displayed
	void OnSwitchIn();
	// Called when this page is hidden
	void OnSwitchOut();

	// wxWidgets event handlers
	void OnClickListBoxRow(wxCommandEvent& event);
	void OnViewModeToggle(wxCommandEvent& event);
	void OnShowItemIDsToggle(wxCommandEvent& event);

	// Get the panel's sizer
	wxSizer* GetSizer() const { return sizer; }

	// Make sizer public so it can be accessed from navigation panel
	wxSizer* sizer;

protected:
	void LoadViewMode();
	void CleanupBrushbox(BrushBoxInterface* box);

protected:
	const TilesetCategory* tileset;
	BrushBoxInterface* brushbox;
	bool loaded;
	BrushListType list_type;
	wxCheckBox* view_mode_toggle;
	wxChoice* view_type_choice;
	wxCheckBox* show_ids_toggle;

	DECLARE_EVENT_TABLE();
};

class BrushPalettePanel : public PalettePanel {
public:
	BrushPalettePanel(wxWindow* parent, const TilesetContainer& tilesets, TilesetCategoryType category, wxWindowID id = wxID_ANY);
	virtual ~BrushPalettePanel();

	// Structure to hold selection information
	struct SelectionInfo {
		std::vector<Brush*> brushes;
	};

	// Get currently selected brushes
	const SelectionInfo& GetSelectionInfo() const;

	virtual void InvalidateContents() override;
	virtual void LoadCurrentContents() override;
	virtual void LoadAllContents() override;

	PaletteType GetType() const override;

	// Sets the display type (list or icons)
	void SetListType(BrushListType ltype);
	void SetListType(wxString ltype);

	virtual void SelectFirstBrush() override;
	virtual Brush* GetSelectedBrush() const override;
	virtual bool SelectBrush(const Brush* whatbrush) override;

	virtual void OnSwitchIn() override;
	void OnSwitchingPage(wxChoicebookEvent& event);
	void OnPageChanged(wxChoicebookEvent& event);
	void OnClickAddTileset(wxCommandEvent& event);
	void OnClickAddItemTileset(wxCommandEvent& event);
	void OnClickQuickAddItemTileset(wxCommandEvent& event);
	void OnClickCreateBorder(wxCommandEvent& event);

	// Properly destroy all caches and resources
	void DestroyAllCaches();

protected:
	wxButton* quick_add_button;
	std::string last_tileset_name;
	wxChoicebook* choicebook;
	TilesetCategoryType palette_type;
	PalettePanel* size_panel;
	std::map<wxWindow*, Brush*> remembered_brushes;

	DECLARE_EVENT_TABLE();
};

#endif
""",
    "wxwidgets/palette_creature.cpp": """\
//////////////////////////////////////////////////////////////////////
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
""",
    "wxwidgets/palette_creature.h": """\
//////////////////////////////////////////////////////////////////////
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
    "wxwidgets/palette_house.cpp": """\
//////////////////////////////////////////////////////////////////////
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
""",
    "wxwidgets/palette_house.h": """\
//////////////////////////////////////////////////////////////////////
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
    "wxwidgets/palette_waypoints.cpp": """\
//////////////////////////////////////////////////////////////////////
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
    "wxwidgets/palette_waypoints.h": """\
//////////////////////////////////////////////////////////////////////
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
for path, content in file_contents.items():
    md5_hash = hashlib.md5(content.encode('utf-8')).hexdigest()
    content_lite = "\\n".join(content.splitlines()[:200])
    description = "" # Add specific descriptions if needed, or use a generic one
    if path == "wxwidgets/main_toolbar.cpp":
        description = "Implements `MainToolBar` which creates and manages several `wxAuiToolBar` instances (standard, brushes, position, sizes) with various controls and actions. Handles their layout and updates their state."
    elif path == "wxwidgets/main_toolbar.h":
        description = "Header for `MainToolBar`."
    elif path == "wxwidgets/palette_window.cpp":
        description = "Implements `PaletteWindow`, a `wxPanel` containing a `wxChoicebook` to switch between different palette types (Terrain, Doodad, Item, Creature, House, Waypoint, RAW). Manages creation and state of these individual palette panels."
    elif path == "wxwidgets/palette_window.h":
        description = "Header for `PaletteWindow`."
    elif path == "wxwidgets/palette_brushlist.cpp":
        description = "Implements `BrushPalettePanel` (generic panel for Terrain, Doodad, Item, etc., palettes, containing another `wxChoicebook` for tilesets) and `BrushPanel` (displays brushes from a specific tileset category using different view modes like lists or icon grids, including `DirectDrawBrushPanel` and `SeamlessGridPanel`)."
    elif path == "wxwidgets/palette_brushlist.h":
        description = "Header for `BrushPalettePanel`, `BrushPanel`, and various brush display interfaces/classes (`BrushBoxInterface`, `BrushListBox`, `BrushIconBox`, `DirectDrawBrushPanel`, `SeamlessGridPanel`)."
    elif path == "wxwidgets/palette_creature.cpp":
        description = "Implements `CreaturePalettePanel`, specialized for selecting creature brushes and spawn settings. Includes search, view toggles (list/sprite), and NPC/Monster file loading."
    elif path == "wxwidgets/palette_creature.h":
        description = "Header for `CreaturePalettePanel`, `CreatureSpritePanel`, and `CreatureSeamlessGridPanel`."
    elif path == "wxwidgets/palette_house.cpp":
        description = "Implements `HousePalettePanel` for managing house brushes, selecting towns, and house exits. Includes an `EditHouseDialog`."
    elif path == "wxwidgets/palette_house.h":
        description = "Header for `HousePalettePanel` and `EditHouseDialog`."
    elif path == "wxwidgets/palette_waypoints.cpp":
        description = "Implements `WaypointPalettePanel` for listing, adding, removing, and renaming waypoints."
    elif path == "wxwidgets/palette_waypoints.h":
        description = "Header for `WaypointPalettePanel`."

    analyzed_files_data.append({
        "file_path": path,
        "description": description,
        "md5_hash": md5_hash,
        "content_lite": content_lite
    })

yaml_data = {
    "wbs_item_id": "UI-02",
    "name": "Port Toolbars & Palettes",
    "description": "Recreate main application toolbars (standard, brushes, position, sizes) and the palette system (terrain, doodad, item, creature, house, waypoint, etc.) using Qt6 widgets.",
    "dependencies": [
        "UI-01",
        "CORE-02",
        "CORE-04",
    ],
    "input_files": list(file_contents.keys()),
    "analyzed_input_files": analyzed_files_data,
    "documentation_references": [
        "QToolBar: https://doc.qt.io/qt-6/qtoolbar.html",
        "QAction: https://doc.qt.io/qt-6/qaction.html",
        "QDockWidget: https://doc.qt.io/qt-6/qdockwidget.html",
        "QTabWidget: https://doc.qt.io/qt-6/qtabwidget.html",
        "QListView / QListWidget: https://doc.qt.io/qt-6/qlistview.html",
        "QAbstractItemModel / QStyledItemDelegate: for custom list/grid views.",
        "QSpinBox, QLineEdit, QPushButton, QComboBox for various controls."
    ],
    "current_functionality_summary": """\
The wxWidgets version has a `MainToolBar` class that creates and manages four dockable `wxAuiToolBar` instances:
1.  Standard Toolbar: File operations, Undo/Redo, Cut/Copy/Paste.
2.  Brushes Toolbar: Toggle buttons for specialized brushes like Eraser, Zone brushes, Door types, Window types.
3.  Position Toolbar: X, Y, Z coordinate inputs and a "Go" button.
4.  Sizes Toolbar: Toggles for brush shape (circular/square) and predefined brush sizes.
Toolbars use `wxArtProvider` or PNGs for icons. Their state (enabled/disabled, toggled) is updated based on application context.

The `PaletteWindow` is a `wxPanel` hosting a `wxChoicebook` to switch between different palette types (Terrain, Doodad, Item, Collection, Creature, House, Waypoint, RAW).
- `BrushPalettePanel` is a generic base for several palette types, itself containing a `wxChoicebook` for different tilesets/categories. Each page of this inner choicebook is a `BrushPanel`.
- `BrushPanel` displays brushes using various view modes (list, icon grid, direct draw, seamless grid).
- Specialized palettes (`CreaturePalettePanel`, `HousePalettePanel`, `WaypointPalettePanel`) have custom UIs for their specific functionalities, including search, settings inputs, and dedicated brush types. Palettes are populated based on loaded XML asset data (`tilesets.xml`, `items.xml`, etc.) or map-specific data (houses, waypoints).\
""",
    "definition_of_done": [
        "The four main application toolbars (Standard, Brushes, Position, Sizes) are recreated as `QToolBar` objects within the `QMainWindow`.",
        "Actions on the Standard and Brushes toolbars are implemented using `QAction`s with appropriate icons and tooltips, connected to placeholder slots.",
        "The Position toolbar includes `QSpinBox` (or validated `QLineEdit`) controls for X, Y, Z coordinates and a `QPushButton` for 'Go'.",
        "The Sizes toolbar uses `QActionGroup`s or checkable `QAction`s to allow selection of brush shape and size, with visual feedback.",
        "A `QDockWidget` is created to house the main palette system.",
        "Inside the dock widget, a `QTabWidget` (`paletteTabWidget`) is used to switch between different palette types (Terrain, Doodad, Item, Creature, House, Waypoint, RAW, Collection).",
        "For generic palette types (Terrain, Doodad, Item, RAW, Collection):",
        "  - Each tab contains a `QComboBox` or another `QTabWidget` to select the specific tileset/category (e.g., 'Forest Grounds', 'City Items').",
        "  - Brushes/items from the selected tileset/category are displayed in a `QListView` (with a custom model/delegate for icon+text or grid view) or a `QListWidget`.",
        "  - Basic search/filter functionality using a `QLineEdit` is provided for these palettes.",
        "Specialized palettes are implemented:",
        "  - `CreaturePalette`: `QComboBox` for creature categories, search field, display area (list or grid), `QSpinBox` for spawn time/radius, toggles for creature/spawn brush.",
        "  - `HousePalette`: `QComboBox` for towns, `QListWidget` for houses, Add/Edit/Remove buttons, toggles for house/exit brush.",
        "  - `WaypointPalette`: `QListWidget` with editable items for waypoint names, Add/Remove buttons.",
        "Selecting a brush/item in any palette correctly updates the application's global current brush state.",
        "Toolbars and palettes are correctly enabled/disabled/updated based on application context (e.g., map open, current tool)."
    ],
    "boilerplate_coder_ai_prompt": """\
Your task is to port the main application toolbars and the comprehensive palette system from wxWidgets to Qt6. This involves recreating multiple toolbars with various controls and a tabbed palette window that displays different types of brushes and game elements.

**Reference Files:** `wxwidgets/main_toolbar.*`, `wxwidgets/palette_window.*`, `wxwidgets/palette_brushlist.*`, `wxwidgets/palette_creature.*`, `wxwidgets/palette_house.*`, `wxwidgets/palette_waypoints.*`. Palette content is driven by XML asset files.

**I. Main Toolbars (to be added to `QMainWindow`):**

1.  **Standard Toolbar (`QToolBar` named `standardToolbar`):**
    - Actions: New, Open, Save, Save As, Undo, Redo, Cut, Copy, Paste.
    - Use `QAction` for each, set icons (e.g., `QIcon::fromTheme("document-new")` or custom PNGs) and tooltips. Connect `triggered()` signals to placeholder slots.

2.  **Brushes Toolbar (`QToolBar` named `brushesToolbar`):**
    - Checkable `QAction`s for: Eraser, Optional Border, PZ, NoPVP, NoLogout, PvPZone, Zone Brush.
    - Checkable `QAction`s for Door Types: Normal, Locked, Magic, Quest, Normal Alt, Archway.
    - Checkable `QAction`s for Window Types: Hatch, Window.
    - Use a `QActionGroup` if some of these are mutually exclusive. Set icons and tooltips.

3.  **Position Toolbar (`QToolBar` named `positionToolbar`):**
    - Three `QSpinBox` widgets for X, Y, Z coordinates. Set appropriate ranges (e.g., 0 to map max width/height/depth).
    - A `QPushButton` labeled "Go".
    - Connect `QSpinBox::valueChanged` and `QPushButton::clicked` to slots.

4.  **Sizes Toolbar (`QToolBar` named `sizesToolbar`):**
    - `QActionGroup` for Brush Shape:
        - `QAction` "Rectangular" (checked by default).
        - `QAction` "Circular".
    - `QActionGroup` for Brush Size (or individual checkable `QAction`s if preferred, ensuring only one is active):
        - Actions for sizes 1x1, 2x2, 3x3, ..., 7x7 (or equivalent radius for circular). Set icons representing the size/shape.
    - Connect `triggered()` signals.

5.  **Toolbar Management:**
    - Implement `void MainWindow::updateToolbars()` to enable/disable toolbar actions based on application state (map loaded, selection active, etc.).
    - Implement `void MainWindow::updateBrushToolbarStates()` to reflect the currently selected global brush.
    - Implement `void MainWindow::updateBrushSizeToolbar(BrushShape shape, int size)` to update the Sizes toolbar.

**II. Palette System (within a `QDockWidget`):**

1.  **Main Palette Container:**
    - Use a `QDockWidget` (e.g., titled "Palettes").
    - Inside, place a `QTabWidget` (`paletteTabWidget`) for different palette categories.

2.  **Generic Brush Palette (`BrushPalettePanel` equivalent for Terrain, Doodad, Item, RAW, Collection):**
    - For each of these types, create a tab in `paletteTabWidget`.
    - Each such tab will contain:
        - A `QComboBox` (or another `QTabWidget`) to select the specific *tileset category* (e.g., "Forest Grounds," "City Items," loaded from `g_materials.tilesets`).
        - A display area for brushes/items from the selected tileset:
            - Recommended: `QListView` with a custom `QAbstractListModel` and `QStyledItemDelegate` for rendering icons and text.
            - Alternative: `QListWidget` (simpler, less flexible).
            - Consider a toggle for list view vs. icon grid view (`QListView::setViewMode`).
        - A `QLineEdit` for searching/filtering items within the current tileset category.
        - For palettes that use them (Terrain, Doodad, Collection), integrate tool panels (Brush Tools, Thickness, Size) as separate widgets or toolbars within this palette tab. The original `BrushPalettePanel` added these via `AddToolPanel`.

3.  **Specialized Palettes:**

    - **Creature Palette Tab:**
        - `QComboBox` for creature tilesets (e.g., "All Creatures," "NPCs," "Forest Monsters").
        - `QLineEdit` for search (by name, or "lt:id" for looktype).
        - Display area: `QListView` or a custom grid widget (like `CreatureSeamlessGridPanel`) to show creature sprites/names.
        - `QSpinBox` for "Spawntime" and "Spawn Size/Radius".
        - `QRadioButton` or `QActionGroup` for "Creature Brush" vs. "Spawn Brush" mode.
        - `QPushButton`s: "Load NPCs Folder," "Load Monsters Folder," "Purge Creatures."
        - Toggles for "Sprite View" / "List View", "64x64 Sprites", "Zoom" for sprite view.

    - **House Palette Tab:**
        - `QComboBox` to select `Town`.
        - `QListWidget` to display houses in the selected town (Name, ID, Size).
        - `QPushButton`s: "Add House," "Edit House," "Remove House."
        - `QRadioButton` or `QActionGroup` for "House Tile Brush" vs. "Select Exit Brush."

    - **Waypoint Palette Tab:**
        - `QListWidget` where items are waypoint names. Items should be editable (`Qt::ItemIsEditable`).
        - `QPushButton`s: "Add Waypoint," "Remove Waypoint."

4.  **Palette Interaction:**
    - When a brush/item/creature/etc. is selected in any palette, a signal should be emitted, which the `MainWindow` or `Editor` class connects to, to set the application's current active brush.
    - Implement logic similar to `PaletteWindow::OnSelectBrush` to switch to the correct palette tab and select the brush if it's activated externally (e.g., hotkey).
    - Palette content should be refreshed when data changes (e.g., after `CORE-02` or `CORE-04` reload assets, or map waypoints/houses change).

This is a large UI component. Break it down into manageable sub-widgets for each palette type.
The `analyzed_input_files` section should be populated by reading the first 200 lines of the listed C++ files and calculating their MD5 hashes.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/UI-02.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_data, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")

# Clean up the temporary file contents from variables
del file_contents
del yaml_data
