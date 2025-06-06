import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "DOCS-02",
    "section": "Build, Deployment, & Documentation",
    "title": "Create Initial User Manual",
    "original_input_files": "`main_menubar.xml` (for features)",
    "analyzed_input_files": [
        "wxwidgets/main_menubar.h",
        "wxwidgets/main_menubar.cpp",
        "main_menubar.xml (Not Found)"
    ],
    "dependencies": [
        "FINAL-04"
    ],
    "current_functionality_summary": """\
The task is to create an initial user manual. The primary input, `main_menubar.xml` (which defines the application's menu structure and thus its features), was not found. The functionality of `main_menubar.h/.cpp` shows it loads an XML to create menus and maps items to a comprehensive list of `ActionID`s, revealing the application's feature set. The manual should cover installation, a UI tour, and usage of core editing tools.\
""",
    "qt6_migration_steps": """\
1. Review the list of `ActionID` enums in `wxwidgets/main_menubar.h` to understand the full list of application features and menu commands.
2. If `main_menubar.xml` (or `menubar.xml`) becomes available, parse it to understand the exact menu hierarchy, item names, and shortcuts as presented to the user.
3. Based on the features, outline the structure of the user manual (e.g., `UserManual.md`). Suggested sections:
   - "1. Introduction": Brief overview of the map editor.
   - "2. Installation":
     - Windows (MSI from `BUILD-02`).
     - macOS (DMG from `BUILD-02`).
     - Linux (.deb from `BUILD-02`).
   - "3. Getting Started - UI Overview":
     - Main Window (from `UI-01`, `UI-02`).
     - Menu Bar and Toolbars (from `UI-02`, based on `main_menubar.xml` or inferred from `main_menubar.h`).
     - Map Viewport (from `RENDER-01`).
     - Palettes (Brush, Creature, House, etc. from `UI-PALETTE-*` tasks).
     - Status Bar (from `FINAL-06`).
   - "4. Basic Operations":
     - Creating, Opening, Saving Maps (File menu actions).
     - Undo/Redo (Edit menu actions).
   - "5. Drawing Tools (Brushes)":
     - Overview of different brush types (Terrain, Doodad, Item, Creature, House, etc. - from palette tasks and material definitions).
     - How to select and use brushes.
     - Brush size/shape options (if applicable, from UI related to `GuiManager`).
   - "6. Editing Features":
     - Selection modes (from `SELECT_MODE_*` actions).
     - Copy, Cut, Paste (Edit menu).
     - Find/Replace Items (Tools menu).
     - Map Properties & Statistics (Map menu).
   - "7. Viewing Options":
     - Zooming, Floor navigation (View menu).
     - Show/Hide various elements (grid, creatures, spawns, lights - View menu).
   - "8. Advanced Tools (if applicable)":
     - Borderize, Randomize (Map menu).
     - Map Cleanup operations (Map menu).
   - "(Optional) Appendix: Keyboard Shortcuts" (from `main_menubar.xml` or `HotkeyManager`).
4. Write content for each section in clear, user-friendly language using Markdown formatting.
5. Include screenshots of the new Qt6 UI where helpful (once UI tasks like `FINAL-04` are complete).\
""",
    "definition_of_done": """\
A basic user manual in Markdown format (`UserManual.md`) is created.
Key requirements:
- Covers application installation for Windows, macOS, and Linux.
- Provides a tour of the new Qt6 UI, explaining major components (main window, map view, palettes, toolbars).
- Includes instructions for using core drawing and editing tools/features, derived from the application's menu structure (ideally from `main_menubar.xml` or inferred from `main_menubar.h`).
- The manual is written clearly and is suitable for new users.
- The Markdown is well-formatted.\
""",
    "boilerplate_coder_ai_prompt": """\
*This is a documentation task for a technical writer, not a direct coding task for a Coder AI.*

**Task: Create Initial User Manual**

**Input:**
- `wxwidgets/main_menubar.h` (for list of ActionIDs representing features).
- (Ideally) `main_menubar.xml` or `menubar.xml` if it can be located/provided, as it defines the user-facing menu structure, item names, and shortcuts. (Currently Not Found by AI Agent).
- General knowledge of the application's purpose (Tibia map editor).
- UI/Feature descriptions from other completed WBS tasks (especially UI-xx, RENDER-xx, FINAL-xx). This task depends on `FINAL-04` for UI stability.

**Output:**
- A Markdown file (e.g., `UserManual.md`) containing the initial user manual.

**Content Requirements:**
1.  **Installation Guide:** Brief steps for Windows (MSI), macOS (DMG), Linux (.deb).
2.  **UI Overview (Qt6 Version):**
    - Main window layout.
    - Menu bar and toolbars.
    - Map display area.
    - Key palettes (Brush, Item, Creature, House, etc.).
3.  **Core Functionality Guide:**
    - Creating, opening, saving maps.
    - Using different brush types for drawing (terrain, items, doodads).
    - Selecting and modifying map elements.
    - Using common tools (e.g., Find Item, Map Properties).
    - Navigating floors, zooming.
4.  Use clear language and provide step-by-step instructions where appropriate.
5.  Assume the user is new to the application.
6.  The manual should be structured logically with clear headings.
(Screenshots will be added later once the UI is stable).\
"""
}

output_file_path = "enhanced_wbs_yaml_files/DOCS-02.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
