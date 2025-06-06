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
        "wxwidgets/graphics.cpp",
        "items.xml (Not Found via URL)",
        "creatures.xml (Not Found via URL)",
        "clients.xml (Not Found via URL)"
    ],
    "dependencies": [
        "CORE-01"
    ],
    "current_functionality_summary": """\
The input files manage critical game asset data:
- `ItemDatabase` (from `items.h/cpp`): Loads item definitions (type, flags, attributes) from `.otb` or legacy XML files (which would have been parsed by pugixml). Populates `g_items`.
- `CreatureDatabase` (from `creatures.h/cpp`): Loads creature/NPC definitions (name, outfit) from XML files (custom editor and OT server formats, which would have been parsed by pugixml). Populates `g_creatures`.
- `ClientVersion` (from `client_version.h/cpp`): Parses `clients.xml` (which would have been parsed by pugixml) for client compatibility data.
- `GraphicManager` (from `graphics.h/cpp`): Loads sprite metadata from `.dat` and image data from `.spr` files.
- `pugixml`: The XML parsing library to be replaced.
Note: The specific XML files (`items.xml`, `creatures.xml`, `clients.xml`) were not directly accessible for analysis during task generation; understanding of their expected structure is inferred from the C++ parsing code and `prompt.md`.\
""",
    "qt6_migration_steps": """\
1. Port the `ItemDatabase`, `ItemType`, `CreatureDatabase`, `CreatureType`, `ClientVersion`, `GraphicManager`, `GameSprite`, and `Animator` classes to the `mapcore` library. These should be structured as individual .h/.cpp files within the library's source tree.
2. Replace all XML parsing functionality currently using `pugixml` (in `ItemDatabase` for items.xml, `CreatureDatabase` for creatures.xml, `ClientVersion` for clients.xml) with `QXmlStreamReader` from the Qt XML module. The Coder AI will need to infer the XML structure from the existing C++ parsing logic or define a new one if necessary.
3. Update all file I/O operations (for `.otb`, `.dat`, `.spr`, and XML files if not handled by `QXmlStreamReader` directly) to use `QFile` and `QDataStream` (for binary) or `QTextStream` (for text).
4. Remove wxWidgets dependencies: Replace `wxString` with `std::string` (as per CORE-01 guidelines), `wxArrayString` with `std::vector<std::string>`, and remove any wx-specific logging or utility functions.
5. Refactor `GraphicManager` and `GameSprite`:
   - The core logic for parsing `.dat` and `.spr` files should be retained, using Qt file I/O classes.
   - `GameSprite` should primarily store raw pixel data and metadata. Methods like `getDC` that rely on `wxMemoryDC` should be removed from `mapcore`. The responsibility of creating `QImage` or `QPixmap` from this raw data will fall to the UI layer or utility functions outside/above `mapcore`.
   - The OpenGL texture ID aspect (`getHardwareID`) can be kept if `mapcore` is to provide data directly consumable by an OpenGL renderer, but abstract any direct GL calls.
6. Manage `ItemDatabase`, `CreatureDatabase`, and `GraphicManager` instances within `mapcore` (e.g., as library-internal singletons or through a dedicated context class), removing reliance on `g_gui` for `GraphicManager`.
7. Ensure the ported classes correctly use the data model classes (e.g., `Item`, `Creature`) isolated in `CORE-01`.
8. Verify that the `mapcore` library, now including these asset systems, compiles cleanly without wxWidgets or pugixml dependencies.\
""",
    "definition_of_done": """\
The `mapcore` library, structured as a collection of modular source files, can successfully parse and manage game asset definitions.
Key requirements:
- `ItemDatabase` loads item definitions from `.otb` files and from the legacy XML format (if applicable, structure inferred from code) using Qt XML.
- `CreatureDatabase` loads creature/NPC definitions from XML files (structure inferred from code) using Qt XML.
- `ClientVersion` loads client configuration from `clients.xml` (structure inferred from code) using Qt XML.
- `GraphicManager` loads sprite metadata from `.dat` and sprite pixel data from `.spr` files using Qt file I/O.
- `GameSprite` stores sprite data; wxWidgets specific rendering methods (like getDC) are removed.
- All pugixml dependencies are replaced with Qt XML.
- All wxWidgets dependencies are removed.
- The `mapcore` library remains self-contained and compiles successfully.
- Parsed data (items, creatures, client versions, sprites) is accessible through the respective database/manager classes.\
""",
    "boilerplate_coder_ai_prompt": """\
Your task is to port asset loading classes (`ItemDatabase`, `CreatureDatabase`, `ClientVersion`, `GraphicManager`, `GameSprite`, `Animator`) into the `mapcore` static library, building upon `CORE-01`.
These classes should be added as separate, modular `.h/.cpp` files within the `mapcore` library's source structure, NOT merged into a single file.
1.  Replace all `pugixml` XML parsing logic with `QXmlStreamReader` (from Qt XML module) for `items.xml`, `creatures.xml`, and `clients.xml`. Since the exact XML files might not be available to you directly, infer the structure from the existing C++ parsing code that uses pugixml.
2.  Replace all file I/O (for `.otb`, `.dat`, `.spr`, XMLs) with `QFile`, `QDataStream`, and `QTextStream`.
3.  Remove wxWidgets types (`wxString` to `std::string`, etc.) and functions.
4.  Refactor `GraphicManager`/`GameSprite`:
    - Sprite data loading from `.dat`/`.spr` should use Qt I/O.
    - `GameSprite` should hold raw sprite data. Remove `wxMemoryDC` methods; UI-specific image creation from this data is outside `mapcore`'s direct responsibility.
5.  Ensure these systems use the data models from `CORE-01`.
6.  The classes `ItemDatabase`, `CreatureDatabase`, `GraphicManager` should be instantiable and usable within `mapcore` without UI dependencies.
7.  The `mapcore` library, now including these asset systems, must compile cleanly without wxWidgets or pugixml.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/CORE-02.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
