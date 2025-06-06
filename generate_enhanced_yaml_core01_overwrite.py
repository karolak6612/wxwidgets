import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "CORE-01",
    "section": "Core Migration Tasks",
    "title": "Isolate Core Data Models",
    "original_input_files": "position.h, tile.h, item.h, item_attributes.h, complexitem.h, creature.h, outfit.h, spawn.h, house.h, town.h, waypoints.h, basemap.h, map_region.h, map.h, map_allocator.h",
    "analyzed_input_files": [
        "wxwidgets/position.h",
        "wxwidgets/tile.h",
        "wxwidgets/tile.cpp",
        "wxwidgets/item.h",
        "wxwidgets/item.cpp",
        "wxwidgets/item_attributes.h",
        "wxwidgets/item_attributes.cpp",
        "wxwidgets/complexitem.h",
        "wxwidgets/complexitem.cpp",
        "wxwidgets/creature.h",
        "wxwidgets/creature.cpp",
        "wxwidgets/outfit.h",
        "wxwidgets/spawn.h",
        "wxwidgets/spawn.cpp",
        "wxwidgets/house.h",
        "wxwidgets/house.cpp",
        "wxwidgets/town.h",
        "wxwidgets/town.cpp",
        "wxwidgets/waypoints.h",
        "wxwidgets/waypoints.cpp",
        "wxwidgets/basemap.h",
        "wxwidgets/basemap.cpp",
        "wxwidgets/map_region.h",
        "wxwidgets/map_region.cpp",
        "wxwidgets/map.h",
        "wxwidgets/map.cpp",
        "wxwidgets/map_allocator.h"
    ],
    "dependencies": None,
    "current_functionality_summary": """\
The specified files define the fundamental data structures for the map editor. This includes `Position`, `Tile` (with items, creatures, spawns, house info), `Item` (and its attributes and complex derived types like `Container`, `Door`), `Creature`, `Outfit`, `Spawn`, `House`, `Town`, `Waypoint`. The map itself is represented by `Map` (derived from `BaseMap`), which uses a quadtree-like structure (`QTreeNode`, `Floor`, `TileLocation`) for spatial organization and a `MapAllocator` for memory management. These models currently have some dependencies on wxWidgets types (e.g., `wxString`) and potentially global objects for item/creature definitions.\
""",
    "qt6_migration_steps": """\
1. Create a new CMake target for a static library named `mapcore`.
2. Move the source code of all listed input files (`position.h`, `tile.h/cpp`, `item.h/cpp`, etc.) into the `mapcore` library's source tree, maintaining their separate file structure.
3. Systematically remove all wxWidgets dependencies from these files:
   - Replace `wxString` with `std::string`.
   - Replace `wxArrayString` with `std::vector<std::string>`.
   - Replace `wxFileName` with `std::filesystem::path` (from C++17) for any path operations, ensuring such operations are minimal and justified within core models.
   - Remove `wxLog` calls and any other wx-specific macros or functions. Use `assert` for critical checks or a simple `std::cerr` for temporary debugging if needed.
   - Eliminate includes of any wxWidgets headers (e.g., `<wx/string.h>`, `<wx/filename.h>`).
4. Audit the `main.h` file included by many of these classes. If it contains wxWidgets includes or UI-related globals, refactor it. Create a new core-specific common header if necessary for `mapcore` that only contains definitions essential for these data models (e.g., constants like `MAP_MAX_WIDTH`, forward declarations).
5. Review and ensure all classes use standard C++ types consistently (e.g., `uint16_t`, `int32_t`, `std::vector`, `std::map`, `std::string`).
6. The `MapAllocator` using `newd` (presumably `new`) is acceptable. Ensure `newd` is defined/accessible without wx dependencies.
7. Preserve the existing custom binary serialization logic (OTBM/OTMM) within `Item`, `ComplexItem`, `ItemAttributes`, ensuring it uses standard C++ stream types or the provided `BinaryNode`/`NodeFileWriteHandle` without wx file/stream classes.
8. For global dependencies like `g_items` and `g_creatures` (item/creature type definitions): For this task, these can be declared as `extern` within `mapcore` if their full management is handled in a subsequent task (e.g., CORE-02). The key is `mapcore` itself should not define them or depend on wxWidgets for their initialization.
9. Ensure all header files have proper include guards.
10. Compile the `mapcore` library using CMake. Resolve any compilation errors until the library builds successfully as a self-contained unit without wxWidgets dependencies.\
""",
    "definition_of_done": """\
A static library named `mapcore` (e.g., `libmapcore.a` or `mapcore.lib`) is created and compiles successfully via CMake.
Key requirements:
- Contains all specified data model classes (Position, Tile, Item, Map, Creature, etc.), each in their own `.h` and `.cpp` files (or `.h` only if applicable) organized within the `mapcore` library's source structure.
- These classes are NOT merged into a single large `mapcore.cpp/h` file.
- All wxWidgets dependencies (e.g., wxString, wxArrayString, wxFileName, wxLog, wx headers) have been removed from these classes and replaced with standard C++ equivalents (std::string, std::vector, std::filesystem::path, assert/std::cerr).
- The library is self-contained regarding these core data models, with external dependencies (like item definitions) handled via `extern` declarations or passed-in references if refactored.
- The custom OTBM/OTMM serialization logic is preserved and functional using standard C++ types/streams or existing non-wx helper classes.
- The library compiles without errors or warnings related to missing wxWidgets types or functions.\
""",
    "boilerplate_coder_ai_prompt": """\
Your task is to refactor the provided C++ classes (Position, Tile, Item, Map, Creature, etc., and their dependencies like ItemAttributes, ComplexItem, BaseMap, MapAllocator, MapRegion, Outfit, Spawn, House, Town, Waypoints) into a new static library named `mapcore`.
This means creating a library target in CMake (e.g., `libmapcore.a` or `mapcore.lib`) and organizing these classes as separate, modular `.h/.cpp` files within that library's source directory. **Do NOT merge them into a single `mapcore.cpp` or `mapcore.h` file.**
The primary goal is to remove ALL wxWidgets dependencies from these individual files.
1.  Replace `wxString` with `std::string`.
2.  Replace `wxArrayString` with `std::vector<std::string>`.
3.  Replace `wxFileName` with `std::filesystem::path`.
4.  Remove any wxWidgets headers (e.g. `<wx/string.h>`).
5.  Remove or replace wxWidgets logging/assertion macros (e.g., `wxLogDebug`, `wxASSERT`). Use `assert()` for critical checks.
6.  The classes should use standard C++ types and practices.
7.  Pay attention to `main.h`; it must be stripped of wx-specific content for `mapcore` compilation. Create a `mapcore_utils.h` or similar if needed for shared constants/enums previously in `main.h` that are common to multiple classes within `mapcore`.
8.  Global item/creature definitions (`g_items`, `g_creatures`) can be declared `extern` for now within the relevant `mapcore` files that use them.
9.  The existing custom binary serialization logic must be preserved but use standard types.
10. Ensure the `mapcore` static library, consisting of these individual C++ files, compiles cleanly using CMake.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/CORE-01.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Overwritten/Generated {output_file_path}")
