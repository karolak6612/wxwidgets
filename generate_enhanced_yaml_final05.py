import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "FINAL-05",
    "section": "Finalization",
    "title": "Port About and Welcome Dialogs",
    "original_input_files": "about_window.h/cpp, welcome_dialog.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/about_window.h",
        "wxwidgets/about_window.cpp",
        "wxwidgets/welcome_dialog.h",
        "wxwidgets/welcome_dialog.cpp"
    ],
    "dependencies": [
        "UI-01"  # Implicitly FINAL-03 for AppSettings in WelcomeDialog
    ],
    "current_functionality_summary": """\
`about_window.h/cpp` defines a wxDialog displaying application information (name, version, license) and includes simple embedded Tetris and Snake games rendered using `wxDC`.
`welcome_dialog.h/cpp` defines a custom wxDialog shown on startup. It offers actions like "New", "Open", "Preferences", lists recent files, displays a "What's New" section from a text file, and has custom-styled UI elements.\
""",
    "qt6_migration_steps": """\
1. Create `AboutDialog.h/.cpp` using `QDialog` for the 'About' window.
   - Display application name, version, Qt version (`QT_VERSION_STR`), and OpenGL version (if accessible) using `QLabel`s.
   - Display license information, potentially in a `QTextBrowser` loading from a resource file.
   - Implement the Tetris and Snake games:
     - Create a base `GameWidget : QWidget` to handle `paintEvent` (for `QPainter` drawing), `keyPressEvent`, and a `QTimer` for the game loop logic.
     - Create `TetrisWidget : GameWidget` and `SnakeWidget : GameWidget`. Port the original game logic and rendering from the wxWidgets versions to use `QPainter` operations within their `paintEvent`.
     - Integrate these game widgets into the `AboutDialog`. They could be initially hidden and made visible via `QShortcut`s (e.g., specific key presses) or debug menu options.
2. Create `WelcomeDialog.h/.cpp` using `QDialog` for the 'Welcome' screen.
   - Replicate the layout using Qt layout classes (e.g., `QVBoxLayout`, `QHBoxLayout`, `QGridLayout`).
   - Display the application logo using a `QLabel` with a `QPixmap`.
   - Display title and version text using `QLabel`s.
   - Implement "New", "Open", and "Preferences" actions as `QPushButton`s. Connect their `clicked()` signals to slots that will trigger the corresponding application-level actions (e.g., by emitting signals that the main window or application class connects to).
   - For the "Recent Files" list: Use a `QListWidget`. Populate it with data retrieved from `AppSettings`. Handle `itemClicked` or `itemDoubleClicked` signals to open the selected file.
   - For the "What's New" section: Use a `QTextBrowser` to load and display content from `whatsnew.txt` (which should be included as a Qt resource or be in a standard application data location).
   - Implement the "Show this dialog on startup" `QCheckBox` and connect its `stateChanged(int)` signal to update the corresponding setting in `AppSettings`.
   - For custom styling of buttons or list items (e.g., hover effects), consider using event filters (`installEventFilter`, `eventFilter` method) or promoting widgets to custom classes that override `enterEvent`, `leaveEvent`, and `paintEvent`. Alternatively, simpler styling can be achieved with Qt Style Sheets (QSS) if exact visual replication is not paramount.
3. Remove the original `wxwidgets/about_window.*` and `wxwidgets/welcome_dialog.*` files from the build.\
""",
    "definition_of_done": """\
The "About" and "Welcome" dialogs are re-implemented using `QDialog` and Qt Widgets, providing equivalent functionality.
Key requirements:
- `AboutDialog` correctly displays application information (name, version, Qt version, license).
- The Tetris and Snake easter egg games are ported to `QWidget`-based implementations using `QPainter` for rendering and `QTimer` for game loops, and are accessible from the `AboutDialog`.
- `WelcomeDialog` provides functional "New", "Open", and "Preferences" actions.
- `WelcomeDialog` accurately displays a list of recent files (which are clickable to open them) and a "What's New" section.
- The "Show welcome dialog on startup" setting in `WelcomeDialog` correctly interacts with `AppSettings`.
- The dialogs are visually acceptable and function as reliable equivalents to their wxWidgets counterparts.\
""",
    "boilerplate_coder_ai_prompt": """\
Re-create the `AboutDialog` and `WelcomeDialog` as Qt dialogs (`QDialog`).
1.  **AboutDialog (`AboutDialog.h`, `AboutDialog.cpp`):**
    - Use `QLabel`s for textual information (application name, version, Qt version (`QT_VERSION_STR`), license summary).
    - For the Tetris and Snake games:
        - Create a base `GameWidget : QWidget` with `paintEvent(QPaintEvent*)` and `QTimer` logic.
        - Implement `TetrisWidget : GameWidget` and `SnakeWidget : GameWidget`.
        - Port the game logic from `wxwidgets/about_window.cpp`. Replace `wxDC` drawing with `QPainter` calls in `paintEvent`.
        - Make these games accessible from `AboutDialog` (e.g., via `QShortcut` or hidden buttons).
2.  **WelcomeDialog (`WelcomeDialog.h`, `WelcomeDialog.cpp`):**
    - Use Qt layouts (`QVBoxLayout`, `QHBoxLayout`) to arrange elements.
    - Use `QLabel` (with `QPixmap`) for the logo, and `QLabel`s for title/version text.
    - Use `QPushButton` for "New", "Open", "Preferences" actions. Connect `clicked()` signals to slots that will trigger application-level logic.
    - Use `QListWidget` for the recent files list. Populate from `AppSettings`. Handle `itemClicked` to open the selected file.
    - Use `QTextBrowser` to display `whatsnew.txt` (assume it's a resource or in a known path).
    - Use `QCheckBox` for the "Show this dialog on startup" option; connect its state to `AppSettings`.
    - If custom styling for hover effects on buttons/list items is needed, implement custom widgets overriding mouse/paint events or use event filters. Otherwise, standard Qt styling is acceptable.
3.  Ensure dialogs are correctly modal or modeless as appropriate and integrate with the main application flow (e.g., showing WelcomeDialog on startup if the setting is true and no map is opened).\
"""
}

output_file_path = "enhanced_wbs_yaml_files/FINAL-05.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
