import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "FINAL-01",
    "section": "Core Migration Tasks",
    "title": "Port Settings & Preferences",
    "original_input_files": "settings.h/cpp, preferences.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/settings.h",
        "wxwidgets/settings.cpp",
        "wxwidgets/preferences.h",
        "wxwidgets/preferences.cpp"
    ],
    "dependencies": [
        "UI-01"
    ],
    "current_functionality_summary": """\
The `Settings` class (`settings.h/cpp`) manages all application configurations using an enum `Config::Key` and `wxConfig` for persistence (file or registry). `PreferencesWindow` (`preferences.h/cpp`) is a `wxDialog` with multiple pages allowing users to modify these settings via various wxWidgets UI controls.\
""",
    "qt6_migration_steps": """\
1. Refactor the `Settings` class to eliminate `wxConfigBase` dependency.
   - Replace its file/registry I/O backend with `QSettings`. Choose a storage format (e.g., INI file via `QSettings::IniFormat`) and location (e.g., using `QStandardPaths`).
   - Update the `IO()` method (and its internal macros `Int`, `String`, `Float`) to use `qsettings.setValue()` for saving and `qsettings.value().toType()` for loading.
   - Convert `wxString` usage to `QString` when interacting with `QSettings` or `std::string` for internal storage, ensuring consistency with `mapcore` data types.
   - Ensure the `Settings` class itself becomes UI-framework-agnostic.
2. Create a new `PreferencesDialog` class inheriting from `QDialog` using Qt Widgets.
3. Replicate the multi-page structure of `PreferencesWindow` using `QTabWidget` or similar, creating tabs for 'General', 'Editor', 'Graphics', 'Interface', 'Client Version', 'LOD', 'Automagic'.
4. For each configuration item in `Config::Key`, add a corresponding Qt widget to the appropriate tab in `PreferencesDialog` (e.g., `QCheckBox`, `QSpinBox`, `QComboBox`, `QLineEdit` + `QPushButton` for directory picking, custom color picker for `QColor`).
5. Populate the Qt widgets with current values from the refactored `g_settings` object when the dialog is constructed.
6. Implement an `apply()` slot in `PreferencesDialog` that:
   - Reads the current values from all Qt widgets.
   - Calls the appropriate setters in the `g_settings` object.
   - Calls `g_settings.save()`.
7. Connect the 'OK' button to `apply()` and then `accept()`, 'Apply' button to `apply()`, and 'Cancel' button to `reject()`.
8. Address any logic that requires immediate UI updates or signals an application restart upon changing certain settings (e.g., by emitting signals from the dialog).\
""",
    "definition_of_done": """\
Application settings are managed using QSettings and can be modified through a new Qt-based Preferences dialog.
Key requirements:
- The `Settings` class uses `QSettings` for loading and saving all configurations, replacing `wxConfig`.
- A new `PreferencesDialog` (inheriting `QDialog`) is implemented using Qt Widgets, replicating the functionality and layout of the original `PreferencesWindow`.
- All settings defined in `Config::Key` are accessible and modifiable through the new dialog.
- Changes made in the dialog are correctly saved via `QSettings` and reflected in the application's behavior.
- The `Settings` class is UI-framework-agnostic after refactoring.\
""",
    "boilerplate_coder_ai_prompt": """\
Refactor the `Settings` class (settings.h/cpp) to use `QSettings` as its backend instead of `wxConfig`.
1. Modify the `load()` and `save()` methods (and the internal `IO()` method with its macros) to read from and write to `QSettings`.
   - Choose a suitable format (e.g., `QSettings::IniFormat`) and path (e.g., `QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)`).
   - Use `QSettings::setValue(QString key, QVariant value)` and `QSettings::value(QString key, QVariant defaultValue).toType()` for I/O. The key should be the string name of the Config::Key enum member.
2. Replace `wxString` usage with `QString` or `std::string` as appropriate.
3. Create a new `PreferencesDialog` class inheriting `QDialog`.
4. Replicate the UI of the existing `PreferencesWindow` (from `preferences.h/cpp`) using `QTabWidget` and appropriate Qt input widgets (`QCheckBox`, `QSpinBox`, `QComboBox`, `QLineEdit` for paths, etc.) for all settings. Consult `preferences.cpp` for the structure and types of controls used for each setting.
5. Connect the dialog widgets to load values from the refactored `Settings` instance and to save values back using `Settings::set*` methods and `Settings::save()`.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/FINAL-01.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
