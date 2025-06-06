import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "FINAL-03",
    "section": "Finalization",
    "title": "Port Application Settings Logic",
    "original_input_files": "settings.h/cpp, preferences.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/settings.h",
        "wxwidgets/settings.cpp",
        "wxwidgets/preferences.h",
        "wxwidgets/preferences.cpp",
        "Note: Relies on FINAL-01 for QSettings backend in Settings class and UI-DIALOGS-02 for the Preferences Dialog."
    ],
    "dependencies": [
        "UI-DIALOGS-02"
    ],
    "current_functionality_summary": """\
The application currently uses a global `g_settings` object (instance of `Settings` from `settings.h/cpp`) to manage configurations, which were originally persisted using `wxConfig`. Task `FINAL-01` aimed to refactor the underlying `Settings` class to use `QSettings`. This task (`FINAL-03`) focuses on creating a new singleton accessor (e.g., `AppSettings`) for these settings and replacing all usages of the old `g_settings` global throughout the entire codebase with this new singleton.\
""",
    "qt6_migration_steps": """\
1. Define a new singleton class named `AppSettings` (e.g., in `AppSettings.h` and `AppSettings.cpp`). This class will be the central point for accessing all application settings.
2. The `AppSettings` class must provide a static method to access its single instance (e.g., `AppSettings::instance()`).
3. Internally, `AppSettings` must use `QSettings` for loading and saving all configuration values. It can either encapsulate the `Settings` class refactored in `FINAL-01` (which should now use `QSettings`) or re-implement similar logic directly using `QSettings`.
   - Ensure it uses a consistent organization name (e.g., "RME-QtDev") and application name (e.g., "RME-Qt") for `QSettings`, possibly set via `QCoreApplication::setOrganizationName()` and `QCoreApplication::setApplicationName()` early in main.
   - It must support all configuration keys from the `Config::Key` enum (defined in the original `wxwidgets/settings.h`).
4. Implement typed getter methods in `AppSettings` (e.g., `int getInt(Config::Key key, int defaultValue) const;`, `QString getString(Config::Key key, const QString& defaultValue) const;`) that retrieve values from `QSettings`.
5. Implement typed setter methods in `AppSettings` (e.g., `void setInt(Config::Key key, int value);`, `void setString(Config::Key key, const QString& value);`) that write values to `QSettings`. Consider if `QSettings::sync()` is needed after sets or if it can be deferred.
6. Perform a codebase-wide replacement of all accesses to the old global `g_settings` object (e.g., `g_settings.getInteger(Config::SOME_KEY)`) with calls to the new `AppSettings` singleton (e.g., `AppSettings::instance()->getInt(Config::SOME_KEY, default_value)`).
7. Ensure that the `PreferencesDialog` (from task `UI-DIALOGS-02`, which relies on `FINAL-01`'s work) correctly interacts with the new `AppSettings` singleton to load current settings into the UI and to save modified settings.
8. Verify that all application settings are correctly persisted to the chosen `QSettings` backend (e.g., INI file or native registry) and are loaded correctly across application sessions.\
""",
    "definition_of_done": """\
The application uses a new `AppSettings` singleton for all configuration management, fully replacing the old `g_settings` global.
Key requirements:
- An `AppSettings` singleton class is implemented and uses `QSettings` for persistence.
- All settings previously accessed via `g_settings` and defined in `Config::Key` are now managed by `AppSettings`.
- All code throughout the application (ported `mapcore` logic, new Qt UI code) uses `AppSettings::instance()` to access settings.
- Settings are correctly loaded at startup and saved upon modification or application exit.
- The `PreferencesDialog` (from UI-DIALOGS-02 / FINAL-01) correctly reflects and modifies settings through `AppSettings`.\
""",
    "boilerplate_coder_ai_prompt": """\
Create a new singleton class, `AppSettings`, to manage all application settings using `QSettings`. This task builds upon the settings system refactored in `FINAL-01` and the preferences dialog from `UI-DIALOGS-02`.
1. Define `AppSettings.h` and `AppSettings.cpp`.
2. Implement the singleton pattern (e.g., a static `AppSettings& instance()` method).
3. The `AppSettings` class should use `QSettings` as its storage backend. It might internally use or adapt the `Settings` class logic from `FINAL-01` (which should have been refactored to use `QSettings`). Ensure consistent use of organization and application names for `QSettings`.
4. Ensure `AppSettings` supports all configuration keys from the `Config::Key` enum (originally in `wxwidgets/settings.h`). You might need to copy/adapt this enum into `AppSettings.h` or a shared core header.
5. Provide typed public methods in `AppSettings` for getting and setting each configuration key (e.g., `int getUndoSize() const;`, `void setUndoSize(int size);`). These methods will internally use `QSettings::value()` and `QSettings::setValue()`, mapping `Config::Key` enum values to string keys for `QSettings`.
6. Search the entire codebase (ported `mapcore` and new Qt UI code) and replace every instance of `g_settings.getInteger(Config::KEY)`, `g_settings.getString(Config::KEY)`, etc., with calls to the new `AppSettings::instance()` methods.
7. Ensure the `PreferencesDialog` (from `UI-DIALOGS-02`, which depends on `FINAL-01`) correctly uses `AppSettings` to display and save settings.
8. Verify settings persistence across application sessions.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/FINAL-03.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
