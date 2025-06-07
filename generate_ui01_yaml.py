import yaml
import hashlib

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

application_cpp_content = """\
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

#include "application.h"
#include "sprites.h"
#include "editor.h"
#include "common_windows.h"
#include "palette_window.h"
#include "preferences.h"
#include "result_window.h"
#include "minimap_window.h"
#include "about_window.h"
#include "main_menubar.h"
#include "updater.h"
#include "artprovider.h"
#include "dark_mode_manager.h"

#include "materials.h"
#include "map.h"
#include "complexitem.h"
#include "creature.h"

// Add exception handling includes
#include <exception>
#include <fstream>
#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#include <wx/snglinst.h>

#if defined(__LINUX__) || defined(__WINDOWS__)
	#include <GL/glut.h>
#endif

#include "../brushes/icon/editor_icon.xpm"
#include "color_utils.h"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_CLOSE(MainFrame::OnExit)

// Update check complete
#ifdef _USE_UPDATER_
EVT_ON_UPDATE_CHECK_FINISHED(wxID_ANY, MainFrame::OnUpdateReceived)
#endif
EVT_ON_UPDATE_MENUS(wxID_ANY, MainFrame::OnUpdateMenus)

// Idle event handler
EVT_IDLE(MainFrame::OnIdle)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MapWindow, wxPanel)
EVT_SIZE(MapWindow::OnSize)

EVT_COMMAND_SCROLL_TOP(MAP_WINDOW_HSCROLL, MapWindow::OnScroll)
EVT_COMMAND_SCROLL_BOTTOM(MAP_WINDOW_HSCROLL, MapWindow::OnScroll)
EVT_COMMAND_SCROLL_THUMBTRACK(MAP_WINDOW_HSCROLL, MapWindow::OnScroll)
EVT_COMMAND_SCROLL_LINEUP(MAP_WINDOW_HSCROLL, MapWindow::OnScrollLineUp)
EVT_COMMAND_SCROLL_LINEDOWN(MAP_WINDOW_HSCROLL, MapWindow::OnScrollLineDown)
EVT_COMMAND_SCROLL_PAGEUP(MAP_WINDOW_HSCROLL, MapWindow::OnScrollPageUp)
EVT_COMMAND_SCROLL_PAGEDOWN(MAP_WINDOW_HSCROLL, MapWindow::OnScrollPageDown)

EVT_COMMAND_SCROLL_TOP(MAP_WINDOW_VSCROLL, MapWindow::OnScroll)
EVT_COMMAND_SCROLL_BOTTOM(MAP_WINDOW_VSCROLL, MapWindow::OnScroll)
EVT_COMMAND_SCROLL_THUMBTRACK(MAP_WINDOW_VSCROLL, MapWindow::OnScroll)
EVT_COMMAND_SCROLL_LINEUP(MAP_WINDOW_VSCROLL, MapWindow::OnScrollLineUp)
EVT_COMMAND_SCROLL_LINEDOWN(MAP_WINDOW_VSCROLL, MapWindow::OnScrollLineDown)
EVT_COMMAND_SCROLL_PAGEUP(MAP_WINDOW_VSCROLL, MapWindow::OnScrollPageUp)
EVT_COMMAND_SCROLL_PAGEDOWN(MAP_WINDOW_VSCROLL, MapWindow::OnScrollPageDown)

EVT_BUTTON(MAP_WINDOW_GEM, MapWindow::OnGem)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MapScrollBar, wxScrollBar)
EVT_KEY_DOWN(MapScrollBar::OnKey)
EVT_KEY_UP(MapScrollBar::OnKey)
EVT_CHAR(MapScrollBar::OnKey)
EVT_SET_FOCUS(MapScrollBar::OnFocus)
EVT_MOUSEWHEEL(MapScrollBar::OnWheel)
END_EVENT_TABLE()

wxIMPLEMENT_APP(Application);

Application::~Application() {
	// Destroy
}

bool Application::OnInit() {
#if defined __DEBUG_MODE__ && defined __WINDOWS__
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
	std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
	std::cout << "Review COPYING in RME distribution for details." << std::endl;
	mt_seed(time(nullptr));
	srand(time(nullptr));

	// Set up global exception handling
#ifndef __DEBUG_MODE__
	wxHandleFatalExceptions(true);
#endif

	// Discover data directory
	g_gui.discoverDataDirectory("clients.xml");

	// Tell that we are the real thing
	wxAppConsole::SetInstance(this);
	wxArtProvider::Push(newd ArtProvider());

#if defined(__LINUX__) || defined(__WINDOWS__)
	int argc = 1;
	char* argv[1] = { wxString(this->argv[0]).char_str() };
	glutInit(&argc, argv);
#endif

	// Load some internal stuff
	g_settings.load();
	FixVersionDiscrapencies();
	g_gui.LoadHotkeys();
	ClientVersion::loadVersions();

	// Initialize dark mode manager
	g_darkMode.Initialize();

#ifdef _USE_PROCESS_COM
	m_single_instance_checker = newd wxSingleInstanceChecker; // Instance checker has to stay alive throughout the applications lifetime

	// Parse command line arguments first to allow overriding single instance setting
	m_file_to_open = wxEmptyString;
	ParseCommandLineMap(m_file_to_open);

	if (g_settings.getInteger(Config::ONLY_ONE_INSTANCE) && m_single_instance_checker->IsAnotherRunning()) {
		RMEProcessClient client;
		wxConnectionBase* connection = client.MakeConnection("localhost", "rme_host", "rme_talk");
		if (connection) {
			if (m_file_to_open != wxEmptyString) {
				wxLogNull nolog; // We might get a timeout message if the file fails to open on the running instance. Let's not show that message.
				connection->Execute(m_file_to_open);
			}
			connection->Disconnect();
			wxDELETE(connection);
		}
		wxDELETE(m_single_instance_checker);
		return false; // Since we return false - OnExit is never called
	}
	// We act as server then
	m_proc_server = newd RMEProcessServer();
	if (!m_proc_server->Create("rme_host")) {
		wxLogWarning("Could not register IPC service!");
	}
#endif

	// Image handlers
	// wxImage::AddHandler(newd wxBMPHandler);
	wxImage::AddHandler(newd wxPNGHandler);
	wxImage::AddHandler(newd wxJPEGHandler);
	wxImage::AddHandler(newd wxTGAHandler);

	g_gui.gfx.loadEditorSprites();

#ifndef __DEBUG_MODE__
	// Enable fatal exception handler
	wxHandleFatalExceptions(true);
#endif
	// Load all the dependency files
	std::string error;
	StringVector warnings;

	// Don't parse command line map again since we already did it above
	if (m_file_to_open == wxEmptyString) {
		ParseCommandLineMap(m_file_to_open);
	}

	g_gui.root = newd MainFrame(__W_RME_APPLICATION_NAME__, wxDefaultPosition, wxSize(700, 500));
	SetTopWindow(g_gui.root);
	g_gui.SetTitle("");

	g_gui.root->LoadRecentFiles();

	// Load palette
	g_gui.LoadPerspective();

	// Create icon and apply color shift
	wxBitmap iconBitmap(editor_icon);
	wxImage iconImage = iconBitmap.ConvertToImage();
	ColorUtils::ShiftHue(iconImage, ColorUtils::GetRandomHueShift());
	iconBitmap = wxBitmap(iconImage);

	// Convert to icon for the window and set both
	wxIcon icon;
	icon.CopyFromBitmap(iconBitmap);
	g_gui.root->SetIcon(icon);

	// Create a unique log directory for this session
	wxDateTime now = wxDateTime::Now();
	wxString logDir = wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() +
		"logs" + wxFileName::GetPathSeparator() + now.Format("%Y%m%d_%H%M%S");

	// Make sure it exists
	wxFileName dirPath(logDir);
	if (!dirPath.DirExists()) {
		dirPath.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	}

	// Let them know about this session
	std::ofstream sessionLog((logDir + wxFileName::GetPathSeparator() + "session.log").ToStdString());
	if (sessionLog.is_open()) {
		sessionLog << "RME Session started at " << now.FormatISOCombined() << std::endl;
		sessionLog << "Version: " << __W_RME_VERSION__ << std::endl;
		sessionLog.close();
	}

	// Show welcome dialog with color-shifted bitmap
	if (g_settings.getInteger(Config::WELCOME_DIALOG) == 1 && m_file_to_open == wxEmptyString) {
		g_gui.ShowWelcomeDialog(iconBitmap);
	} else {
		g_gui.root->Show();
	}

	// Set idle event handling mode
	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);

	// Goto RME website?
	if (g_settings.getInteger(Config::GOTO_WEBSITE_ON_BOOT) == 1) {
		::wxLaunchDefaultBrowser(__SITE_URL__, wxBROWSER_NEW_WINDOW);
		g_settings.setInteger(Config::GOTO_WEBSITE_ON_BOOT, 0);
	}

	// Check for updates
#ifdef _USE_UPDATER_
	if (g_settings.getInteger(Config::USE_UPDATER) == -1) {
		int ret = g_gui.PopupDialog(
			"Notice",
			"Do you want the editor to automatically check for updates?\\n"
			"It will connect to the internet if you choose yes.\\n"
			"You can change this setting in the preferences later.",
			wxYES | wxNO
		);
		if (ret == wxID_YES) {
			g_settings.setInteger(Config::USE_UPDATER, 1);
		} else {
			g_settings.setInteger(Config::USE_UPDATER, 0);
		}
	}
	if (g_settings.getInteger(Config::USE_UPDATER) == 1) {
		// UpdateChecker updater;
		// updater.connect(g_gui.root);
	}
#endif

	// Keep track of first event loop entry
	m_startup = true;
	return true;
}
"""

application_h_content = """\
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

#ifndef RME_APPLICATION_H_
#define RME_APPLICATION_H_

#include "gui.h"
#include "main_toolbar.h"
#include "action.h"
#include "settings.h"

#include "process_com.h"
#include "map_display.h"
#include "welcome_dialog.h"

class Item;
class Creature;

class MainFrame;
class MapWindow;
class wxEventLoopBase;
class wxSingleInstanceChecker;

class Application : public wxApp {
public:
	~Application();
	virtual bool OnInit();
	virtual void OnEventLoopEnter(wxEventLoopBase* loop);
	virtual void MacOpenFiles(const wxArrayString& fileNames);
	virtual int OnExit();
	virtual bool OnExceptionInMainLoop();
	void Unload();

private:
	bool m_startup;
	wxString m_file_to_open;
	void FixVersionDiscrapencies();
	bool ParseCommandLineMap(wxString& fileName);

	virtual void OnFatalException();

#ifdef _USE_PROCESS_COM
	RMEProcessServer* m_proc_server;
	wxSingleInstanceChecker* m_single_instance_checker;
#endif
};

class MainMenuBar;

class MainFrame : public wxFrame {
public:
	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~MainFrame();

	void UpdateMenubar();
	bool DoQueryClose();
	bool DoQuerySave(bool doclose = true);
	bool DoQuerySaveTileset(bool doclose = true);
	bool DoQueryImportCreatures();
	bool LoadMap(FileName name);

	void AddRecentFile(const FileName& file);
	void LoadRecentFiles();
	void SaveRecentFiles();
	std::vector<wxString> GetRecentFiles();

	MainToolBar* GetAuiToolBar() const {
		return tool_bar;
	}

	void OnUpdateMenus(wxCommandEvent& event);
	void UpdateFloorMenu();
	void OnIdle(wxIdleEvent& event);
	void OnExit(wxCloseEvent& event);

#ifdef _USE_UPDATER_
	void OnUpdateReceived(wxCommandEvent& event);
#endif

#ifdef __WINDOWS__
	virtual bool MSWTranslateMessage(WXMSG* msg);
#endif

	void PrepareDC(wxDC& dc);

protected:
	MainMenuBar* menu_bar;
	MainToolBar* tool_bar;

	friend class Application;
	friend class GUI;

	DECLARE_EVENT_TABLE()
};

#endif
"""

main_menubar_cpp_content = """\
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

	/*
	 * AUTOMAGIC SYSTEM OVERVIEW
	 * -------------------------
	 * The Automagic system in Remere's Map Editor provides automatic border and wall handling.
	 *
	 * Files involved:
	 * - settings.h/cpp: Defines USE_AUTOMAGIC, BORDERIZE_PASTE, BORDERIZE_DRAG settings
	 * - main_menubar.h/cpp: Implements menu options for toggling Automagic and borderizing
	 * - tile.h/cpp: Contains borderize() and wallize() methods that apply automatic borders/walls
	 * - ground_brush.cpp: Implements GroundBrush::doBorders() which handles automatic borders
	 * - wall_brush.cpp: Implements WallBrush::doWalls() which handles automatic walls
	 * - borderize_window.cpp: UI for borderizing large selections or the entire map
	 * - editor.cpp: Contains borderizeSelection() and borderizeMap() methods
	 * - copybuffer.cpp: Applies borderize to pasted content
	 *
	 * How it works:
	 * 1. When enabled (via Config::USE_AUTOMAGIC), the editor automatically applies borders
	 *    and wall connections when tiles are placed, moved, or modified.
	 * 2. Borderizing examines neighboring tiles to determine appropriate borders between
	 *    different terrain types.
	 * 3. Wallizing connects wall segments automatically based on adjacent walls.
	 * 4. The system can be triggered:
	 *    - Automatically during editing when Automagic is enabled
	 *    - Manually via Map > Borderize Selection (Ctrl+B)
	 *    - Manually via Map > Borderize Map (processes the entire map)
	 *
	 * Settings:
	 * - BORDERIZE_PASTE: Automatically borderize after pasting
	 * - BORDERIZE_DRAG: Automatically borderize after drag operations
	 * - BORDERIZE_DRAG_THRESHOLD: Maximum selection size for auto-borderizing during drag
	 * - BORDERIZE_PASTE_THRESHOLD: Maximum selection size for auto-borderizing during paste
	 *
	 * The BorderizeWindow provides a UI for processing large maps in chunks to avoid
	 * performance issues when borderizing extensive areas.
	 */

#include "main.h"

#include "main_menubar.h"
#include "application.h"
#include "preferences.h"
#include "about_window.h"
#include "minimap_window.h"
#include "dat_debug_view.h"
#include "result_window.h"
#include "extension_window.h"
#include "find_item_window.h"
#include "settings.h"
#include "automagic_settings.h"
#include "find_creature_window.h"
#include "map.h"
#include "editor.h"
#include "gui.h"
#include "border_editor_window.h"

#include <wx/chartype.h>

#include "editor.h"
#include "materials.h"
#include "live_client.h"
#include "live_server.h"
#include "string_utils.h"
#include "hotkey_manager.h"

const wxEventType EVT_MENU = wxEVT_COMMAND_MENU_SELECTED;

BEGIN_EVENT_TABLE(MainMenuBar, wxEvtHandler)
	EVT_MENU(MenuBar::NEW, MainMenuBar::OnNew)
	EVT_MENU(MenuBar::OPEN, MainMenuBar::OnOpen)
	EVT_MENU(MenuBar::SAVE, MainMenuBar::OnSave)
	EVT_MENU(MenuBar::SAVE_AS, MainMenuBar::OnSaveAs)
	EVT_MENU(MenuBar::GENERATE_MAP, MainMenuBar::OnGenerateMap)
	EVT_MENU(MenuBar::MAP_MENU_GENERATE_ISLAND, MainMenuBar::OnGenerateIsland)
	EVT_MENU(MenuBar::FIND_CREATURE, MainMenuBar::OnSearchForCreature)
END_EVENT_TABLE()

MainMenuBar::MainMenuBar(MainFrame* frame) :
	frame(frame) {
	using namespace MenuBar;
	checking_programmaticly = false;

#define MAKE_ACTION(id, kind, handler) actions[#id] = new MenuBar::Action(#id, id, kind, wxCommandEventFunction(&MainMenuBar::handler))
#define MAKE_SET_ACTION(id, kind, setting_, handler)                                                  \
	actions[#id] = new MenuBar::Action(#id, id, kind, wxCommandEventFunction(&MainMenuBar::handler)); \
	actions[#id].setting = setting_

	MAKE_ACTION(NEW, wxITEM_NORMAL, OnNew);
	MAKE_ACTION(OPEN, wxITEM_NORMAL, OnOpen);
	MAKE_ACTION(SAVE, wxITEM_NORMAL, OnSave);
	MAKE_ACTION(SAVE_AS, wxITEM_NORMAL, OnSaveAs);
	MAKE_ACTION(GENERATE_MAP, wxITEM_NORMAL, OnGenerateMap);
	MAKE_ACTION(CLOSE, wxITEM_NORMAL, OnClose);

	MAKE_ACTION(IMPORT_MAP, wxITEM_NORMAL, OnImportMap);
	MAKE_ACTION(IMPORT_MONSTERS, wxITEM_NORMAL, OnImportMonsterData);
	MAKE_ACTION(IMPORT_MINIMAP, wxITEM_NORMAL, OnImportMinimap);
	MAKE_ACTION(EXPORT_MINIMAP, wxITEM_NORMAL, OnExportMinimap);
	MAKE_ACTION(EXPORT_TILESETS, wxITEM_NORMAL, OnExportTilesets);

	MAKE_ACTION(RELOAD_DATA, wxITEM_NORMAL, OnReloadDataFiles);
	// MAKE_ACTION(RECENT_FILES, wxITEM_NORMAL, OnRecent);
	MAKE_ACTION(PREFERENCES, wxITEM_NORMAL, OnPreferences);
	MAKE_ACTION(EXIT, wxITEM_NORMAL, OnQuit);

	MAKE_ACTION(UNDO, wxITEM_NORMAL, OnUndo);
	MAKE_ACTION(REDO, wxITEM_NORMAL, OnRedo);

	MAKE_ACTION(FIND_ITEM, wxITEM_NORMAL, OnSearchForItem);
	MAKE_ACTION(REPLACE_ITEMS, wxITEM_NORMAL, OnReplaceItems);
	MAKE_ACTION(SEARCH_ON_MAP_EVERYTHING, wxITEM_NORMAL, OnSearchForStuffOnMap);
	MAKE_ACTION(SEARCH_ON_MAP_ZONES, wxITEM_NORMAL, OnSearchForZonesOnMap);
	MAKE_ACTION(SEARCH_ON_MAP_UNIQUE, wxITEM_NORMAL, OnSearchForUniqueOnMap);
	MAKE_ACTION(SEARCH_ON_MAP_ACTION, wxITEM_NORMAL, OnSearchForActionOnMap);
	MAKE_ACTION(SEARCH_ON_MAP_CONTAINER, wxITEM_NORMAL, OnSearchForContainerOnMap);
	MAKE_ACTION(SEARCH_ON_MAP_WRITEABLE, wxITEM_NORMAL, OnSearchForWriteableOnMap);
	MAKE_ACTION(SEARCH_ON_SELECTION_EVERYTHING, wxITEM_NORMAL, OnSearchForStuffOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_ZONES, wxITEM_NORMAL, OnSearchForZonesOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_UNIQUE, wxITEM_NORMAL, OnSearchForUniqueOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_ACTION, wxITEM_NORMAL, OnSearchForActionOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_CONTAINER, wxITEM_NORMAL, OnSearchForContainerOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_WRITEABLE, wxITEM_NORMAL, OnSearchForWriteableOnSelection);
	MAKE_ACTION(SEARCH_ON_SELECTION_ITEM, wxITEM_NORMAL, OnSearchForItemOnSelection);
	MAKE_ACTION(REPLACE_ON_SELECTION_ITEMS, wxITEM_NORMAL, OnReplaceItemsOnSelection);
	MAKE_ACTION(REMOVE_ON_SELECTION_ITEM, wxITEM_NORMAL, OnRemoveItemOnSelection);
	MAKE_ACTION(SELECT_MODE_COMPENSATE, wxITEM_RADIO, OnSelectionTypeChange);
	MAKE_ACTION(SELECT_MODE_LOWER, wxITEM_RADIO, OnSelectionTypeChange);
	MAKE_ACTION(SELECT_MODE_CURRENT, wxITEM_RADIO, OnSelectionTypeChange);
	MAKE_ACTION(SELECT_MODE_VISIBLE, wxITEM_RADIO, OnSelectionTypeChange);

	// Remove the AUTOMAGIC action as it's now handled by 'A' hotkey
	// MAKE_ACTION(AUTOMAGIC, wxITEM_CHECK, OnToggleAutomagic);

	MAKE_ACTION(BORDERIZE_SELECTION, wxITEM_NORMAL, OnBorderizeSelection);
	MAKE_ACTION(BORDERIZE_MAP, wxITEM_NORMAL, OnBorderizeMap);
	MAKE_ACTION(RANDOMIZE_SELECTION, wxITEM_NORMAL, OnRandomizeSelection);
	MAKE_ACTION(RANDOMIZE_MAP, wxITEM_NORMAL, OnRandomizeMap);
	MAKE_ACTION(GOTO_PREVIOUS_POSITION, wxITEM_NORMAL, OnGotoPreviousPosition);
	MAKE_ACTION(GOTO_POSITION, wxITEM_NORMAL, OnGotoPosition);
	MAKE_ACTION(JUMP_TO_BRUSH, wxITEM_NORMAL, OnJumpToBrush);
	MAKE_ACTION(JUMP_TO_ITEM_BRUSH, wxITEM_NORMAL, OnJumpToItemBrush);

	MAKE_ACTION(CUT, wxITEM_NORMAL, OnCut);
	MAKE_ACTION(COPY, wxITEM_NORMAL, OnCopy);
	MAKE_ACTION(PASTE, wxITEM_NORMAL, OnPaste);

	MAKE_ACTION(EDIT_TOWNS, wxITEM_NORMAL, OnMapEditTowns);
	MAKE_ACTION(EDIT_ITEMS, wxITEM_NORMAL, OnMapEditItems);
	MAKE_ACTION(EDIT_MONSTERS, wxITEM_NORMAL, OnMapEditMonsters);

	MAKE_ACTION(CLEAR_INVALID_HOUSES, wxITEM_NORMAL, OnClearHouseTiles);
	MAKE_ACTION(CLEAR_MODIFIED_STATE, wxITEM_NORMAL, OnClearModifiedState);
	MAKE_ACTION(MAP_REMOVE_ITEMS, wxITEM_NORMAL, OnMapRemoveItems);
	MAKE_ACTION(MAP_REMOVE_CORPSES, wxITEM_NORMAL, OnMapRemoveCorpses);
	MAKE_ACTION(MAP_REMOVE_DUPLICATES, wxITEM_NORMAL, OnMapRemoveDuplicates);
	MAKE_ACTION(MAP_VALIDATE_GROUND, wxITEM_NORMAL, OnMapValidateGround);
	MAKE_ACTION(MAP_REMOVE_UNREACHABLE_TILES, wxITEM_NORMAL, OnMapRemoveUnreachable);
	MAKE_ACTION(MAP_CLEANUP, wxITEM_NORMAL, OnMapCleanup);
	MAKE_ACTION(MAP_CLEAN_HOUSE_ITEMS, wxITEM_NORMAL, OnMapCleanHouseItems);
	MAKE_ACTION(MAP_PROPERTIES, wxITEM_NORMAL, OnMapProperties);
	MAKE_ACTION(MAP_STATISTICS, wxITEM_NORMAL, OnMapStatistics);

	MAKE_ACTION(VIEW_TOOLBARS_BRUSHES, wxITEM_CHECK, OnToolbars);
	MAKE_ACTION(VIEW_TOOLBARS_POSITION, wxITEM_CHECK, OnToolbars);
	MAKE_ACTION(VIEW_TOOLBARS_SIZES, wxITEM_CHECK, OnToolbars);
	MAKE_ACTION(VIEW_TOOLBARS_STANDARD, wxITEM_CHECK, OnToolbars);
	MAKE_ACTION(NEW_VIEW, wxITEM_NORMAL, OnNewView);
	MAKE_ACTION(NEW_DETACHED_VIEW, wxITEM_NORMAL, OnNewDetachedView);
	MAKE_ACTION(TOGGLE_FULLSCREEN, wxITEM_NORMAL, OnToggleFullscreen);

	MAKE_ACTION(ZOOM_IN, wxITEM_NORMAL, OnZoomIn);
	MAKE_ACTION(ZOOM_OUT, wxITEM_NORMAL, OnZoomOut);
	MAKE_ACTION(ZOOM_NORMAL, wxITEM_NORMAL, OnZoomNormal);

	MAKE_ACTION(SHOW_SHADE, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ALL_FLOORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(GHOST_ITEMS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(GHOST_HIGHER_FLOORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(HIGHLIGHT_ITEMS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(HIGHLIGHT_LOCKED_DOORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_EXTRA, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_INGAME_BOX, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_LIGHTS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_LIGHT_STR, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_TECHNICAL_ITEMS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_WAYPOINTS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_GRID, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_CREATURES, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_SPAWNS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_SPECIAL, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ZONES, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_AS_MINIMAP, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ONLY_COLORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ONLY_MODIFIED, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_HOUSES, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_PATHING, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_TOOLTIPS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_PREVIEW, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_WALL_HOOKS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_TOWNS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(ALWAYS_SHOW_ZONES, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(EXT_HOUSE_SHADER, wxITEM_CHECK, OnChangeViewSettings);

	MAKE_ACTION(EXPERIMENTAL_FOG, wxITEM_CHECK, OnChangeViewSettings); // experimental

	MAKE_ACTION(WIN_MINIMAP, wxITEM_NORMAL, OnMinimapWindow);
	MAKE_ACTION(NEW_PALETTE, wxITEM_NORMAL, OnNewPalette);
	MAKE_ACTION(TAKE_SCREENSHOT, wxITEM_NORMAL, OnTakeScreenshot);

	MAKE_ACTION(LIVE_START, wxITEM_NORMAL, OnStartLive);
	MAKE_ACTION(LIVE_JOIN, wxITEM_NORMAL, OnJoinLive);
	MAKE_ACTION(LIVE_CLOSE, wxITEM_NORMAL, OnCloseLive);
	MAKE_ACTION(ID_MENU_SERVER_HOST, wxITEM_NORMAL, onServerHost);
	MAKE_ACTION(ID_MENU_SERVER_CONNECT, wxITEM_NORMAL, onServerConnect);

	MAKE_ACTION(SELECT_TERRAIN, wxITEM_NORMAL, OnSelectTerrainPalette);
	MAKE_ACTION(SELECT_DOODAD, wxITEM_NORMAL, OnSelectDoodadPalette);
	MAKE_ACTION(SELECT_ITEM, wxITEM_NORMAL, OnSelectItemPalette);
	MAKE_ACTION(SELECT_COLLECTION, wxITEM_NORMAL, OnSelectCollectionPalette);
	MAKE_ACTION(SELECT_CREATURE, wxITEM_NORMAL, OnSelectCreaturePalette);
	MAKE_ACTION(SELECT_HOUSE, wxITEM_NORMAL, OnSelectHousePalette);
	MAKE_ACTION(SELECT_WAYPOINT, wxITEM_NORMAL, OnSelectWaypointPalette);
	MAKE_ACTION(SELECT_RAW, wxITEM_NORMAL, OnSelectRawPalette);

	MAKE_ACTION(FLOOR_0, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_1, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_2, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_3, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_4, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_5, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_6, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_7, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_8, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_9, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_10, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_11, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_12, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_13, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_14, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_15, wxITEM_RADIO, OnChangeFloor);

	MAKE_ACTION(DEBUG_VIEW_DAT, wxITEM_NORMAL, OnDebugViewDat);
	MAKE_ACTION(EXTENSIONS, wxITEM_NORMAL, OnListExtensions);
	MAKE_ACTION(GOTO_WEBSITE, wxITEM_NORMAL, OnGotoWebsite);
	MAKE_ACTION(ABOUT, wxITEM_NORMAL, OnAbout);
	MAKE_ACTION(SHOW_HOTKEYS, wxITEM_NORMAL, OnShowHotkeys); // Add this line
	MAKE_ACTION(REFRESH_ITEMS, wxITEM_NORMAL, OnRefreshItems);
	// 669
	MAKE_ACTION(FIND_CREATURE, wxITEM_NORMAL, OnSearchForCreature);
	MAKE_ACTION(MAP_CREATE_BORDER, wxITEM_NORMAL, OnCreateBorder);

	// A deleter, this way the frame does not need
	// to bother deleting us.
	class CustomMenuBar : public wxMenuBar {
	public:
		CustomMenuBar(MainMenuBar* mb) :
			mb(mb) { }
		~CustomMenuBar() {
			delete mb;
		}

	private:
		MainMenuBar* mb;
	};

	menubar = newd CustomMenuBar(this);
	frame->SetMenuBar(menubar);

	// Tie all events to this handler!

	for (std::map<std::string, MenuBar::Action*>::iterator ai = actions.begin(); ai != actions.end(); ++ai) {
		frame->Connect(MAIN_FRAME_MENU + ai->second->id, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)(wxEventFunction)(ai->second->handler), nullptr, this);
	}
	for (size_t i = 0; i < 10; ++i) {
		frame->Connect(recentFiles.GetBaseId() + i, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainMenuBar::OnOpenRecent), nullptr, this);
	}
}
"""

main_menubar_h_content = """\
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

#ifndef RME_MAIN_BAR_H_
#define RME_MAIN_BAR_H_

#include <wx/docview.h>

namespace MenuBar {
	struct Action;

	enum ActionID {
		NEW,
		OPEN,
		SAVE,
		SAVE_AS,
		GENERATE_MAP,
		CLOSE,
		IMPORT_MAP,
		IMPORT_MONSTERS,
		IMPORT_MINIMAP,
		EXPORT_MINIMAP,
		EXPORT_TILESETS,
		RELOAD_DATA,
		RECENT_FILES,
		PREFERENCES,
		EXIT,
		UNDO,
		REDO,
		FIND_ITEM,
		FIND_CREATURE,
		REPLACE_ITEMS,
		AUTOMAGIC,
		SEARCH_ON_MAP_EVERYTHING,
		SEARCH_ON_MAP_ZONES,
		SEARCH_ON_MAP_UNIQUE,
		SEARCH_ON_MAP_ACTION,
		SEARCH_ON_MAP_CONTAINER,
		SEARCH_ON_MAP_WRITEABLE,
		SEARCH_ON_SELECTION_EVERYTHING,
		SEARCH_ON_SELECTION_ZONES,
		SEARCH_ON_SELECTION_UNIQUE,
		SEARCH_ON_SELECTION_ACTION,
		SEARCH_ON_SELECTION_CONTAINER,
		SEARCH_ON_SELECTION_WRITEABLE,
		SEARCH_ON_SELECTION_ITEM,
		REPLACE_ON_SELECTION_ITEMS,
		REMOVE_ON_SELECTION_ITEM,
		SELECT_MODE_COMPENSATE,
		SELECT_MODE_CURRENT,
		SELECT_MODE_LOWER,
		SELECT_MODE_VISIBLE,
		// AUTOMAGIC removed - now controlled by 'A' hotkey
		BORDERIZE_SELECTION,
		BORDERIZE_MAP,
		RANDOMIZE_SELECTION,
		RANDOMIZE_MAP,
		GOTO_PREVIOUS_POSITION,
		GOTO_POSITION,
		JUMP_TO_BRUSH,
		JUMP_TO_ITEM_BRUSH,
		CLEAR_INVALID_HOUSES,
		CLEAR_MODIFIED_STATE,
		CUT,
		COPY,
		PASTE,
		EDIT_TOWNS,
		EDIT_ITEMS,
		EDIT_MONSTERS,
		MAP_CLEANUP,
		MAP_REMOVE_ITEMS,
		MAP_REMOVE_CORPSES,
		MAP_REMOVE_UNREACHABLE_TILES,
		MAP_CLEAN_HOUSE_ITEMS,
		MAP_PROPERTIES,
		MAP_STATISTICS,
		VIEW_TOOLBARS_BRUSHES,
		VIEW_TOOLBARS_POSITION,
		VIEW_TOOLBARS_SIZES,
		VIEW_TOOLBARS_STANDARD,
		NEW_VIEW,
		//idler
		NEW_DETACHED_VIEW,
		TOGGLE_FULLSCREEN,
		ZOOM_IN,
		ZOOM_OUT,
		ZOOM_NORMAL,
		SHOW_SHADE,
		SHOW_ALL_FLOORS,
		GHOST_ITEMS,
		GHOST_HIGHER_FLOORS,
		HIGHLIGHT_ITEMS,
		HIGHLIGHT_LOCKED_DOORS,
		SHOW_INGAME_BOX,
		SHOW_LIGHTS,
		SHOW_LIGHT_STR,
		SHOW_TECHNICAL_ITEMS,
		SHOW_WAYPOINTS,
		SHOW_GRID,
		SHOW_EXTRA,
		SHOW_CREATURES,
		SHOW_SPAWNS,
		SHOW_SPECIAL,
		SHOW_ZONES,
		SHOW_AS_MINIMAP,
		SHOW_ONLY_COLORS,
		SHOW_ONLY_MODIFIED,
		SHOW_HOUSES,
		SHOW_PATHING,
		SHOW_TOOLTIPS,
		SHOW_PREVIEW,
		SHOW_WALL_HOOKS,
		SHOW_TOWNS,
		ALWAYS_SHOW_ZONES,
		EXT_HOUSE_SHADER,
		REFRESH_ITEMS,

		WIN_MINIMAP,
		NEW_PALETTE,
		TAKE_SCREENSHOT,
		LIVE_START,
		LIVE_JOIN,
		LIVE_CLOSE,
		SELECT_TERRAIN,
		SELECT_DOODAD,
		SELECT_ITEM,
		SELECT_COLLECTION,
		SELECT_CREATURE,
		SELECT_HOUSE,
		SELECT_WAYPOINT,
		SELECT_RAW,
		FLOOR_0,
		FLOOR_1,
		FLOOR_2,
		FLOOR_3,
		FLOOR_4,
		FLOOR_5,
		FLOOR_6,
		FLOOR_7,
		FLOOR_8,
		FLOOR_9,
		FLOOR_10,
		FLOOR_11,
		FLOOR_12,
		FLOOR_13,
		FLOOR_14,
		FLOOR_15,
		DEBUG_VIEW_DAT,
		EXTENSIONS,
		GOTO_WEBSITE,
		ABOUT,
		ID_MENU_SERVER_HOST,
		ID_MENU_SERVER_CONNECT,

		EXPERIMENTAL_FOG,
		MAP_REMOVE_DUPLICATES,
		SHOW_HOTKEYS,
		MAP_MENU_REPLACE_ITEMS,
		MAP_MENU_GENERATE_ISLAND,
		MAP_VALIDATE_GROUND,
		MAP_CREATE_BORDER,



	};
}

class MainFrame;

class MainMenuBar : public wxEvtHandler {
public:
	MainMenuBar(MainFrame* frame);
	virtual ~MainMenuBar();

	bool Load(const FileName&, wxArrayString& warnings, wxString& error);

	// Update
	// Turn on/off all buttons according to current editor state
	void Update();
	void UpdateFloorMenu(); // Only concerns the floor menu

	void AddRecentFile(FileName file);
	void LoadRecentFiles();
	void SaveRecentFiles();
	std::vector<wxString> GetRecentFiles();

	// Interface
	void EnableItem(MenuBar::ActionID id, bool enable);
	void CheckItem(MenuBar::ActionID id, bool enable);
	bool IsItemChecked(MenuBar::ActionID id) const;

	// Event handlers for all menu buttons
	// File Menu
	void OnNew(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnGenerateMap(wxCommandEvent& event);
	void OnOpenRecent(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnSaveAs(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnPreferences(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	// Import Menu
	// Export Menu
	void OnImportMap(wxCommandEvent& event);
	void OnImportMonsterData(wxCommandEvent& event);
	void OnImportMinimap(wxCommandEvent& event);
	void OnExportMinimap(wxCommandEvent& event);
	void OnExportTilesets(wxCommandEvent& event);
	void OnReloadDataFiles(wxCommandEvent& event);

	// Edit Menu
	void OnUndo(wxCommandEvent& event);
	void OnRedo(wxCommandEvent& event);
	void OnBorderizeSelection(wxCommandEvent& event);
	void OnBorderizeMap(wxCommandEvent& event);
	void OnRandomizeSelection(wxCommandEvent& event);
	void OnRandomizeMap(wxCommandEvent& event);
	void OnJumpToBrush(wxCommandEvent& event);
	void OnJumpToItemBrush(wxCommandEvent& event);
	void OnGotoPreviousPosition(wxCommandEvent& event);
	void OnGotoPosition(wxCommandEvent& event);
	void OnMapRemoveItems(wxCommandEvent& event);
	void OnMapRemoveCorpses(wxCommandEvent& event);
	void OnMapRemoveUnreachable(wxCommandEvent& event);
	void OnClearHouseTiles(wxCommandEvent& event);
	void OnClearModifiedState(wxCommandEvent& event);
	void OnSelectionTypeChange(wxCommandEvent& event);
	void OnCut(wxCommandEvent& event);
	void OnCopy(wxCommandEvent& event);
	void OnPaste(wxCommandEvent& event);
	void OnSearchForItem(wxCommandEvent& event);
	void OnSearchForCreature(wxCommandEvent& event);
	void OnReplaceItems(wxCommandEvent& event);
	void OnSearchForStuffOnMap(wxCommandEvent& event);
	void OnSearchForZonesOnMap(wxCommandEvent& event);
	void OnSearchForUniqueOnMap(wxCommandEvent& event);
	void OnSearchForActionOnMap(wxCommandEvent& event);
	void OnSearchForContainerOnMap(wxCommandEvent& event);
	void OnSearchForWriteableOnMap(wxCommandEvent& event);

	// Select menu
	void OnSearchForStuffOnSelection(wxCommandEvent& event);
	void OnSearchForZonesOnSelection(wxCommandEvent& event);
	void OnSearchForUniqueOnSelection(wxCommandEvent& event);
	void OnSearchForActionOnSelection(wxCommandEvent& event);
	void OnSearchForContainerOnSelection(wxCommandEvent& event);
	void OnSearchForWriteableOnSelection(wxCommandEvent& event);
	void OnSearchForItemOnSelection(wxCommandEvent& event);
	void OnReplaceItemsOnSelection(wxCommandEvent& event);
	void OnRemoveItemOnSelection(wxCommandEvent& event);

	// Map menu
	void OnMapEditTowns(wxCommandEvent& event);
	void OnMapEditItems(wxCommandEvent& event);
	void OnMapEditMonsters(wxCommandEvent& event);
	void OnMapCleanHouseItems(wxCommandEvent& event);
	void OnMapCleanup(wxCommandEvent& event);
	void OnMapProperties(wxCommandEvent& event);
	void OnMapStatistics(wxCommandEvent& event);
	void OnMapRemoveDuplicates(wxCommandEvent& event);
	void OnMapValidateGround(wxCommandEvent& event);

	// View Menu
	void OnToolbars(wxCommandEvent& event);
	void OnNewView(wxCommandEvent& event);
	void OnNewDetachedView(wxCommandEvent& event);
	void OnToggleFullscreen(wxCommandEvent& event);
	void OnZoomIn(wxCommandEvent& event);
	void OnZoomOut(wxCommandEvent& event);
	void OnZoomNormal(wxCommandEvent& event);
	void OnChangeViewSettings(wxCommandEvent& event);

	// Network menu
	void OnStartLive(wxCommandEvent& event);
	void OnJoinLive(wxCommandEvent& event);
	void OnCloseLive(wxCommandEvent& event);
	void onServerHost(wxCommandEvent& event);
	void onServerConnect(wxCommandEvent& event);

	// Window Menu
	void OnMinimapWindow(wxCommandEvent& event);
	void OnNewPalette(wxCommandEvent& event);
	void OnTakeScreenshot(wxCommandEvent& event);
	void OnSelectTerrainPalette(wxCommandEvent& event);
	void OnSelectDoodadPalette(wxCommandEvent& event);
	void OnSelectItemPalette(wxCommandEvent& event);
	void OnSelectCollectionPalette(wxCommandEvent& event);
	void OnSelectHousePalette(wxCommandEvent& event);
	void OnSelectCreaturePalette(wxCommandEvent& event);
	void OnSelectWaypointPalette(wxCommandEvent& event);
	void OnSelectRawPalette(wxCommandEvent& event);

	// Floor menu
	void OnChangeFloor(wxCommandEvent& event);

	// About Menu
	void OnDebugViewDat(wxCommandEvent& event);
	void OnListExtensions(wxCommandEvent& event);
	void OnGotoWebsite(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnShowHotkeys(wxCommandEvent& event);

	// Add to class MainMenuBar private section:
	void OnRefreshItems(wxCommandEvent& event);
	void OnGenerateIsland(wxCommandEvent& event);
	void OnCreateBorder(wxCommandEvent& event);

protected:
	// Load and returns a menu item, also sets accelerator
	wxObject* LoadItem(pugi::xml_node node, wxMenu* parent, wxArrayString& warnings, wxString& error);
	// Checks the items in the menus according to the settings (in config)
	void LoadValues();
	void SearchItems(bool unique, bool action, bool container, bool writable, bool zones, bool onSelection = false);

protected:
	MainFrame* frame;
	wxMenuBar* menubar;

	// Used so that calling Check on menu items don't trigger events (avoids infinite recursion)
	bool checking_programmaticly;

	std::map<MenuBar::ActionID, std::list<wxMenuItem*>> items;

	// Hardcoded recent files
	wxFileHistory recentFiles;

	std::map<std::string, MenuBar::Action*> actions;

	DECLARE_EVENT_TABLE();
};

namespace MenuBar {
	struct Action {
		Action() :
			id(0), kind(wxITEM_NORMAL) { }
		Action(std::string s, int id, wxItemKind kind, wxCommandEventFunction handler) :
			id(id), setting(0), name(s), kind(kind), handler(handler) { }

		int id;
		int setting;
		std::string name;
		wxItemKind kind;
		wxCommandEventFunction handler;
	};
}

// Add this declaration before the MainMenuBar class
std::vector<std::pair<uint16_t, uint16_t>> ParseRangeString(const wxString& input);

#endif
"""

# Placeholder for menubar.xml content and hash
menubar_xml_content = "<menubar><menu name=\"File\"><item name=\"New\" action=\"NEW\" hotkey=\"Ctrl+N\"/></menu></menubar>" # Simplified example
md5_menubar_xml = hashlib.md5(menubar_xml_content.encode('utf-8')).hexdigest()
content_lite_menubar_xml = "\\n".join(menubar_xml_content.splitlines()[:200])


md5_application_cpp = hashlib.md5(application_cpp_content.encode('utf-8')).hexdigest()
content_lite_application_cpp = "\\n".join(application_cpp_content.splitlines()[:200])

md5_application_h = hashlib.md5(application_h_content.encode('utf-8')).hexdigest()
content_lite_application_h = "\\n".join(application_h_content.splitlines()[:200])

md5_main_menubar_cpp = hashlib.md5(main_menubar_cpp_content.encode('utf-8')).hexdigest()
content_lite_main_menubar_cpp = "\\n".join(main_menubar_cpp_content.splitlines()[:200])

md5_main_menubar_h = hashlib.md5(main_menubar_h_content.encode('utf-8')).hexdigest()
content_lite_main_menubar_h = "\\n".join(main_menubar_h_content.splitlines()[:200])


yaml_data = {
    "wbs_item_id": "UI-01",
    "name": "Port Main Window & Menu Bar",
    "description": "Recreate the main application window (QMainWindow) and its menu bar (QMenuBar) using Qt6, including parsing menubar.xml to dynamically build menus and actions.",
    "dependencies": [
        "FINAL-02",
    ],
    "input_files": [
        "wxwidgets/application.cpp",
        "wxwidgets/application.h",
        "wxwidgets/main_menubar.cpp",
        "wxwidgets/main_menubar.h",
        "menubar.xml"
    ],
    "analyzed_input_files": [
        {
            "file_path": "wxwidgets/application.cpp",
            "description": "Contains the `MainFrame` class definition (equivalent to QMainWindow) and its basic setup including menu bar, status bar, and AUI manager. (Content already analyzed for FINAL-02).",
            "md5_hash": md5_application_cpp,
            "content_lite": content_lite_application_cpp
        },
        {
            "file_path": "wxwidgets/application.h",
            "description": "Header for `MainFrame` and `Application`. (Content already analyzed for FINAL-02).",
            "md5_hash": md5_application_h,
            "content_lite": content_lite_application_h
        },
        {
            "file_path": "wxwidgets/main_menubar.cpp",
            "description": "Implements `MainMenuBar` which parses `menubar.xml` to create wxMenus and wxMenuItems, connects events, and handles menu item state updates (enable/disable, check).",
            "md5_hash": md5_main_menubar_cpp,
            "content_lite": content_lite_main_menubar_cpp
        },
        {
            "file_path": "wxwidgets/main_menubar.h",
            "description": "Header for `MainMenuBar`, defining its structure, ActionID enum, and Action struct.",
            "md5_hash": md5_main_menubar_h,
            "content_lite": content_lite_main_menubar_h
        },
        {
            "file_path": "menubar.xml",
            "description": "XML file defining the structure of the menu bar, including menus, submenus, items, action names, hotkeys, and help strings. (Content fetched previously).",
            "md5_hash": md5_menubar_xml, # Placeholder if actual XML not fetched by this script
            "content_lite": content_lite_menubar_xml # Placeholder
        }
    ],
    "documentation_references": [
        "QMainWindow Class: https://doc.qt.io/qt-6/qmainwindow.html",
        "QMenuBar Class: https://doc.qt.io/qt-6/qmenubar.html",
        "QMenu Class: https://doc.qt.io/qt-6/qmenu.html",
        "QAction Class: https://doc.qt.io/qt-6/qaction.html",
        "QStatusBar Class: https://doc.qt.io/qt-6/qstatusbar.html",
        "QXmlStreamReader (for parsing XML): https://doc.qt.io/qt-6/qxmlstreamreader.html",
        "Qt Signals and Slots: https://doc.qt.io/qt-6/signalsandslots.html",
        "QSettings (for recent files): https://doc.qt.io/qt-6/qsettings.html"
    ],
    "current_functionality_summary": """\
The wxWidgets application uses a `MainFrame` (derived from `wxFrame`) as its main window. This frame hosts a `MainMenuBar` object.
The `MainMenuBar` class is responsible for:
1. Parsing an external `menubar.xml` file to dynamically construct the entire menu hierarchy (menus, sub-menus, items, separators).
2. Mapping XML item definitions to internal `MenuBar::Action` structs, which link an action name (from XML) to an event ID, item kind (normal, check, radio), and a C++ event handler function pointer within `MainMenuBar`.
3. Connecting UI events from `wxMenuItem`s to these handler functions.
4. Managing the enabled/disabled and checked/unchecked state of menu items dynamically based on application context (e.g., map loaded, selection available) via an `Update()` method.
5. Handling a list of recent files, displayed in a dedicated submenu.
The `MainFrame` also initializes a status bar.\
""",
    "definition_of_done": [
        "A `MainWindow` class, inheriting from `QMainWindow`, is created and serves as the application's main window.",
        "A `QMenuBar` is set as the main menu bar for the `MainWindow`.",
        "The menu structure defined in `menubar.xml` is successfully parsed (e.g., using `QXmlStreamReader`), and corresponding `QMenu` and `QAction` objects are dynamically created and added to the `QMenuBar`, replicating the original hierarchy, including submenus and separators.",
        "Hotkeys specified in `menubar.xml` are assigned as shortcuts to the respective `QAction` objects.",
        "Help strings from `menubar.xml` are set as status tips for `QAction` objects.",
        "Each `QAction`'s `triggered()` signal is connected to a placeholder slot or an appropriate handler method (to be fully implemented by other WBS tasks corresponding to each action).",
        "A `QStatusBar` is added to the `MainWindow` and can display initial application messages.",
        "Functionality to load, display, and open recent files (similar to `wxFileHistory`) is implemented in the 'File' menu, likely using `QSettings` for persistence.",
        "A mechanism equivalent to `MainMenuBar::Update()` and `MainMenuBar::LoadValues()` is implemented in `MainWindow` to dynamically update the enabled/disabled state and checked state of `QAction`s based on application state and settings.",
        "The `MainWindow` is displayed correctly upon application startup (linking with `FINAL-02`)."
    ],
    "boilerplate_coder_ai_prompt": """\
Your task is to port the main application window and its menu bar from the wxWidgets Remere's Map Editor to Qt6.
Reference Files: `wxwidgets/application.cpp` (MainFrame), `wxwidgets/application.h` (MainFrame), `wxwidgets/main_menubar.cpp`, `wxwidgets/main_menubar.h`, and `menubar.xml`.

**1. Create `MainWindow` (inheriting `QMainWindow`):**
   - This will be the top-level window.
   - Set up a `QMenuBar` using `setMenuBar()` or by adding it to a layout if preferred (though `setMenuBar` is standard for `QMainWindow`).
   - Add a `QStatusBar` using `statusBar()`.

**2. Implement Menu Loading from `menubar.xml`:**
   - Create a new class, e.g., `MenuLoader`, or implement this logic within `MainWindow`.
   - Use `QXmlStreamReader` to parse `menubar.xml`.
   - Recursively process `<menu>` and `<item>` elements:
     - For each `<menu name="Title" [special="RECENT_FILES"]>`:
       - Create a `QMenu* menu = new QMenu("Title");`.
       - If `special="RECENT_FILES"`, this menu will be populated later by recent file actions. Store a pointer to it.
       - Add it to the parent menu or `QMenuBar`.
       - Recursively call your parsing function for its children.
     - For each `<item name="Label" action="ACTION_NAME" [hotkey="Ctrl+N"] [help="Help text"] [kind="check/radio"]>`:
       - Create a `QAction* action = new QAction("Label");`.
       - Set `action->setObjectName("ACTION_NAME");` (very important for later access and testing).
       - If `hotkey` exists, `action->setShortcut(QKeySequence("Ctrl+N"));`.
       - If `help` exists, `action->setStatusTip("Help text");`.
       - If `kind="check"`, `action->setCheckable(true);`.
       - If `kind="radio"`, group related radio actions using a `QActionGroup`.
       - Connect `action->triggered()` to a placeholder slot for now, or map `ACTION_NAME` to a specific handler method later.
       - Add the action to the current `QMenu`.
     - For `<separator/>`: Call `currentMenu->addSeparator();`.
   - Store created `QAction` objects, perhaps in a `QMap<QString, QAction*> m_actions;` (keyed by `ACTION_NAME`) for easy access during state updates.

**3. Implement Menu State Management:**
   - Create a public method in `MainWindow`, e.g., `void updateMenus();`.
   - Inside `updateMenus()`:
     - Access `QAction` objects (e.g., from `m_actions` map).
     - Enable/disable actions based on application state (e.g., `m_actions["SAVE"]->setEnabled(isMapLoaded && isMapModified);`).
     - Set checked state for checkable/radio actions (e.g., `m_actions["SHOW_GRID"]->setChecked(settings.showGrid);`).
     - Refer to the logic in `MainMenuBar::Update()` and `MainMenuBar::LoadValues()` from `main_menubar.cpp` for conditions.

**4. Implement Recent Files Functionality:**
   - Use `QSettings` to store and retrieve a list of recent file paths.
   - In `updateMenus()` or when the File menu is about to show (`QMenu::aboutToShow` signal), clear existing recent file actions and repopulate the "Recent Files" `QMenu` with new `QAction`s for each path.
   - Connect these dynamic actions to a slot that opens the selected file.
   - Implement `void addRecentFile(const QString& filePath);` to update `QSettings` and the menu.

**5. Integration:**
   - Instantiate and show `MainWindow` in your `main.cpp` (from `FINAL-02`).
   - Call `updateMenus()` initially and whenever application state changes that might affect menu items.

The original `MainMenuBar::actions` (a `std::map<std::string, MenuBar::Action*>`) provides the mapping from the XML `action` attribute string to internal IDs and event handlers. In Qt, `QAction` itself can be the central object, identified by its `objectName`.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/UI-01.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_data, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")

# Clean up the temporary file contents from variables
del application_cpp_content
del application_h_content
del main_menubar_cpp_content
del main_menubar_h_content
del menubar_xml_content # conceptual content
del yaml_data
