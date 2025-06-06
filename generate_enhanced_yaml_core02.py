import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "CORE-02",
    "section": "Core Migration Tasks",
    "title": "Port Asset Database & Parsers",
    "original_input_files": "items.h/cpp, creatures.h/cpp, ext/pugixml.hpp, ext/pugixml.cpp, client_version.h/cpp, graphics.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/items.h",
        "wxwidgets/items.cpp",
        "wxwidgets/creatures.h",
        "wxwidgets/creatures.cpp",
        "wxwidgets/ext/pugixml.hpp",
        "wxwidgets/ext/pugixml.cpp",
        "wxwidgets/client_version.h",
        "wxwidgets/client_version.cpp",
        "wxwidgets/graphics.h",
        "wxwidgets/graphics.cpp"
    ],
    "dependencies": [
        "CORE-01"
    ],
    "current_functionality_summary": """\
The input files manage critical game asset data:
- `ItemDatabase` (from `items.h/cpp`): Loads item definitions (type, flags, attributes) from `.otb` or legacy XML files (using pugixml). Populates the `g_items` global.
- `CreatureDatabase` (from `creatures.h/cpp`): Loads creature/NPC definitions (name, outfit) from XML files (custom editor and OT server formats) using pugixml. Populates `g_creatures`.
- `ClientVersion` (from `client_version.h/cpp`): Parses `clients.xml` (using pugixml) for client compatibility data (OTB versions, DAT/SPR signatures).
- `GraphicManager` (from `graphics.h/cpp`): Loads sprite metadata from `.dat` and image data from `.spr` files. Manages animations. Populates `g_gui.gfx`.
- `pugixml`: The XML parsing library used.\
""",
    "qt6_migration_steps": """\
1. Port the `ItemDatabase`, `ItemType`, `CreatureDatabase`, `CreatureType`, `ClientVersion`, `GraphicManager`, `GameSprite`, and `Animator` classes to the `mapcore` library.
2. Replace all XML parsing currently using `pugixml` (in `ItemDatabase` for items.xml, `CreatureDatabase` for creatures.xml, `ClientVersion` for clients.xml) with `QXmlStreamReader` from Qt.
3. Update file I/O for `.otb`, `.dat`, `.spr`, and XML files to use `QFile`, `QDataStream` (binary), or `QTextStream` (text).
4. Remove wxWidgets dependencies: `wxString` to `std::string`, `wxArrayString` to `std::vector<std::string>`, remove wx-specific logging/utilities.
5. Refactor `GraphicManager` and `GameSprite`:
   - Retain core logic for parsing `.dat` and `.spr` using Qt file I/O.
   - `GameSprite` should primarily store raw pixel data and metadata. Remove `wxMemoryDC`-based methods; creating `QImage`/`QPixmap` is for the UI layer.
   - OpenGL texture ID management can remain conceptually, but direct GL calls should be abstracted or handled by UI.
6. Manage `ItemDatabase`, `CreatureDatabase`, `GraphicManager` instances within `mapcore` (e.g., as library-internal singletons or via a context class), removing `g_gui` dependency for `GraphicManager`.
7. Ensure ported classes use data models from `CORE-01`.
8. Compile `mapcore` cleanly without wxWidgets or pugixml dependencies.\
""",
    "definition_of_done": """\
The `mapcore` library can successfully parse and manage game asset definitions.
Key requirements:
- `ItemDatabase` loads item definitions from `.otb` files and legacy XML format using Qt XML.
- `CreatureDatabase` loads creature/NPC definitions from XML files using Qt XML.
- `ClientVersion` loads client configuration from `clients.xml` using Qt XML.
- `GraphicManager` loads sprite metadata from `.dat` and sprite pixel data from `.spr` files using Qt file I/O.
- `GameSprite` stores sprite data; wxWidgets specific rendering methods (like getDC) are removed.
- All pugixml dependencies are replaced with Qt XML.
- All wxWidgets dependencies are removed.
- The `mapcore` library remains self-contained and compiles successfully.
- Parsed asset data is accessible through the respective database/manager classes.\
""",
    "boilerplate_coder_ai_prompt": """\
Port asset loading classes (`ItemDatabase`, `CreatureDatabase`, `ClientVersion`, `GraphicManager`, `GameSprite`, `Animator`) to the `mapcore` library (depends on `CORE-01`).
1. Replace `pugixml` with `QXmlStreamReader` for all XML parsing (`items.xml`, `creatures.xml`, `clients.xml`).
2. Use `QFile`, `QDataStream`, `QTextStream` for all file I/O (`.otb`, `.dat`, `.spr`, XMLs).
3. Remove wxWidgets types (`wxString` to `std::string`, etc.) and functions.
4. Refactor `GraphicManager`/`GameSprite`:
   - `.dat`/`.spr` parsing logic should use Qt I/O.
   - `GameSprite` to hold raw sprite data. Remove `wxMemoryDC` methods. UI layer will handle `QImage`/`QPixmap` creation.
5. Ensure usage of `CORE-01` data models.
6. Make `ItemDatabase`, `CreatureDatabase`, `GraphicManager` usable within `mapcore` without UI dependencies.
7. `mapcore` must compile cleanly without wxWidgets or pugixml.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/CORE-02.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
