import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

# Ensure placeholders are treated as literals
boilerplate_prompt_content = """\
Implement a `ThemeManager` class and Qt Style Sheet (`.qss`) based theming.
1. Create `ThemeManager.h` and `ThemeManager.cpp`.
2. The `ThemeManager` will read theme preferences (light/dark, and optionally a custom dark base color) from `AppSettings` (task `FINAL-03`).
3. Create `light.qss` (can be minimal for default Qt look) and `dark.qss`. `dark.qss` should style common widgets (windows, buttons, inputs, lists, menus, toolbars, statusbar etc.) for a dark appearance. Store these as Qt resources or in an accessible directory.
4. Implement `ThemeManager::applyCurrentTheme()`:
   - Fetches theme settings from `AppSettings`.
   - If dark theme + custom color: Consider loading a `dark_template.qss` with placeholders (e.g., `@bgColor: {{backgroundColor}};`) and replacing them with actual color values from `AppSettings` to form the final QSS string. (Note: {{backgroundColor}} is a placeholder example, actual QSS syntax for variables might differ or require string replacement).
   - Otherwise, load the content of `light.qss` or the standard `dark.qss`.
   - Call `qApp->setStyleSheet(finalQssString)`.
5. Integrate `ThemeManager`:
   - Call `applyCurrentTheme()` after `AppSettings` are loaded during application startup.
   - In `PreferencesDialog`, when theme-related settings are changed:
     - Update `AppSettings`.
     - Call `ThemeManager::instance()->applyCurrentTheme()` (if ThemeManager is a singleton).
6. Remove the old `wxwidgets/dark_mode_manager.h/cpp` and its usages.\
"""

yaml_content = {
    "id": "FINAL-04",
    "section": "Finalization",
    "title": "Implement Theme and Style Management",
    "original_input_files": "dark_mode_manager.h/cpp, application.h/cpp (`FixVersionDiscrapencies`)",
    "analyzed_input_files": [
        "wxwidgets/dark_mode_manager.h",
        "wxwidgets/dark_mode_manager.cpp",
        "wxwidgets/application.h",
        "wxwidgets/application.cpp"
    ],
    "dependencies": [
        "UI-01"  # Implicitly depends on FINAL-03 for AppSettings
    ],
    "current_functionality_summary": """\
The existing `DarkModeManager` (`dark_mode_manager.h/cpp`) programmatically sets background and foreground colors of `wxWindow` objects to switch between a light and a dark theme. The dark theme can use default hardcoded dark colors or a user-defined custom base color fetched from `g_settings`. It does not use external stylesheet files. The `FixVersionDiscrapencies` method in `application.cpp` is for settings migration and not directly for theming logic.\
""",
    "qt6_migration_steps": """\
1. Create a new C++ class `ThemeManager` (e.g., in `ThemeManager.h` and `ThemeManager.cpp`). This class will be responsible for managing theme application.
2. Design `ThemeManager` to load and apply Qt Style Sheets (.qss files). It should support at least 'light' and 'dark' themes.
3. Create `light.qss` and `dark.qss` files. These should be included as Qt resources or placed in a discoverable application directory. `dark.qss` must define styles for common Qt widgets to achieve a comprehensive dark theme. `light.qss` can be minimal if the default Qt styling is acceptable for the light theme.
4. To support the original feature of a user-defined custom base color for the dark theme:
   - `ThemeManager` should read this custom base color preference from `AppSettings` (from `FINAL-03`).
   - If a custom color is set and enabled, `ThemeManager` might need to dynamically generate the final `dark.qss` content. This could involve loading a `dark_template.qss` file with placeholders (e.g., `@backgroundColor`, `@textColor`) and replacing these placeholders with the user's custom color values before applying the stylesheet.
5. Implement a method in `ThemeManager` (e.g., `void applyCurrentTheme()`) that:
   - Determines the current theme (light/dark) and custom color settings from `AppSettings`.
   - Loads (and potentially processes for custom colors) the appropriate `.qss` file content.
   - Applies the stylesheet globally to the application using `qApp->setStyleSheet(qssString)`.
6. In the application's main function or main window constructor, after `AppSettings` are loaded, instantiate/initialize `ThemeManager` and call its method to apply the initially configured theme.
7. Modify the `PreferencesDialog` (from `UI-DIALOGS-02` / `FINAL-01`):
   - The 'Dark Mode' checkbox, when its state changes, should update the relevant setting in `AppSettings`.
   - The custom dark color picker (if implemented) should update its setting in `AppSettings`.
   - After any theme-related setting is changed in `AppSettings` via the dialog, the `ThemeManager` should be invoked to re-apply the theme to reflect changes immediately.
8. Remove the old `wxwidgets/dark_mode_manager.h` and `wxwidgets/dark_mode_manager.cpp` files and ensure no part of the codebase attempts to use `g_darkMode`.\
""",
    "definition_of_done": """\
The application supports light and dark themes using Qt Style Sheets (.qss), configurable via the preferences dialog.
Key requirements:
- A `ThemeManager` class is implemented for managing theme loading and application.
- At least `light.qss` and `dark.qss` files are created and provide distinct visual styles for common application widgets.
- The application applies the selected theme (and custom color if that feature is retained) on startup, based on settings from `AppSettings`.
- The "Dark Mode" checkbox (and custom color picker, if retained) in the `PreferencesDialog` correctly updates settings in `AppSettings` and triggers `ThemeManager` to apply the theme changes live.
- The old wxWidgets-based `DarkModeManager` is completely removed and no longer used.
- The visual appearance of the application consistently and correctly reflects the selected theme across all windows and dialogs.\
""",
    "boilerplate_coder_ai_prompt": boilerplate_prompt_content
}

output_file_path = "enhanced_wbs_yaml_files/FINAL-04.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
