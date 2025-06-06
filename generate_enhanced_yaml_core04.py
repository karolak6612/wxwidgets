import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "CORE-04",
    "section": "Core Migration Tasks",
    "title": "Port Brush & Materials System",
    "original_input_files": "brush.h/cpp (and all `*_brush.*`), materials.h/cpp, tileset.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/brush.h",
        "wxwidgets/brush.cpp",
        "wxwidgets/materials.h",
        "wxwidgets/materials.cpp",
        "wxwidgets/tileset.h",
        "wxwidgets/tileset.cpp",
        "wxwidgets/ground_brush.h",
        "wxwidgets/ground_brush.cpp",
        "wxwidgets/wall_brush.h",
        "wxwidgets/wall_brush.cpp",
        "wxwidgets/doodad_brush.h",
        "wxwidgets/doodad_brush.cpp",
        "wxwidgets/creature_brush.h",
        "wxwidgets/creature_brush.cpp",
        "wxwidgets/house_brush.h",
        "wxwidgets/house_brush.cpp",
        "wxwidgets/spawn_brush.h",
        "wxwidgets/spawn_brush.cpp",
        "wxwidgets/raw_brush.h",
        "wxwidgets/raw_brush.cpp",
        "wxwidgets/carpet_brush.h",
        "wxwidgets/carpet_brush.cpp",
        "wxwidgets/table_brush.h",
        "wxwidgets/table_brush.cpp",
        "wxwidgets/waypoint_brush.h",
        "wxwidgets/waypoint_brush.cpp",
    ],
    "dependencies": [
        "CORE-02"
    ],
    "current_functionality_summary": """\
The specified files define the map editing brushes and the material/tileset system.
- `brush.h/cpp` & `*_brush.h/cpp` files: Define a hierarchy of brush classes (Base `Brush`, `TerrainBrush`, `GroundBrush`, `WallBrush`, `DoodadBrush`, etc.) responsible for the logic of placing/modifying map elements. `Brushes` (`g_brushes`) is a global manager for these.
- `materials.h/cpp`: Defines `Materials` (`g_materials`), which loads `materials.xml` (using pugixml). This XML defines brush properties, items, border rules (`AutoBorder`), and organizes them into tilesets. Also handles material extensions.
- `tileset.h/cpp`: Defines `Tileset` and `TilesetCategory` for organizing brushes, primarily for the UI palette.
These systems depend on `CORE-01` data models and `CORE-02` asset definitions.\
""",
    "qt6_migration_steps": """\
1. Port all brush classes (`Brush` hierarchy, `Brushes` manager, `AutoBorder`), `Materials` manager, `Tileset`, and `TilesetCategory` classes to the `mapcore` library. Ensure these are structured as individual `.h/.cpp` files within `mapcore`.
2. In `Materials::loadMaterials` and its helper methods (like `Brushes::unserializeBrush`, `AutoBorder::load`, specific brush `load` methods), replace all `pugixml` XML parsing logic with `QXmlStreamReader`.
3. Remove all wxWidgets dependencies:
   - Convert `wxString` to `std::string`.
   - Convert `wxArrayString` to `std::vector<std::string>`.
   - Convert `wxFileName` to `std::filesystem::path` or `QFileInfo`.
   - Remove wx-specific logging or utilities.
4. Ensure all brush implementations correctly use data models from `CORE-01` (e.g., `Tile`, `Item`) and asset definitions (`g_items`, `g_creatures`) now managed within `mapcore` (as per `CORE-02`).
5. Manage `g_brushes` and `g_materials` instances within `mapcore` (e.g., as library-internal singletons or through a context class).
6. Port `Tileset` and `TilesetCategory` data structures. Their primary role is to provide data for the UI; remove any direct UI rendering or interaction logic if present.
7. Update path handling in `Materials::loadExtensions` to use `std::filesystem::path` or `QDir`/`QFileInfo` for directory iteration and loading extension XMLs.
8. Verify that `mapcore` compiles cleanly with the ported brush and materials system, ensuring no wxWidgets or pugixml dependencies remain.\
""",
    "definition_of_done": """\
The brush system and materials/tileset definitions are successfully ported to and functional within the `mapcore` library.
Key requirements:
- All brush classes (`Brush` hierarchy, `Brushes` manager, `AutoBorder`) and the `Materials`/`Tileset` system are part of `mapcore`, with each class in its own modular files.
- XML parsing for `materials.xml` and any extensions is performed using Qt XML (`QXmlStreamReader`), completely replacing `pugixml`.
- All wxWidgets dependencies (types, UI calls, logging) are removed.
- Brushes correctly operate on the `CORE-01` data models and utilize asset definitions from `CORE-02`.
- The `mapcore` library, now including these systems, compiles without errors.
- The brush and tileset data structures are correctly populated after parsing `materials.xml` (and extensions).\
""",
    "boilerplate_coder_ai_prompt": """\
Your task is to port the brush system (all classes derived from `Brush`, the `Brushes` manager, `AutoBorder`) and the materials system (`Materials`, `Tileset`, `TilesetCategory`) to the `mapcore` static library. This depends on `CORE-01` and `CORE-02`.
1. Relocate all relevant classes to `mapcore`, maintaining them as separate `.h/.cpp` files within the library's source structure.
2. Replace all `pugixml` usage with `QXmlStreamReader` for parsing `materials.xml` and any material extension XML files (e.g., in `Materials::loadMaterials`, `Brushes::unserializeBrush`, `AutoBorder::load`, and individual brush `load` methods).
3. Remove all wxWidgets dependencies (e.g., `wxString` to `std::string`, `wxArrayString` to `std::vector<std::string>`, `wxFileName` to `std::filesystem::path` or `QFileInfo`).
4. Ensure that the brush logic correctly interacts with the data models from `CORE-01` and asset definitions from `CORE-02` (now part of `mapcore`).
5. `Tileset` and `TilesetCategory` should be ported as data structures. Remove any direct UI code from them.
6. The global instances `g_brushes` and `g_materials` should be managed within `mapcore`.
7. Confirm that `mapcore` compiles cleanly and the materials system can successfully load its configuration from `materials.xml`.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/CORE-04.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
