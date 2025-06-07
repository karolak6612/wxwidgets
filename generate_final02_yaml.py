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
			"Do you want the editor to automatically check for updates?\n"
			"It will connect to the internet if you choose yes.\n"
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

void Application::OnEventLoopEnter(wxEventLoopBase* loop) {

	// First startup?
	if (!m_startup) {
		return;
	}
	m_startup = false;

	// Don't try to create a map if we didn't load the client map.
	if (ClientVersion::getLatestVersion() == nullptr) {
		return;
	}

	// Open a map.
	if (m_file_to_open != wxEmptyString) {
		g_gui.LoadMap(FileName(m_file_to_open));
	} else if (!g_gui.IsWelcomeDialogShown() && g_gui.NewMap()) { // Open a new empty map
		// You generally don't want to save this map...
		g_gui.GetCurrentEditor()->map.clearChanges();
	}

	// Check when the URLs were last opened
	time_t currentTime = time(nullptr);
	time_t lastOpenTime = static_cast<time_t>(g_settings.getInteger(Config::LAST_WEBSITES_OPEN_TIME));

	// Calculate difference in days
	const time_t secondsPerDay = 60 * 60 * 24;
	const int daysToWait = 7;

	// If last open time is 0 (never opened) or if 7+ days have passed
	if (lastOpenTime == 0 || difftime(currentTime, lastOpenTime) > daysToWait * secondsPerDay) {
		// Open Discord and Idler.live URLs in the default browser
		::wxLaunchDefaultBrowser("https://discord.gg/FD2cYKBq5E", wxBROWSER_NEW_WINDOW);
		// Update the last open time
		g_settings.setInteger(Config::LAST_WEBSITES_OPEN_TIME, static_cast<int>(currentTime));
	}
}

void Application::MacOpenFiles(const wxArrayString& fileNames) {
	if (!fileNames.IsEmpty()) {
		g_gui.LoadMap(FileName(fileNames.Item(0)));
	}
}

void Application::FixVersionDiscrapencies() {
	// Here the registry should be fixed, if the version has been changed
	if (g_settings.getInteger(Config::VERSION_ID) < MAKE_VERSION_ID(1, 0, 5)) {
		g_settings.setInteger(Config::USE_MEMCACHED_SPRITES_TO_SAVE, 0);
	}

	if (g_settings.getInteger(Config::VERSION_ID) < __RME_VERSION_ID__ && ClientVersion::getLatestVersion() != nullptr) {
		g_settings.setInteger(Config::DEFAULT_CLIENT_VERSION, ClientVersion::getLatestVersion()->getID());
	}

	wxString ss = wxstr(g_settings.getString(Config::SCREENSHOT_DIRECTORY));
	if (ss.empty()) {
		ss = wxStandardPaths::Get().GetDocumentsDir();
#ifdef __WINDOWS__
		ss += "/My Pictures/RME/";
#endif
	}
	g_settings.setString(Config::SCREENSHOT_DIRECTORY, nstr(ss));

	// Set registry to newest version
	g_settings.setInteger(Config::VERSION_ID, __RME_VERSION_ID__);
}

void Application::Unload() {
	g_gui.CloseAllEditors();
	g_gui.UnloadVersion();
	g_gui.SaveHotkeys();
	g_gui.SavePerspective();
	g_gui.root->SaveRecentFiles();
	ClientVersion::saveVersions();
	ClientVersion::unloadVersions();
	g_settings.save(true);
	g_gui.root = nullptr;
}

int Application::OnExit() {
#ifdef _USE_PROCESS_COM
	wxDELETE(m_proc_server);
	wxDELETE(m_single_instance_checker);
#endif
	return 1;
}

void Application::OnFatalException() {
	// Log the fatal exception to a file
	wxDateTime now = wxDateTime::Now();
	wxString logFileName = wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() + "fatal_error_" +
		now.Format("%Y%m%d_%H%M%S") + ".log";

	// Create log directory if it doesn't exist
	wxFileName logDir(wxStandardPaths::Get().GetUserDataDir());
	if (!logDir.DirExists()) {
		wxMkdir(logDir.GetPath());
	}

	// Log details to a file
	std::ofstream logFile(logFileName.ToStdString(), std::ios::app);
	if (logFile.is_open()) {
		logFile << "Fatal exception occurred at " << now.FormatISOCombined() << std::endl;
		logFile << "RME version: " << __W_RME_VERSION__ << std::endl;
		logFile << "Please report this crash to the developers." << std::endl;
		logFile.close();
	}

	// Create and show a simple crash dialog
	wxString msg = "A fatal error has occurred in " + wxString(__W_RME_APPLICATION_NAME__) + ".\n\n";
	msg += "The application will now close. A log file has been created at:\n";
	msg += logFileName + "\n\n";
	msg += "Please report this error to the developers.";

	wxMessageBox(msg, "Fatal Error", wxICON_ERROR | wxOK);
}

bool Application::ParseCommandLineMap(wxString& fileName) {
	if (argc == 2) {
		// Check if it's a special command to force multiple instances
		if (wxString(argv[1]) == "-force-multi-instance") {
			g_settings.setInteger(Config::ONLY_ONE_INSTANCE, 0);
			return false;
		}
		fileName = wxString(argv[1]);
		return true;
	} else if (argc == 3) {
		if (argv[1] == "-ws") {
			g_settings.setInteger(Config::WELCOME_DIALOG, argv[2] == "1" ? 1 : 0);
		} else if (argv[1] == "-multi-instance") {
			// Allow forcing multiple instances via command line
			g_settings.setInteger(Config::ONLY_ONE_INSTANCE, argv[2] == "1" ? 0 : 1);
		}
	}
	return false;
}

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
	wxFrame((wxFrame*)nullptr, -1, title, pos, size, wxDEFAULT_FRAME_STYLE) {
	// Receive idle events
	SetExtraStyle(wxWS_EX_PROCESS_IDLE);

#if wxCHECK_VERSION(3, 1, 0) // 3.1.0 or higher
	// Make sure ShowFullScreen() uses the full screen API on macOS
	EnableFullScreenView(true);
#endif

	// Creates the file-dropdown menu
	menu_bar = newd MainMenuBar(this);
	wxArrayString warnings;
	wxString error;

	wxFileName filename;
	filename.Assign(g_gui.getFoundDataDirectory() + "menubar.xml");
	if (!filename.FileExists()) {
		filename = FileName(GUI::GetDataDirectory() + "menubar.xml");
	}

	if (!menu_bar->Load(filename, warnings, error)) {
		wxLogError(wxString() + "Could not load menubar.xml, editor will NOT be able to show its menu.\n");
	}

	wxStatusBar* statusbar = CreateStatusBar();
	statusbar->SetFieldsCount(4);
	SetStatusText(wxString("Welcome to ") << __W_RME_APPLICATION_NAME__ << " " << __W_RME_VERSION__);

	// Le sizer
	g_gui.aui_manager = newd wxAuiManager(this);
	g_gui.tabbook = newd MapTabbook(this, wxID_ANY);

	tool_bar = newd MainToolBar(this, g_gui.aui_manager);

	g_gui.aui_manager->AddPane(g_gui.tabbook, wxAuiPaneInfo().CenterPane().Floatable(false).CloseButton(false).PaneBorder(false));
	g_gui.aui_manager->Update();

	UpdateMenubar();

	// Apply dark mode if enabled
	if (g_darkMode.IsDarkModeEnabled()) {
		g_darkMode.ApplyTheme(this);
		g_darkMode.ApplyThemeToMainMenuBar(menu_bar);
		g_darkMode.ApplyThemeToMainToolBar(tool_bar);
		g_darkMode.ApplyThemeToStatusBar(GetStatusBar());
	}
}

MainFrame::~MainFrame() = default;

void MainFrame::OnIdle(wxIdleEvent& event) {
	g_gui.CheckAutoSave();
	event.Skip();
}

#ifdef _USE_UPDATER_
void MainFrame::OnUpdateReceived(wxCommandEvent& event) {
	std::string data = *(std::string*)event.GetClientData();
	delete (std::string*)event.GetClientData();
	size_t first_colon = data.find(':');
	size_t second_colon = data.find(':', first_colon + 1);

	if (first_colon == std::string::npos || second_colon == std::string::npos) {
		return;
	}

	std::string update = data.substr(0, first_colon);
	std::string verstr = data.substr(first_colon + 1, second_colon - first_colon - 1);
	std::string url = (second_colon == data.size() ? "" : data.substr(second_colon + 1));

	if (update == "yes") {
		int ret = g_gui.PopupDialog(
			"Update Notice",
			wxString("There is a newd update available (") << wxstr(verstr) << "). Do you want to go to the website and download it?",
			wxYES | wxNO,
			"I don't want any update notices",
			Config::AUTOCHECK_FOR_UPDATES
		);
		if (ret == wxID_YES) {
			::wxLaunchDefaultBrowser(wxstr(url), wxBROWSER_NEW_WINDOW);
		}
	}
}
#endif

void MainFrame::OnUpdateMenus(wxCommandEvent&) {
	UpdateMenubar();
	g_gui.UpdateMinimap(true);
	g_gui.UpdateTitle();
}

#ifdef __WINDOWS__
bool MainFrame::MSWTranslateMessage(WXMSG* msg) {
	if (g_gui.AreHotkeysEnabled()) {
		if (wxFrame::MSWTranslateMessage(msg)) {
			return true;
		}
	} else {
		if (wxWindow::MSWTranslateMessage(msg)) {
			return true;
		}
	}
	return false;
}
#endif

void MainFrame::UpdateMenubar() {
	menu_bar->Update();
	tool_bar->UpdateButtons();
}

bool MainFrame::DoQueryClose() {
	Editor* editor = g_gui.GetCurrentEditor();
	if (editor) {
		if (editor->IsLive()) {
			long ret = g_gui.PopupDialog(
				"Must Close Server",
				wxString("You are currently connected to a live server, to close this map the connection must be severed."),
				wxOK | wxCANCEL
			);
			if (ret == wxID_OK) {
				editor->CloseLiveServer();
			} else {
				return false;
			}
		}
	}
	return true;
}

bool MainFrame::DoQuerySaveTileset(bool doclose) {

	if (!g_materials.needSave()) {
		// skip dialog when there is nothing to save
		return true;
	}

	long ret = g_gui.PopupDialog(
		"Export tileset",
		"Do you want to export your tileset changes before exiting?",
		wxYES | wxNO | wxCANCEL
	);

	if (ret == wxID_NO) {
		// "no" - exit without saving
		return true;
	} else if (ret == wxID_CANCEL) {
		// "cancel" - just close the dialog
		return false;
	}

	// "yes" button was pressed, open tileset exporting dialog
	if (g_gui.GetCurrentEditor()) {
		ExportTilesetsWindow dlg(this, *g_gui.GetCurrentEditor());
		dlg.ShowModal();
		dlg.Destroy();
	}

	return !g_materials.needSave();
}

bool MainFrame::DoQuerySave(bool doclose) {
	if (!g_gui.IsEditorOpen()) {
		return true;
	}

	Editor* editor = g_gui.GetCurrentEditor();
	if (g_gui.HasDetachedViews(editor)) {
		wxString message = "This map has one or more detached views open.\n";
		message += "You must close all detached views before closing the map.";

		int choice = wxMessageBox(
			message,
			"Detached Views Open",
			wxOK | wxCANCEL | wxICON_EXCLAMATION
		);

		if (choice == wxOK) {
			// User chose to close detached views
			g_gui.CloseDetachedViews(editor);
		} else {
			// User canceled operation
			return false;
		}
	}

	if (!DoQuerySaveTileset()) {
		return false;
	}

	if (editor->IsLiveClient()) {
		long ret = g_gui.PopupDialog(
			"Disconnect",
			"Do you want to disconnect?",
			wxYES | wxNO
		);

		if (ret != wxID_YES) {
			return false;
		}

		editor->CloseLiveServer();
		return DoQuerySave(doclose);
	} else if (editor->IsLiveServer()) {
		long ret = g_gui.PopupDialog(
			"Shutdown",
			"Do you want to shut down the server? (any clients will be disconnected)",
			wxYES | wxNO
		);

		if (ret != wxID_YES) {
			return false;
		}

		editor->CloseLiveServer();
		return DoQuerySave(doclose);
	} else if (g_gui.ShouldSave()) {
		long ret = g_gui.PopupDialog(
			"Save changes",
			"Do you want to save your changes to \"" + wxstr(g_gui.GetCurrentMap().getName()) + "\"?",
			wxYES | wxNO | wxCANCEL
		);

		if (ret == wxID_YES) {
			if (g_gui.GetCurrentMap().hasFile()) {
				g_gui.SaveCurrentMap(true);
			} else {
				wxFileDialog file(this, "Save...", "", "", "*.otbm", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
				int32_t result = file.ShowModal();
				if (result == wxID_OK) {
					g_gui.SaveCurrentMap(file.GetPath(), true);
				} else {
					return false;
				}
			}
		} else if (ret == wxID_CANCEL) {
			return false;
		}
	}

	if (doclose) {
		UnnamedRenderingLock();
		g_gui.CloseCurrentEditor();
	}

	return true;
}

bool MainFrame::DoQueryImportCreatures() {
	if (g_creatures.hasMissing()) {
		long ret = g_gui.PopupDialog("Missing creatures", "There are missing creatures and/or NPC in the editor, do you want to load them from an OT monster/npc file?", wxYES | wxNO);
		if (ret == wxID_YES) {
			do {
				wxFileDialog dlg(g_gui.root, "Import monster/npc file", "", "", "*.xml", wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
				if (dlg.ShowModal() == wxID_OK) {
					wxArrayString paths;
					dlg.GetPaths(paths);
					for (uint32_t i = 0; i < paths.GetCount(); ++i) {
						wxString error;
						wxArrayString warnings;
						bool ok = g_creatures.importXMLFromOT(FileName(paths[i]), error, warnings);
						if (ok) {
							g_gui.ListDialog("Monster loader errors", warnings);
						} else {
							wxMessageBox("Error OT data file \"" + paths[i] + "\".\n" + error, "Error", wxOK | wxICON_INFORMATION, g_gui.root);
						}
					}
				} else {
					break;
				}
			} while (g_creatures.hasMissing());
		}
	}
	g_gui.RefreshPalettes();
	return true;
}

void MainFrame::UpdateFloorMenu() {
	menu_bar->UpdateFloorMenu();
}

bool MainFrame::LoadMap(FileName name) {
	return g_gui.LoadMap(name);
}

void MainFrame::OnExit(wxCloseEvent& event) {
	// clicking 'x' button

	// do you want to save map changes?
	while (g_gui.IsEditorOpen()) {
		if (!DoQuerySave()) {
			if (event.CanVeto()) {
				event.Veto();
				return;
			} else {
				break;
			}
		}
	}
	g_gui.aui_manager->UnInit();
	((Application&)wxGetApp()).Unload();
#ifdef __RELEASE__
	// Hack, "crash" gracefully in release builds, let OS handle cleanup of windows
	exit(0);
#endif
	Destroy();
}

void MainFrame::AddRecentFile(const FileName& file) {
	menu_bar->AddRecentFile(file);
}

void MainFrame::LoadRecentFiles() {
	menu_bar->LoadRecentFiles();
}

void MainFrame::SaveRecentFiles() {
	menu_bar->SaveRecentFiles();
}

std::vector<wxString> MainFrame::GetRecentFiles() {
	return menu_bar->GetRecentFiles();
}

void MainFrame::PrepareDC(wxDC& dc) {
	dc.SetLogicalOrigin(0, 0);
	dc.SetAxisOrientation(1, 0);
	dc.SetUserScale(1.0, 1.0);
	dc.SetMapMode(wxMM_TEXT);
}

// Add this method to handle exceptions in the main event loop
bool Application::OnExceptionInMainLoop() {
	try {
		throw; // Rethrow the exception to catch it
	} catch (const std::exception& e) {
		// Log the exception
		wxDateTime now = wxDateTime::Now();
		wxString logFileName = wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() +
			"exception_" + now.Format("%Y%m%d_%H%M%S") + ".log";

		// Create log directory
		wxFileName logDir(wxStandardPaths::Get().GetUserDataDir());
		if (!logDir.DirExists()) {
			wxMkdir(logDir.GetPath());
		}

		// Log details
		std::ofstream logFile(logFileName.ToStdString(), std::ios::app);
		if (logFile.is_open()) {
			logFile << "Exception in main loop at " << now.FormatISOCombined() << std::endl;
			logFile << "Exception: " << e.what() << std::endl;
			logFile << "RME version: " << __W_RME_VERSION__ << std::endl;
			logFile.close();
		}

		// Only show dialog during drawing operations for serious errors
		if (g_gui.IsEditorOpen()) {
			// Silently continue for editor operations to prevent UI freezes
			return true; // Continue execution
		} else {
			wxString msg = "An error occurred in " + wxString(__W_RME_APPLICATION_NAME__) + ".\n\n";
			msg += "Error details: " + wxString(e.what()) + "\n\n";
			msg += "The application will try to continue. If problems persist, please restart.";
			wxMessageBox(msg, "Error", wxICON_WARNING | wxOK);
			return true; // Continue execution
		}
	} catch (...) {
		// Unknown exception - more serious
		wxDateTime now = wxDateTime::Now();
		wxString logFileName = wxStandardPaths::Get().GetUserDataDir() + wxFileName::GetPathSeparator() +
			"unknown_exception_" + now.Format("%Y%m%d_%H%M%S") + ".log";

		// Create log directory
		wxFileName logDir(wxStandardPaths::Get().GetUserDataDir());
		if (!logDir.DirExists()) {
			wxMkdir(logDir.GetPath());
		}

		// Log whatever we can
		std::ofstream logFile(logFileName.ToStdString(), std::ios::app);
		if (logFile.is_open()) {
			logFile << "Unknown exception in main loop at " << now.FormatISOCombined() << std::endl;
			logFile << "RME version: " << __W_RME_VERSION__ << std::endl;
			logFile.close();
		}

		// Only show dialog during drawing operations for serious errors
		if (g_gui.IsEditorOpen()) {
			// Silently continue for editor operations
			return true; // Continue execution
		} else {
			wxString msg = "An unknown error occurred in " + wxString(__W_RME_APPLICATION_NAME__) + ".\n\n";
			msg += "The application will try to continue. If problems persist, please restart.";
			wxMessageBox(msg, "Error", wxICON_WARNING | wxOK);
			return true; // Continue execution
		}
	}

	return false; // If all else fails, let the default handler take over
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

md5_application_cpp = hashlib.md5(application_cpp_content.encode('utf-8')).hexdigest()
content_lite_application_cpp = "\\n".join(application_cpp_content.splitlines()[:200])

md5_application_h = hashlib.md5(application_h_content.encode('utf-8')).hexdigest()
content_lite_application_h = "\\n".join(application_h_content.splitlines()[:200])


yaml_data = {
    "wbs_item_id": "FINAL-02",
    "name": "Port Splash Screen & Startup Logic",
    "description": "Recreate the application's splash screen and initial startup sequence in Qt6. This includes loading initial configurations, checking for updates (if applicable), and transitioning to the main editor window.",
    "dependencies": [
        "CORE-02", # For client version loading
        "UI-01"    # Main window needs to be available to finish splash screen
    ],
    "input_files": [
        "wxwidgets/application.cpp",
        "wxwidgets/application.h"
    ],
    "analyzed_input_files": [
        {
            "file_path": "wxwidgets/application.cpp",
            "description": "Contains the main application class `Application`, including `OnInit` which handles startup sequences, settings loading, single instance check, main frame creation, and showing a welcome dialog (acting as a splash screen). `OnEventLoopEnter` handles initial map loading.",
            "md5_hash": md5_application_cpp,
            "content_lite": content_lite_application_cpp
        },
        {
            "file_path": "wxwidgets/application.h",
            "description": "Header file for the `Application` and `MainFrame` classes, defining their structure and event tables.",
            "md5_hash": md5_application_h,
            "content_lite": content_lite_application_h
        }
    ],
    "documentation_references": [
        "Qt SplashScreen Example: https://doc.qt.io/qt-6/qsplashscreen.html",
        "Qt QSettings: https://doc.qt.io/qt-6/qsettings.html",
        "Qt QSingleApplication (or custom IPC for single instance): https://doc.qt.io/archives/qq/qq26-singleapplication.html (Note: this is an older article, modern approach might use QSharedMemory + QLocalServer)",
        "Qt QCommandLineParser: https://doc.qt.io/qt-6/qcommandlineparser.html",
        "Qt QDesktopServices::openUrl: https://doc.qt.io/qt-6/qdesktopservices.html#openUrl"
    ],
    "current_functionality_summary": """\
The wxWidgets application (`Application::OnInit`) initializes various components:
- Sets up global exception handling.
- Discovers data directory.
- Loads settings, hotkeys, and client versions.
- Initializes image handlers and editor sprites.
- Performs a single instance check using `wxSingleInstanceChecker`. If another instance is running, it can send a file path via IPC and exit.
- Creates the `MainFrame`.
- Loads UI perspective and recent files.
- Shows a `wxWelcomeDialog` (which includes an icon `editor_icon.xpm` that gets a random hue shift) if no map is opened via command line and settings permit. This dialog acts as the splash/initial screen.
- `Application::OnEventLoopEnter` handles opening a map passed via command line or creating a new map.
- Logic exists to open specific URLs (Discord, Idler.live) on first launch or after a 7-day interval.
- Command line arguments like `-force-multi-instance` and `-ws` are parsed.
No separate `splash.cpp`/`.h` was found; the splash functionality is integrated into the welcome dialog shown during `OnInit`.\
""",
    "definition_of_done": [
        "A `QSplashScreen` is implemented, displaying an application logo (e.g., a PNG version of `editor_icon.xpm`).",
        "The random hue shift applied to the original `editor_icon.xpm` is replicated for the splash screen image if this visual effect is desired.",
        "The main application startup sequence (in `main.cpp` or a custom `QApplication` subclass) correctly initializes essential Qt services.",
        "Single instance logic is ported: The application checks if another instance is running. If so, it sends any command-line file path to the existing instance (e.g., using `QSharedMemory` and `QLocalServer`/`QLocalSocket`) and then exits.",
        "Application settings are loaded using `QSettings`.",
        "Client version data (from `CORE-02` task's output) is loaded during startup.",
        "The `QSplashScreen` is displayed early in the startup process.",
        "The main application window (`QMainWindow`) is created.",
        "After the main window is initialized and ready to be displayed, `QSplashScreen::finish(mainWindow)` is called to close the splash screen.",
        "Logic for opening a map from command-line arguments (using `QCommandLineParser`) or creating a new default map is implemented, similar to the behavior in `Application::OnEventLoopEnter`.",
        "The feature to open specific web URLs (e.g., Discord, project website) on first launch or after a set interval (e.g., 7 days) is replicated using `QDesktopServices::openUrl()` and `QSettings` for tracking.",
        "Command-line argument parsing for options like forcing multiple instances or controlling the welcome/splash behavior is implemented using `QCommandLineParser`."
    ],
    "boilerplate_coder_ai_prompt": """\
Your task is to port the wxWidgets application startup logic and splash screen to Qt6 for Remere's Map Editor.
The primary reference files from the wxWidgets version are `wxwidgets/application.cpp` and `wxwidgets/application.h`.

**Splash Screen:**
1.  Implement a splash screen using `QSplashScreen`.
2.  The original application used an XPM icon (`editor_icon.xpm`) and applied a random hue shift to it before displaying it in a welcome dialog (which served as a splash screen).
3.  You will need to:
    a.  Obtain or convert `editor_icon.xpm` to a common format like PNG that Qt can easily load (e.g., `editor_icon.png`).
    b.  If the random hue shift is a desired feature, implement a similar image manipulation function for `QPixmap` before setting it on the `QSplashScreen`.
    c.  Display the `QSplashScreen` early in the `main()` function of your Qt application.

**Application Initialization (typically in `main.cpp` and potentially a custom `MyApplication` class inheriting `QApplication`):**
1.  **Single Instance Check:**
    *   Implement logic to ensure only one instance of the application runs, unless a command-line flag like `-force-multi-instance` is used.
    *   Use `QSharedMemory` to detect if another instance is running.
    *   If another instance is detected, use `QLocalSocket` to send the command-line arguments (especially a map file to open) to the running instance (which should have a `QLocalServer` listening) and then the new instance should terminate.
2.  **Settings:** Load application settings using `QSettings` (equivalent to `g_settings`).
3.  **Client Versions:** Load client version data (this will likely depend on `CORE-02` task which handles parsing `clients.xml`).
4.  **Main Window Creation:** Create the main application window (e.g., `MainWindow : QMainWindow`).
5.  **Splash Screen Finalization:** Once the `MainWindow` is initialized and ready to be shown, call `splash->finish(mainWindow);`.
6.  **Command-Line Argument Parsing:**
    *   Use `QCommandLineParser` to handle command-line arguments.
    *   Support opening a map file specified on the command line.
    *   Support flags like `-force-multi-instance` (to disable single instance check) and `-ws` (to control welcome screen/splash behavior, if this feature is retained beyond a simple splash).
7.  **Initial Map Loading:** After the main window is set up, implement the logic found in `Application::OnEventLoopEnter`:
    *   If a map file was passed via command line, load it.
    *   If no map file is specified and the welcome dialog (if any, or if configured by `-ws`) isn't shown, create and display a new, empty map.
8.  **URL Opening Feature:**
    *   Replicate the logic to open specific URLs (e.g., "https://discord.gg/FD2cYKBq5E", "https://idler.live/") using `QDesktopServices::openUrl()`.
    *   This should happen on the first ever launch or if 7 days have passed since the last time these URLs were opened. Use `QSettings` to store a timestamp (`LAST_WEBSITES_OPEN_TIME`).

**Structure and Error Handling:**
- Refer to `wxwidgets/application.cpp` (`Application::OnInit`, `Application::OnEventLoopEnter`, `Application::ParseCommandLineMap`) for the sequence and details of these operations.
- Implement robust error handling for file operations and IPC.
- Ensure the Qt6 application structure is clean and follows Qt best practices.
- The `analyzed_input_files` section should be populated by reading the first 200 lines of `wxwidgets/application.cpp` and `wxwidgets/application.h` and calculating their MD5 hashes.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/FINAL-02.yaml" # Corrected to FINAL-02

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_data, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")

# Clean up the temporary file contents from variables (optional, good practice)
del application_cpp_content
del application_h_content
del yaml_data
