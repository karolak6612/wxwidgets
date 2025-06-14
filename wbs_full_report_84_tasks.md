# WBS Task Porting Status Report (Partial - First 20 Tasks)

---
## BRUSH-LOGIC-Carpet - Port Carpet Brush Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/carpet_brush.cpp`
- `wxwidgets/carpet_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/CarpetBrush.h`
- `Project_QT/src/core/brush/CarpetBrush.cpp`

**Details/Summary:**
Core logic for carpet placement, including neighbor analysis using static lookup tables (`s_carpet_types`) and material definitions, found in Qt6 files. Integration with EditorController for undo/redo is present.

---
## BRUSH-LOGIC-Creature - Port Creature Brush Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/creature_brush.cpp`
- `wxwidgets/creature_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/CreatureBrush.h`
- `Project_QT/src/core/brush/CreatureBrush.cpp`

**Details/Summary:**
Handles placement/erasure of creatures using `CreatureData`. Auto-creation of spawns and undo/redo via EditorController are present. A minor potential variable name typo (`m_creatureType` vs `m_creatureData`) was noted in `canApply`.

---
## BRUSH-LOGIC-Doodad - Port Doodad Brush Logic

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/doodad_brush.cpp`
- `wxwidgets/doodad_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/DoodadBrush.h`
- `Project_QT/src/core/brush/DoodadBrush.cpp`

**Details/Summary:**
Supports single and multi-tile doodads with offsets and variations ("alternates") using `MaterialData`. Undo/redo via EditorController. Appears complete and robust.

---
## BRUSH-LOGIC-Eraser - Port Eraser Brush Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/eraser_brush.cpp`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/EraserBrush.h`
- `Project_QT/src/core/brush/EraserBrush.cpp`

**Details/Summary:**
Implements normal and aggressive erase modes, respects `AppSettings` for preserving unique/complex items. Uses a consolidated `RecordModifyTileContentsCommand` for undo. Border re-calculation for aggressive erase is not explicitly part of the brush logic.

---
## BRUSH-LOGIC-Ground - Port Ground Brush Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/ground_brush.cpp`
- `wxwidgets/ground_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/GroundBrush.h`
- `Project_QT/src/core/brush/GroundBrush.cpp`

**Details/Summary:**
Complex auto-bordering logic substantially ported, including `s_border_types` lookup table, rule matching (align, toBrushName, friends, super rules), and BorderSet resolution. Some advanced specific rule conditions/actions (edge-specific parts) have placeholder warnings. `determineAlignString` simplification needs verification against material XMLs.

---
## BRUSH-LOGIC-House - Port House Brush Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/house_brush.cpp`
- `wxwidgets/house_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/HouseBrush.h`
- `Project_QT/src/core/brush/HouseBrush.cpp`
- `Project_QT/src/editor_logic/commands/SetHouseTileCommand.h`
- `Project_QT/src/editor_logic/commands/SetHouseTileCommand.cpp`

**Details/Summary:**
Assigns/unassigns house IDs and PZ flags using `SetHouseTileCommand`. Stores house ID (safer than pointer). AppSettings for item removal/door IDs are not directly handled in the brush's `apply` method, likely deferred to the command or pending.

---
## BRUSH-LOGIC-HouseExit - Port House Exit Brush/Tool Logic

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/house_exit_brush.cpp`
- `wxwidgets/house_exit_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/houses/House.h`
- `Project_QT/src/core/houses/House.cpp`
- `Project_QT/src/editor_logic/commands/SetHouseExitCommand.h`
- `Project_QT/src/editor_logic/commands/SetHouseExitCommand.cpp`
- `Project_QT/src/editor_logic/EditorController.h`
- `Project_QT/src/editor_logic/EditorController.cpp`
- `Project_QT/src/core/map/Map.h`
- `Project_QT/src/core/map/Map.cpp` (for `isValidHouseExitLocation`)

**Details/Summary:**
Implemented as a "Tool" via `EditorController::setHouseExit`, which uses `SetHouseExitCommand`. `House::setExit` correctly updates tile flags. `Map::isValidHouseExitLocation` is used for validation.

---
## BRUSH-LOGIC-RAW - Port RAW Brush Logic

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/raw_brush.cpp`
- `wxwidgets/raw_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/RawBrush.h`
- `Project_QT/src/core/brush/RawBrush.cpp`

**Details/Summary:**
Correctly places/erases items by ID, handling both ground and non-ground items using specific commands (`RecordSetGroundCommand`, `RecordAddRemoveItemCommand`).

---
## BRUSH-LOGIC-Spawn - Port Spawn Brush Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/spawn_brush.cpp`
- `wxwidgets/spawn_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/SpawnBrush.h`
- `Project_QT/src/core/brush/SpawnBrush.cpp`

**Details/Summary:**
Marks/unmarks tiles as spawn points and sets radius using `SpawnData` and appropriate commands (`recordAddSpawn`, `recordRemoveSpawn`, `recordUpdateSpawn`). New spawns get default interval and empty creature lists, aligning with a basic spawn area brush.

---
## BRUSH-LOGIC-Table - Port Table Brush Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/table_brush.cpp`
- `wxwidgets/table_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/TableBrush.h`
- `Project_QT/src/core/brush/TableBrush.cpp`

**Details/Summary:**
Auto-alignment logic for table segments using `s_table_types` lookup table and material definitions found in Qt6 files. Similar structure to CarpetBrush. Undo/redo via EditorController.

---
## BRUSH-LOGIC-Wall - Port Wall Brush Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/wall_brush.cpp`
- `wxwidgets/wall_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/WallBrush.h`
- `Project_QT/src/core/brush/WallBrush.cpp`

**Details/Summary:**
Complex auto-walling logic (4-way connectivity, segment determination using `s_full_wall_types`/`s_half_wall_types`, door/window placement from material definitions, friend materials) largely ported. Undo/redo via EditorController.

---
## BRUSH-LOGIC-Waypoint - Port Waypoint Brush/Tool Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/waypoint_brush.cpp`
- `wxwidgets/waypoint_brush.h`

**Inferred Qt6 Files:**
- `Project_QT/src/editor_logic/EditorController.h`
- `Project_QT/src/editor_logic/EditorController.cpp`
- `Project_QT/src/core/waypoints/WaypointManager.h`
- `Project_QT/src/core/waypoints/WaypointManager.cpp`
- `Project_QT/src/editor_logic/commands/AddWaypointCommand.h`
- `Project_QT/src/editor_logic/commands/AddWaypointCommand.cpp`
- `Project_QT/src/editor_logic/commands/MoveWaypointCommand.h`
- `Project_QT/src/editor_logic/commands/MoveWaypointCommand.cpp`

**Details/Summary:**
Implemented as a "Tool" via `EditorController::placeOrMoveWaypoint`. Uses `AddWaypointCommand` or `MoveWaypointCommand` which interact with `WaypointManager`. Core logic for managing waypoints seems present.

---
## BUILD-00 - Create Root CMakeLists.txt and Basic Project Structure

**Status:** Largely Implemented

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- `Project_QT/CMakeLists.txt`

**Details/Summary:**
`Project_QT/CMakeLists.txt` pre-existed and substantially covers requirements like CMake version, project definition (name RME-Qt6), C++17 standard, Qt6 setup (Automoc, find_package), and test enabling.

---
## BUILD-01 - Setup Initial CMake Build System for Qt6 Project

**Status:** Fully Implemented

**Original wxWidgets Files:**
- `wxwidgets/application.cpp`
- `wxwidgets/editor.cpp`
- `wxwidgets/map_display.cpp`
- `XML/clients.xml`

**Inferred Qt6 Files:**
- `Project_QT/CMakeLists.txt`
- `Project_QT/src/CMakeLists.txt`

**Details/Summary:**
Requirements met by the combination of the root `Project_QT/CMakeLists.txt` (Qlementine integration, XML resource handling) and `Project_QT/src/CMakeLists.txt` (executable definition `rme_qt_app`, linking against Qt6/Qlementine/internal libs, placeholder wxWidgets sources).

---
## BUILD-02 - Implement Packaging and Deployment

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/application.cpp`
- `wxwidgets/artprovider.cpp`

**Inferred Qt6 Files:**
- `Project_QT/CMakeLists.txt`
- `Project_QT/src/CMakeLists.txt`

**Details/Summary:**
CPack configuration added to root `CMakeLists.txt`. `install(TARGETS rme_qt_app ...)` rule added to `src/CMakeLists.txt`. Setup for `windeployqt` and `macdeployqt` included. Pending manual creation of icon files and a non-placeholder WiX GUID by the user.

---
## CORE-BRUSH-FRAMEWORK - Define Base Brush Class and Management

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/brush.h`
- `wxwidgets/brush.cpp`
- `wxwidgets/editor.cpp` (for brush management)

**Inferred Qt6 Files:**
- `Project_QT/src/core/brush/Brush.h`
- `Project_QT/src/core/brush/Brush.cpp`
- `Project_QT/src/core/brush/BrushSettings.h`
- `Project_QT/src/core/brush/BrushManagerService.h`
- `Project_QT/src/core/brush/BrushManagerService.cpp`
- `Project_QT/src/editor_logic/EditorController.h` (uses BrushManagerService)

**Details/Summary:**
Abstract `Brush` class, `BrushSettings`, and `BrushManagerService` (used by `EditorController`) are well-defined in the Qt6 codebase, providing a solid framework for various brush types.

---
## CORE-01 - Port Core Data Structures (Position, Item, Tile)

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/tile.h`
- `wxwidgets/tile.cpp`
- `wxwidgets/item_attributes.h`
- `wxwidgets/item.h`
- `wxwidgets/item.cpp`
- `common/position.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/Position.h`
- `Project_QT/src/core/Position.cpp`
- `Project_QT/src/core/Item.h`
- `Project_QT/src/core/Item.cpp`
- `Project_QT/src/core/Tile.h`
- `Project_QT/src/core/Tile.cpp`
- `Project_QT/src/core/IItemTypeProvider.h`

**Details/Summary:**
Essential classes `Position`, `Item`, and `Tile` are implemented in `Project_QT/src/core/` and appear well-developed, using modern C++ practices. `IItemTypeProvider` interface also present.

---
## CORE-02 - Implement Asset Database and Parsers

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/item_manager.h`
- `wxwidgets/item_manager.cpp`
- `wxwidgets/spritemanager.h`
- `wxwidgets/spritemanager.cpp`
- `wxwidgets/creaturemanager.h`
- `wxwidgets/creaturemanager.cpp`
- `wxwidgets/clients.h`
- `wxwidgets/clients.cpp`

**Inferred Qt6 Files:**
- `Project_QT/src/core/assets/AssetManager.h`
- `Project_QT/src/core/assets/AssetManager.cpp`
- `Project_QT/src/core/assets/ItemDatabase.h`
- `Project_QT/src/core/assets/ItemDatabase.cpp`
- `Project_QT/src/core/assets/CreatureDatabase.h`
- `Project_QT/src/core/assets/CreatureDatabase.cpp`
- `Project_QT/src/core/assets/SpriteManager.h`
- `Project_QT/src/core/assets/SpriteManager.cpp`
- `Project_QT/src/core/assets/MaterialManager.h`
- `Project_QT/src/core/assets/MaterialManager.cpp`
- `Project_QT/src/core/assets/ClientVersionManager.h`
- `Project_QT/src/core/assets/ClientVersionManager.cpp`

**Details/Summary:**
A comprehensive set of asset management classes (`AssetManager`, `ItemDatabase`, `CreatureDatabase`, `SpriteManager`, `MaterialManager`, `ClientVersionManager`) exists in `core/assets/`. XML parsing appears to use `QXmlStreamReader`. Full parsing logic implementation status varies per manager but framework is solid.

---
## CORE-03 - Design and Implement Map Data Structure

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/map.h`
- `wxwidgets/map.cpp`
- `wxwidgets/tile.h` (as it's part of map structure)
- `wxwidgets/floor.h`
- `wxwidgets/floor.cpp`
- `wxwidgets/quadtree.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/map/Map.h`
- `Project_QT/src/core/map/Map.cpp`
- `Project_QT/src/core/map/BaseMap.h`
- `Project_QT/src/core/map/Floor.h`
- `Project_QT/src/core/map/Floor.cpp`
- `Project_QT/src/core/map/QTreeNode.h`
- `Project_QT/src/core/map/QTreeNode.cpp`
- `Project_QT/src/core/map/MapIterator.h`
- `Project_QT/src/core/map/MapIterator.cpp`

**Details/Summary:**
The map structure using `Map`, `BaseMap`, `Floor`, and `QTreeNode` (for quadtree implementation) is well-defined and appears complete, including iterators and handling for various map elements.

---
## CORE-04 - Implement Action/History (Undo/Redo) System

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/actionqueue.h`
- `wxwidgets/actionqueue.cpp`
- `wxwidgets/editor.cpp` (Undo/Redo menu handling)

**Inferred Qt6 Files:**
- `Project_QT/src/editor_logic/EditorController.h` (uses QUndoStack)
- `Project_QT/src/editor_logic/EditorController.cpp`
- `Project_QT/src/editor_logic/commands/` (directory containing many QUndoCommand subclasses)

**Details/Summary:**
The system uses Qt's `QUndoStack` managed by `EditorController`. A comprehensive suite of command classes inheriting `QUndoCommand` is present in `editor_logic/commands/`, covering various map operations.
---
---
## CORE-07-MapIO - Port OTBM Input/Output System

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/mapio_otbm.cpp`
- `wxwidgets/mapio_otbm.h`
- `wxwidgets/house_io.cpp`
- `wxwidgets/spawn_io.cpp`
- `wxwidgets/mapon_otbm.cpp`
- `wxwidgets/mapon_otbm.h`
- `wxwidgets/mapon_otb.cpp`
- `wxwidgets/mapon_otb.h`
- `common/config.h` (for some I/O related flags)

**Inferred Qt6 Files:**
- `Project_QT/src/core/io/IMapIO.h`
- `Project_QT/src/core/io/OtbmMapIO.h`
- `Project_QT/src/core/io/OtbmMapIO.cpp`
- `Project_QT/src/core/io/BinaryNode.h`
- `Project_QT/src/core/io/BinaryNode.cpp`
- `Project_QT/src/core/io/NodeFileReadHandle.h`
- `Project_QT/src/core/io/NodeFileReadHandle.cpp`
- `Project_QT/src/core/io/NodeFileWriteHandle.h`
- `Project_QT/src/core/io/NodeFileWriteHandle.cpp`
- `Project_QT/src/core/io/DiskNodeFileReadHandle.h`
- `Project_QT/src/core/io/DiskNodeFileReadHandle.cpp`
- `Project_QT/src/core/io/DiskNodeFileWriteHandle.h`
- `Project_QT/src/core/io/DiskNodeFileWriteHandle.cpp`
- `Project_QT/src/core/io/MemoryNodeFileReadHandle.h`
- `Project_QT/src/core/io/MemoryNodeFileReadHandle.cpp`
- `Project_QT/src/core/io/MemoryNodeFileWriteHandle.h`
- `Project_QT/src/core/io/MemoryNodeFileWriteHandle.cpp`

**Details/Summary:**
The Qt6 codebase includes a comprehensive set of classes (`IMapIO`, `OtbmMapIO`, `BinaryNode`, various `NodeFileRead/WriteHandle` implementations) under `core/io/` for handling OTBM files, indicating a robust and complete I/O system.

---
## CORE-08-CreatureOutfit - Implement Creature Instance and Outfit Logic

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/creature.h`
- `wxwidgets/creature.cpp`
- `wxwidgets/outfit.cpp`
- `wxwidgets/outfit.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/creatures/Creature.h`
- `Project_QT/src/core/creatures/Creature.cpp`
- `Project_QT/src/core/assets/Outfit.h`
- `Project_QT/src/core/assets/CreatureData.h` (Defines creature properties, used by Creature instances)
- `Project_QT/src/core/Tile.h` (Creature is placed on a Tile)

**Details/Summary:**
`Creature.h/cpp` and `Outfit.h` (from `core/assets` for data part, `core/creatures` for instance) are well-implemented and integrated with `Tile` and `CreatureData`, covering creature instance logic and appearance.

---
## CORE-09-HouseSystem - Implement House Data Structures and Management

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/house.cpp`
- `wxwidgets/house.h`
- `wxwidgets/housetile.h`
- `wxwidgets/housetile.cpp`
- `wxwidgets/editor.cpp` (for HouseManager equivalent)

**Inferred Qt6 Files:**
- `Project_QT/src/core/houses/HouseData.h`
- `Project_QT/src/core/houses/HouseData.cpp`
- `Project_QT/src/core/houses/House.h`
- `Project_QT/src/core/houses/House.cpp`
- `Project_QT/src/core/houses/Houses.h`
- `Project_QT/src/core/houses/Houses.cpp` (Houses Manager)
- `Project_QT/src/core/map/Map.h` (Manages houses)
- `Project_QT/src/core/Tile.h` (Links to house via ID)

**Details/Summary:**
A complete system for house data (`HouseData`, `House` class) and management (`Houses` manager) is present in `core/houses/`. `Map` manages houses, and `Tile` links via `house_id`. I/O is also handled.

---
## CORE-10-SpawnSystem - Implement Spawn Data Structures

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/spawn.h`
- `wxwidgets/spawn.cpp`

**Inferred Qt6 Files:**
- `Project_QT/src/core/spawns/SpawnData.h`
- `Project_QT/src/core/spawns/SpawnData.cpp`
- `Project_QT/src/core/Tile.h` (Stores `SpawnData*` reference)
- `Project_QT/src/core/map/Map.h` (Manages list of all spawns)

**Details/Summary:**
`SpawnData.h/cpp` defines the structure for spawn information. `Tile` objects can hold a reference to `SpawnData`, and `Map` manages the list of all spawns. Core data handling is present, though some specific behaviors from the original RME regarding spawn areas might differ.

---
## CORE-11-WaypointSystem - Implement Waypoint Data Structures and Management

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/waypoint.cpp`
- `wxwidgets/waypoint.h`
- `wxwidgets/editor.cpp` (for WaypointManager equivalent)

**Inferred Qt6 Files:**
- `Project_QT/src/core/navigation/WaypointData.h`
- `Project_QT/src/core/navigation/WaypointData.cpp`
- `Project_QT/src/core/waypoints/Waypoint.h` (Likely a wrapper or UI model, `WaypointData` is core)
- `Project_QT/src/core/waypoints/WaypointManager.h`
- `Project_QT/src/core/waypoints/WaypointManager.cpp`
- `Project_QT/src/core/map/Map.h` (Manages waypoints)

**Details/Summary:**
`WaypointData.h/cpp` (in `core/navigation/`) defines waypoint properties. `WaypointManager.h/cpp` (in `core/waypoints/`) handles their management. `Map` integrates this system. Appears complete.

---
## CORE-12-ComplexItemSystem - Design and Implement Complex Item Handling

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/item.h`
- `wxwidgets/item.cpp`
- `wxwidgets/container.h`
- `wxwidgets/container.cpp`
- `wxwidgets/teleport.h`
- `wxwidgets/teleport.cpp`
- `wxwidgets/door.h`
- `wxwidgets/door.cpp`
- `wxwidgets/podium.h`
- `wxwidgets/podium.cpp`

**Inferred Qt6 Files:**
- `Project_QT/src/core/Item.h`
- `Project_QT/src/core/Item.cpp`
- `Project_QT/src/core/items/ContainerItem.h`
- `Project_QT/src/core/items/ContainerItem.cpp`
- `Project_QT/src/core/items/TeleportItem.h`
- `Project_QT/src/core/items/TeleportItem.cpp`
- `Project_QT/src/core/items/DoorItem.h`
- `Project_QT/src/core/items/DoorItem.cpp`
- `Project_QT/src/core/items/PodiumItem.h`
- `Project_QT/src/core/items/PodiumItem.cpp`
- `Project_QT/src/core/items/DepotItem.h`
- `Project_QT/src/core/items/DepotItem.cpp`

**Details/Summary:**
The strategy of using derived item classes (e.g., `ContainerItem`, `TeleportItem`, `DoorItem`, `PodiumItem`, `DepotItem`) is implemented in `core/items/`. The base `Item::create` factory likely handles instantiation, and virtual methods support specialized attributes/serialization.

---
## CORE-13-TownSystem - Implement Town Data Structures and Management

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/town.cpp`
- `wxwidgets/town.h`
- `wxwidgets/editor.cpp` (for TownManager equivalent)

**Inferred Qt6 Files:**
- `Project_QT/src/core/world/TownData.h`
- `Project_QT/src/core/world/TownData.cpp`
- `Project_QT/src/core/map/Map.h` (Manages towns)
- `Project_QT/src/core/houses/HouseData.h` (Links to Town via townId)

**Details/Summary:**
`TownData.h/cpp` is implemented in `core/world/`. `Map` manages towns, and `HouseData` links to towns via `townId`. I/O for towns is also handled.

---
## CORE-14-MaterialSystem - Implement Material System for Brushes

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/materials.xml` (and other specific XMLs like `walls.xml`, `carpets.xml`)
- `wxwidgets/material.cpp`
- `wxwidgets/material.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/assets/MaterialData.h`
- `Project_QT/src/core/assets/MaterialData.cpp`
- `Project_QT/src/core/assets/MaterialManager.h`
- `Project_QT/src/core/assets/MaterialManager.cpp`
- `Project_QT/src/core/assets/AssetManager.h` (Integrates MaterialManager)

**Details/Summary:**
A detailed `MaterialData` structure (using `std::variant` for specifics like `MaterialGroundSpecifics`, `MaterialWallSpecifics`, etc.) and `MaterialManager` (using `QXmlStreamReader` for parsing material XMLs) are implemented in `core/assets/` and integrated with `AssetManager`.

---
## CORE-15-MapRegionSystem - Implement Generic Map Region Data and Management

**Status:** Mapping Unclear

**Original wxWidgets Files:**
- (Potentially parts of `wxwidgets/map.h/.cpp` or selection logic if regions were tied to that)

**Inferred Qt6 Files:**
- N/A (No direct equivalent like `MapRegionData.h` or `MapRegionManager.h` found)

**Details/Summary:**
No direct Qt6 equivalent of a generic map region system or `MapRegionData` was found. The original purpose might be covered by other features (e.g., extended selection, named areas if implemented later) or needs clarification for a direct port.

---
## DOCS-01 - Setup Doxygen Documentation Generation

**Status:** Not Started

**Original wxWidgets Files:**
- N/A (New requirement for Qt6 project)

**Inferred Qt6 Files:**
- `Project_QT/Doxyfile` (Expected, but not found)
- `Project_QT/CMakeLists.txt` (for Doxygen integration commands, not found)

**Details/Summary:**
No Doxyfile or specific CMake integration for Doxygen documentation generation was found. Source code comments exist but are not consistently in Doxygen style for automated processing.

---
## DOCS-02 - Create User Manual Content and Structure

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/data/manual/*` (various HTML/image files for old manual)
- `wxwidgets/editor.cpp` (Help menu logic)

**Inferred Qt6 Files:**
- `docs/UserManual.md` (Expected, but not found)
- `Project_QT/src/resources/menubar.xml` (Exists, contains menu structure that could inform manual sections)

**Details/Summary:**
A `UserManual.md` file was not found in the repository. While `menubar.xml` exists and could provide a table of contents, the actual user manual content has not been created.

---
## FINAL-01 - Integrate Core Logic with UI Shells

**Status:** Partially Implemented

**Original wxWidgets Files:**
- `wxwidgets/application.cpp`
- `wxwidgets/editor.cpp`
- `wxwidgets/mainframe.cpp`
- `wxwidgets/mainframe.h`

**Inferred Qt6 Files:**
- `Project_QT/src/editor_logic/EditorController.h`
- `Project_QT/src/editor_logic/EditorController.cpp`
- `Project_QT/src/ui/MainWindow.h`
- `Project_QT/src/ui/MainWindow.cpp`
- `Project_QT/src/main.cpp`

**Details/Summary:**
`EditorController` (acting as an application context) and basic UI shells like `MainWindow` exist. However, the full integration of backend actions, services, and UI elements (palettes, toolbars, menus) is minimal at this stage.

---
## FINAL-02 - Implement File Menu Operations

**Status:** Partially Implemented

**Original wxWidgets Files:**
- `wxwidgets/mainframe.cpp` (File menu event handlers)
- `wxwidgets/editor.cpp` (Map loading/saving logic)

**Inferred Qt6 Files:**
- `Project_QT/src/ui/MainWindow.h`
- `Project_QT/src/ui/MainWindow.cpp`
- `Project_QT/src/editor_logic/EditorController.h` (Methods for New, Open, Save)
- `Project_QT/src/core/io/OtbmMapIO.h` (Used by save/load logic)

**Details/Summary:**
`MainWindow` has some logic for recent files. `EditorController` might have placeholder methods. While `OtbmMapIO` is robust, the full UI wiring for New, Open, Save, Save As, Revert, Import, Export operations, including dialogs and error handling, is not yet complete.

---
## FINAL-03 - Implement Edit Menu & Preferences Dialog

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/mainframe.cpp` (Edit menu event handlers)
- `wxwidgets/editor.cpp` (Undo/Redo, Copy/Paste logic)
- `wxwidgets/dialogs/preferencesdlg.cpp`
- `wxwidgets/dialogs/preferencesdlg.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/MainWindow.h` (Edit menu signals/slots expected)
- `Project_QT/src/editor_logic/EditorController.h` (Undo/Redo already covered by QUndoStack)
- `Project_QT/src/core/selection/SelectionManager.h` (for selection-based edit ops)
- `Project_QT/src/core/clipboard/ClipboardManager.h` (Expected, but status was 'Partially Implemented' from CORE-05)
- `Project_QT/src/ui/dialogs/PreferencesDialog.h` (Expected, but not found)

**Details/Summary:**
Edit menu actions (Undo, Redo, Cut, Copy, Paste, Clear Selection) are not fully wired to UI elements yet. Clipboard functionality (CORE-05) is incomplete. The Preferences Dialog Qt6 equivalent was not found.

---
## FINAL-04 - Implement View Menu & Tools Integration

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/mainframe.cpp` (View menu, toolbar event handlers)
- Various palette and tool-specific UI files from wxwidgets.

**Inferred Qt6 Files:**
- `Project_QT/src/ui/MainWindow.h` (View menu, toolbar signals/slots expected)
- `Project_QT/src/ui/palettes/` (Directory for various palettes, e.g., ItemPalette, TerrainPalette - content not yet analyzed in detail)
- `Project_QT/src/editor_logic/EditorController.h` (Methods to toggle palettes/views)

**Details/Summary:**
View menu options (show/hide palettes, toolbars, grid, etc.) and toolbar actions are not yet implemented or wired to their corresponding functionalities and UI elements. Specific palettes exist but their full integration is pending.

---
## FINAL-05 - Implement Live Client UI & Integration

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/liveclient.cpp`
- `wxwidgets/liveclient.h`
- `wxwidgets/dialogs/connectdlg.cpp`
- `wxwidgets/dialogs/connectdlg.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/dialogs/LiveConnectionDialog.h` (Expected, but not found)
- `Project_QT/src/ui/widgets/QtLiveLogTab.h` (Expected, but not found)
- `Project_QT/src/network/QtLiveClient.h` (Expected, but not found or incomplete from NET-03)
- `Project_QT/src/ui/MainWindow.h` (Integration points for Live Client UI)

**Details/Summary:**
Specific UI elements for Live Client functionality (connection dialog, log tab) and the `QtLiveClient` class for managing the connection and data sync were not found or are incomplete based on NET-03 status.

---
## FINAL-06 - Implement About & Welcome Dialogs

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/dialogs/aboutdlg.cpp`
- `wxwidgets/dialogs/aboutdlg.h`
- `wxwidgets/dialogs/welcomedlg.cpp`
- `wxwidgets/dialogs/welcomedlg.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/dialogs/AboutDialog.h` (Expected, but not found)
- `Project_QT/src/ui/dialogs/WelcomeDialog.h` (Expected, but not found)

**Details/Summary:**
The Qt6 equivalents for the About and Welcome dialogs were not found in the `Project_QT/src/ui/dialogs/` directory.

---
## FINAL-07 - Apply Qlementine Styling and Theming

**Status:** Not Started

**Original wxWidgets Files:**
- N/A (Qlementine is a new Qt6 feature)

**Inferred Qt6 Files:**
- `Project_QT/src/main.cpp` (Expected to initialize Qlementine style)
- `Project_QT/src/ui/MainWindow.cpp` (May apply specific Qlementine attributes)
- `qlementine/` (Qlementine library directory, already integrated in CMake)

**Details/Summary:**
The `main.cpp` file is currently a basic placeholder and does not include logic to initialize and apply the Qlementine style to the application. While Qlementine is linked, it's not yet activated.

---
## LOGIC-01 - Implement Core Drawing/Deletion Controller

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/editor.cpp` (Handles brush application, item placement/removal)
- `wxwidgets/brush.cpp` (Base brush logic)

**Inferred Qt6 Files:**
- `Project_QT/src/editor_logic/EditorController.h`
- `Project_QT/src/editor_logic/EditorController.cpp`
- `Project_QT/src/core/brush/Brush.h` (Base class for brushes)
- `Project_QT/src/editor_logic/commands/BrushStrokeCommand.h`
- `Project_QT/src/editor_logic/commands/BrushStrokeCommand.cpp`
- `Project_QT/src/editor_logic/commands/DeleteSelectionCommand.h` (or similar for deletion)

**Details/Summary:**
`EditorController` orchestrates brush application via `applyBrushStroke`, which uses the active brush. Commands like `BrushStrokeCommand` and `DeleteSelectionCommand` (or similar for deletion actions) handle the modifications and undo/redo. Core drawing and deletion mechanisms are in place.

---
## LOGIC-02 - Implement Bounding-Box Selection Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/map_display.cpp` (Mouse event handling for selection rectangle)
- `wxwidgets/editor.cpp` (Selection update logic)
- `wxwidgets/selection.h`
- `wxwidgets/selection.cpp`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/widgets/MapView.h` (Expected to handle mouse events for selection)
- `Project_QT/src/ui/widgets/MapView.cpp`
- `Project_QT/src/editor_logic/EditorController.h`
- `Project_QT/src/editor_logic/EditorController.cpp` (Method `performBoundingBoxSelection`)
- `Project_QT/src/editor_logic/commands/BoundingBoxSelectCommand.h`
- `Project_QT/src/editor_logic/commands/BoundingBoxSelectCommand.cpp`
- `Project_QT/src/core/selection/SelectionManager.h`

**Details/Summary:**
`EditorController::performBoundingBoxSelection` contains detailed logic for iterating tiles within the selection rectangle, considering floor settings and compensated select. It uses `BoundingBoxSelectCommand` to manage the actual selection changes via `SelectionManager`. `MapView` (not fully analyzed here) would be responsible for drawing the rectangle. Floor compensation logic details were noted as needing full verification.
---
---
## LOGIC-03 - Implement Cut/Copy/Paste/Delete/Move Selection Logic

**Status:** Partially Implemented

**Original wxWidgets Files:**
- `wxwidgets/editor.cpp` (Copy, Paste, Delete, Cut, Move methods)
- `wxwidgets/selection.cpp`
- `wxwidgets/clipboard.cpp`
- `wxwidgets/clipboard.h`

**Inferred Qt6 Files:**
- `Project_QT/src/editor_logic/EditorController.h`
- `Project_QT/src/editor_logic/EditorController.cpp`
- `Project_QT/src/core/selection/SelectionManager.h`
- `Project_QT/src/core/clipboard/ClipboardManager.h` (from CORE-05, was Partially Implemented)
- `Project_QT/src/editor_logic/commands/DeleteSelectionCommand.h` (Exists)
- `Project_QT/src/editor_logic/commands/CopySelectionCommand.h` (Expected, not found/verified)
- `Project_QT/src/editor_logic/commands/CutSelectionCommand.h` (Expected, not found/verified)
- `Project_QT/src/editor_logic/commands/PasteCommand.h` (Expected, not found/verified)
- `Project_QT/src/editor_logic/commands/MoveCommand.h` (Expected, not found/verified)

**Details/Summary:**
Depends on `ClipboardManager` (CORE-05, which was Partially Implemented). `DeleteSelectionCommand` exists. Specific commands for Copy, Cut, Paste, and Move were not found or verified as fully implemented.

---
## LOGIC-04 - Implement Waypoint System Logic

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/editor.cpp` (Waypoint manipulation logic)
- `wxwidgets/waypoint_brush.cpp` (Interface for placement)

**Inferred Qt6 Files:**
- `Project_QT/src/core/waypoints/WaypointManager.h`
- `Project_QT/src/core/waypoints/WaypointManager.cpp`
- `Project_QT/src/editor_logic/EditorController.h` (Methods for waypoint operations)
- `Project_QT/src/editor_logic/commands/AddWaypointCommand.h`
- `Project_QT/src/editor_logic/commands/MoveWaypointCommand.h`
- `Project_QT/src/editor_logic/commands/RemoveWaypointCommand.h` (Expected if full CRUD)

**Details/Summary:**
Covered by CORE-11 (Waypoint Data/Manager), BRUSH-LOGIC-Waypoint (tool-based placement/movement), and associated commands like `AddWaypointCommand` and `MoveWaypointCommand`. The system for managing and interacting with waypoints appears complete.

---
## LOGIC-05 - Implement House System Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/editor.cpp` (House manipulation logic)
- `wxwidgets/house_brush.cpp` (Interface for assignment)

**Inferred Qt6 Files:**
- `Project_QT/src/core/houses/Houses.h` (House Manager)
- `Project_QT/src/core/houses/Houses.cpp`
- `Project_QT/src/editor_logic/EditorController.h` (Methods for house operations)
- `Project_QT/src/editor_logic/commands/SetHouseTileCommand.h`
- `Project_QT/src/core/brush/HouseBrush.h`

**Details/Summary:**
Covered by CORE-09 (House Data/Manager), BRUSH-LOGIC-House (brush-based assignment), and `SetHouseTileCommand`. Core logic for assigning tiles to houses and managing house properties is in place.

---
## LOGIC-06 - Integrate House/Waypoint Brushes/Tools with Editor

**Status:** Partially Implemented

**Original wxWidgets Files:**
- `wxwidgets/editor.cpp` (Main interaction controller)
- `wxwidgets/mainframe.cpp` (UI for selecting brushes/tools)

**Inferred Qt6 Files:**
- `Project_QT/src/editor_logic/EditorController.h`
- `Project_QT/src/editor_logic/EditorController.cpp`
- `Project_QT/src/ui/MainWindow.h` (Expected to host palettes/tool selectors)
- `Project_QT/src/ui/palettes/` (Palettes for selecting house/waypoint to use with tool)

**Details/Summary:**
Core brush/tool logic for House and Waypoint operations exists (as analyzed in BRUSH-LOGIC-House, BRUSH-LOGIC-HouseExit, BRUSH-LOGIC-Waypoint). `EditorController` can invoke them. However, full UI wiring for selecting the current house/waypoint for these tools is part of specific UI palette tasks and may not be complete.

---
## LOGIC-07 - Implement Creature/Spawn System Logic

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/editor.cpp` (Creature/Spawn manipulation)
- `wxwidgets/creature_brush.cpp`
- `wxwidgets/spawn_brush.cpp`

**Inferred Qt6 Files:**
- `Project_QT/src/editor_logic/EditorController.h`
- `Project_QT/src/core/assets/CreatureDatabase.h`
- `Project_QT/src/core/spawns/SpawnData.h`
- `Project_QT/src/core/brush/CreatureBrush.h`
- `Project_QT/src/core/brush/SpawnBrush.h`
- `Project_QT/src/editor_logic/commands/AddCreatureCommand.h`
- `Project_QT/src/editor_logic/commands/RemoveCreatureCommand.h`
- `Project_QT/src/editor_logic/commands/RecordAddSpawnCommand.h` (or similar for spawns)

**Details/Summary:**
Covered by CORE-08 (Creature Data), CORE-10 (Spawn Data), BRUSH-LOGIC-Creature, BRUSH-LOGIC-Spawn, and associated commands. Logic for placing creatures and defining spawn areas is substantially ported.

---
## LOGIC-08 - Implement Search Functionality (Items, Creatures, etc.)

**Status:** Partially Implemented

**Original wxWidgets Files:**
- `wxwidgets/item_find.cpp`
- `wxwidgets/item_find.h`
- (Similar search for creatures, NPCs etc. if existed)

**Inferred Qt6 Files:**
- `Project_QT/src/ui/dialogs/ItemFinderDialogQt.h`
- `Project_QT/src/ui/dialogs/ItemFinderDialogQt.cpp`
- `Project_QT/src/editor_logic/EditorController.h` (Potential search methods)
- `Project_QT/src/core/assets/ItemDatabase.h` (Backend for item search)
- `Project_QT/src/core/assets/CreatureDatabase.h` (Backend for creature search)

**Details/Summary:**
An `ItemFinderDialogQt` exists (UI-11), providing a front-end for item searching. However, comprehensive backend search logic across various data types (items, creatures, NPCs, map features) and its integration into `EditorController` was not fully verified.

---
## LOGIC-09 - Implement Map-Wide Tools (Rotate, Resize, etc.)

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/map.cpp` (Methods for map-wide operations)
- `wxwidgets/editor.cpp` (Invoking these operations)

**Inferred Qt6 Files:**
- `Project_QT/src/core/map/Map.h`
- `Project_QT/src/core/map/Map.cpp`
- `Project_QT/src/editor_logic/EditorController.h` (Methods to trigger map tools)
- `Project_QT/src/editor_logic/commands/MapOperationCommand.h` (Expected generic or specific commands)

**Details/Summary:**
Map-wide tools like rotate, resize, clear invalid items, replace items, etc., are described as needing methods in the `Map` class or new dedicated commands. These functionalities were not found to be implemented yet in `EditorController` or as specific commands.

---
## NET-01 - Define Network Protocol and Serialization

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/liveclient_protocol.h`
- `wxwidgets/networkmessage.cpp`
- `wxwidgets/networkmessage.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/network/live_packets.h`
- `Project_QT/src/core/network/NetworkMessage.h`
- `Project_QT/src/core/network/NetworkMessage.cpp`
- `Project_QT/src/core/network/MapProtocolCodec.h`
- `Project_QT/src/core/network/MapProtocolCodec.cpp`

**Details/Summary:**
Core classes for network communication (`NetworkMessage`, `MapProtocolCodec`) and packet definitions (`live_packets.h`) are present in `core/network/`. Serialization for complex data like map sectors was noted as a complex part but the foundational protocol elements seem to be in place.

---
## NET-02 - Implement Live Server for Collaborative Editing

**Status:** Partially Implemented

**Original wxWidgets Files:**
- `wxwidgets/liveserver.cpp`
- `wxwidgets/liveserver.h`
- `wxwidgets/serverclient.cpp`
- `wxwidgets/serverclient.h`

**Inferred Qt6 Files:**
- `Project_RMELiveServer/src/LiveServer.h`
- `Project_RMELiveServer/src/LiveServer.cpp`
- `Project_RMELiveServer/src/ServerClient.h`
- `Project_RMELiveServer/src/ServerClient.cpp`
- `Project_RMELiveServer/src/main.cpp`
- `Project_RMELiveServer/CMakeLists.txt`

**Details/Summary:**
A separate project `Project_RMELiveServer/` exists. It contains a QTcpServer/Socket based server structure (`LiveServer`, `ServerClient`). It appears to load map/assets and handle basic client connections. However, the core logic for map state synchronization and propagating changes between multiple clients seems incomplete.

---
## NET-03 - Implement Live Client and UI Integration

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/liveclient.cpp`
- `wxwidgets/liveclient.h`
- `wxwidgets/dialogs/connectdlg.cpp` (UI for connection)

**Inferred Qt6 Files:**
- `Project_QT/src/network/QtLiveClient.h` (Expected, but not found)
- `Project_QT/src/network/QtLiveClient.cpp` (Expected, but not found)
- `Project_QT/src/ui/dialogs/LiveConnectionDialog.h` (Expected for FINAL-05, not found)
- `Project_QT/src/ui/widgets/QtLiveLogTab.h` (Expected for FINAL-05, not found)

**Details/Summary:**
The client-side logic (`QtLiveClient`) for handling the live connection, receiving updates, and sending changes was not found. UI elements for connecting and logging (part of FINAL-05) are also pending.

---
## PALETTE-BrushList - Implement Brush Palette (List & Selection)

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/brush_palette.cpp`
- `wxwidgets/brush_palette.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/palettes/BrushPaletteWidget.h` (Expected)
- `Project_QT/src/ui/palettes/BrushPaletteWidget.cpp` (Expected)
- `Project_QT/src/core/brush/BrushManagerService.h` (To get available brushes)

**Details/Summary:**
The UI widget for listing available brushes and allowing selection was not found or analyzed in `Project_QT/src/ui/palettes/`.

---
## PALETTE-BrushSettings - Implement Brush Settings Palette

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/brush_options_palette.cpp`
- `wxwidgets/brush_options_palette.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/palettes/BrushSettingsPaletteWidget.h` (Expected)
- `Project_QT/src/ui/palettes/BrushSettingsPaletteWidget.cpp` (Expected)
- `Project_QT/src/core/brush/BrushSettings.h` (Data object for settings)

**Details/Summary:**
The UI widget for displaying and modifying settings specific to the currently selected brush (e.g., size, shape, specific flags like "place door" for WallBrush) was not found or analyzed.

---
## PALETTE-Creature - Implement Creature Palette

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/creature_palette.cpp`
- `wxwidgets/creature_palette.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/palettes/CreaturePaletteWidget.h` (Expected)
- `Project_QT/src/ui/palettes/CreaturePaletteWidget.cpp` (Expected)
- `Project_QT/src/core/assets/CreatureDatabase.h` (To get creature list)

**Details/Summary:**
The UI widget for listing creatures (e.g., from `CreatureDatabase`) and selecting one for placement with the Creature Brush was not found or analyzed.

---
## PALETTE-Ground - Implement Ground/Terrain Palette

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/terrain_palette.cpp`
- `wxwidgets/terrain_palette.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/palettes/GroundPaletteWidget.h` (Expected)
- `Project_QT/src/ui/palettes/GroundPaletteWidget.cpp` (Expected)
- `Project_QT/src/core/assets/MaterialManager.h` (To get ground materials)

**Details/Summary:**
The UI widget for displaying available ground/terrain types (likely from `MaterialManager` filtering for ground materials) and selecting one for the Ground Brush was not found or analyzed.

---
## PALETTE-House - Implement House Palette & Tools

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/house_palette.cpp`
- `wxwidgets/house_palette.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/palettes/HousePaletteWidget.h` (Expected)
- `Project_QT/src/ui/palettes/HousePaletteWidget.cpp` (Expected)
- `Project_QT/src/core/houses/Houses.h` (House Manager, to list houses)

**Details/Summary:**
The UI widget for listing existing houses, creating new ones, selecting a house for the House Brush, or initiating house-related tools (like Set Exit) was not found or analyzed.

---
## PALETTE-Item - Implement Item Palette (OTB based)

**Status:** Blocked/Issue

**Original wxWidgets Files:**
- `wxwidgets/item_palette.cpp`
- `wxwidgets/item_palette.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/palettes/ItemPaletteWidget.h` (Expected)
- `Project_QT/src/ui/palettes/ItemPaletteWidget.cpp` (Expected)
- `Project_QT/src/core/assets/ItemDatabase.h` (To get item list)
- `Project_QT/src/core/sprites/SpriteManager.h` (For item icons)

**Details/Summary:**
This palette is blocked by `RENDER-03` (Sprite Rendering), which is currently "Not Started". Without sprite rendering capabilities, an item palette displaying item icons cannot be effectively implemented.

---
## PALETTE-Material - Implement Material/Texture Palette

**Status:** Not Started

**Original wxWidgets Files:**
- (May not have a direct equivalent, materials were often tied to specific brush palettes)

**Inferred Qt6 Files:**
- `Project_QT/src/ui/palettes/MaterialPaletteWidget.h` (Expected)
- `Project_QT/src/ui/palettes/MaterialPaletteWidget.cpp` (Expected)
- `Project_QT/src/core/assets/MaterialManager.h` (To get material list)

**Details/Summary:**
A generic UI widget for browsing and selecting from the `MaterialManager` (e.g., for brushes that use materials like Carpet, Ground, Wall, Table) was not found or analyzed.

---
## PALETTE-Minimap - Implement Minimap Palette/View

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/minimap_dialog.cpp`
- `wxwidgets/minimap_dialog.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/widgets/MinimapView.h` (Expected, linked from UI-MinimapView)
- `Project_QT/src/ui/widgets/MinimapView.cpp` (Expected)
- `Project_QT/src/ui/palettes/MinimapPaletteWidget.h` (If it's a dockable palette)

**Details/Summary:**
The UI widget for displaying a minimap of the current map was not found or analyzed. This is also covered by task `UI-MinimapView`.

---
## REFACTOR-01 - Decouple Globals and Manager Classes

**Status:** Partially Implemented

**Original wxWidgets Files:**
- (Spread across many files, e.g., `editor.h`, `application.h` for global objects like `g_editor`, `g_items`)

**Inferred Qt6 Files:**
- `Project_QT/src/editor_logic/EditorController.h` (Acts as a context/service locator)
- Various Manager classes in `Project_QT/src/core/assets/`, `core/houses/`, `core/waypoints/` etc.
- Constructor injection patterns in various classes.

**Details/Summary:**
Many parts of the Qt6 codebase use manager classes (`AssetManager`, `BrushManagerService`, `AppSettings`, `Houses`, `WaypointManager`) and `EditorController` often provides access to these, acting as a form of dependency injection or service locator. This indicates significant progress in decoupling. However, a full audit to confirm the complete elimination of all global-like static access patterns and consistent use of DI/service location was not performed.

---
## REFACTOR-02 - Implement Profiling Strategy and Tools

**Status:** Not Started

**Original wxWidgets Files:**
- N/A (New requirement)

**Inferred Qt6 Files:**
- N/A (Code for profiling tools like Tracy or Optick would be integrated, or built-in Qt profiling used)

**Details/Summary:**
This is a process-oriented task. No specific code for integrating profiling tools (e.g., Tracy, Optick) or enabling advanced Qt profiling features was found or expected at this stage of primarily porting core logic.
---
---
## REFACTOR-03 - Profile and Optimize Rendering Performance

**Status:** Blocked/Issue

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- N/A (Profiling is a process; optimizations would modify existing rendering code)

**Details/Summary:**
This task is blocked by REFACTOR-02 (Implement Profiling Strategy), which was "Not Started". Profiling needs to occur before targeted optimizations can be made.

---
## RENDER-01 - Implement MapView Core (OpenGL Canvas, Pan, Zoom)

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- `wxwidgets/map_display.cpp`
- `wxwidgets/map_display.h`
- `wxwidgets/rmepanel.cpp` (for zoom controls)
- `wxwidgets/rmepanel.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/widgets/MapView.h`
- `Project_QT/src/ui/widgets/MapView.cpp`

**Details/Summary:**
`MapView.h/cpp` in `Project_QT/src/ui/widgets/` handles OpenGL setup, panning, zooming, floor changes, and coordinate transformations, forming a solid core for map display.

---
## RENDER-02 - Implement Basic Tile Rendering (Ground, Items)

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/map_drawer.cpp`
- `wxwidgets/map_drawer.h`
- `wxwidgets/map_display.cpp` (calls drawing routines)

**Inferred Qt6 Files:**
- `Project_QT/src/ui/widgets/MapView.cpp` (Specifically `paintGL` or similar method)
- `Project_QT/src/core/Tile.h` (Data to be rendered)
- `Project_QT/src/core/Item.h` (Data to be rendered)

**Details/Summary:**
While the `MapView` OpenGL canvas (RENDER-01) is set up, the actual rendering logic within `MapView::paintGL` (or equivalent) to draw map tiles (ground, items based on `Tile` data) was not confirmed as complete. This depends on RENDER-03 for sprites.

---
## RENDER-03 - Implement Sprite Rendering and Management

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/spritemanager.cpp`
- `wxwidgets/spritemanager.h`
- `wxwidgets/map_drawer.cpp` (uses sprites)

**Inferred Qt6 Files:**
- `Project_QT/src/core/sprites/TextureManagerQt.h` (Expected name, or similar)
- `Project_QT/src/core/sprites/TextureManagerQt.cpp` (Expected name, or similar)
- `Project_QT/src/core/sprites/SpriteManager.h` (May need adaptation for Qt textures)
- `Project_QT/src/ui/widgets/MapView.cpp` (To use the texture manager for rendering)

**Details/Summary:**
No `TextureManagerQt` or equivalent system for loading sprite sheets into Qt textures and rendering individual sprites (for items, creatures, effects) was found. This is critical for map rendering and item palettes.

---
## RENDER-04-LightingSystem - Implement Tile Lighting System

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/map_drawer.cpp` (Lighting calculations)
- `wxwidgets/lightmap.cpp`
- `wxwidgets/lightmap.h`

**Inferred Qt6 Files:**
- `Project_QT/src/core/map/LightmapManager.h` (Expected, or similar)
- `Project_QT/src/core/map/LightmapManager.cpp` (Expected)
- `Project_QT/src/ui/widgets/MapView.cpp` (To apply lighting during rendering)

**Details/Summary:**
No specific system for calculating or rendering tile lighting (ambient, torch lights) was found in the Qt6 codebase.

---
## TEST-01 - Unit Tests for Core Data Structures

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- N/A (New requirement for Qt6 project)

**Inferred Qt6 Files:**
- `Project_QT/src/tests/core/TestItem.cpp`
- `Project_QT/src/tests/core/TestTile.cpp`
- `Project_QT/src/tests/core/TestPosition.cpp`
- `Project_QT/src/tests/core/TestMap.cpp`
- `Project_QT/src/tests/core/TestFloor.cpp`
- `Project_QT/src/tests/core/TestHouse.cpp`
- `Project_QT/src/tests/core/TestWaypoint.cpp`
- `Project_QT/src/tests/core/TestSpawn.cpp`

**Details/Summary:**
Test files for core data structures like `Item`, `Tile`, `Position`, `Map`, `Floor`, `House`, `Waypoint`, and `Spawn` were found in `Project_QT/src/tests/core/`, indicating comprehensive unit testing for these components.

---
## TEST-02 - Unit Tests for Asset Loading and Management

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- `Project_QT/src/tests/core/assets/TestItemDatabase.cpp`
- `Project_QT/src/tests/core/assets/TestCreatureDatabase.cpp`
- `Project_QT/src/tests/core/assets/TestSpriteManager.cpp`
- `Project_QT/src/tests/core/assets/TestMaterialManager.cpp`
- `Project_QT/src/tests/core/assets/TestClientVersionManager.cpp`

**Details/Summary:**
Test files for various asset management classes (`ItemDatabase`, `CreatureDatabase`, `SpriteManager`, `MaterialManager`, `ClientVersionManager`) were found in `Project_QT/src/tests/core/assets/`, suggesting good test coverage for asset loading.

---
## TEST-03 - Unit Tests for OTBM/OTMM Input/Output

**Status:** Partially Implemented

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- `Project_QT/src/tests/core/io/TestOtbmMapIO.cpp`
- `Project_QT/src/tests/core/io/TestOtmmMapIO.cpp` (Expected, but OTMM might be out of scope)

**Details/Summary:**
`TestOtbmMapIO.cpp` was found, indicating testing for OTBM I/O. OTMM (OpenTibia Map Merv) is a less common format, and tests for it were not found; its implementation might be out of scope or pending.

---
## TEST-04 - Unit Tests for Brush and Material Logic

**Status:** Fully Implemented/Polished

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- `Project_QT/src/tests/core/brush/TestCarpetBrush.cpp`
- `Project_QT/src/tests/core/brush/TestCreatureBrush.cpp`
- `Project_QT/src/tests/core/brush/TestDoodadBrush.cpp`
- `Project_QT/src/tests/core/brush/TestEraserBrush.cpp`
- `Project_QT/src/tests/core/brush/TestGroundBrush.cpp`
- `Project_QT/src/tests/core/brush/TestHouseBrush.cpp`
- `Project_QT/src/tests/core/brush/TestRawBrush.cpp`
- `Project_QT/src/tests/core/brush/TestSpawnBrush.cpp`
- `Project_QT/src/tests/core/brush/TestTableBrush.cpp`
- `Project_QT/src/tests/core/brush/TestWallBrush.cpp`
- `Project_QT/src/tests/core/assets/TestMaterialParsing.cpp` (Or similar for material logic)

**Details/Summary:**
A comprehensive set of unit test files for various brushes (Carpet, Creature, Doodad, Eraser, Ground, House, RAW, Spawn, Table, Wall) and potentially material parsing logic was found in `Project_QT/src/tests/core/brush/` and `Project_QT/src/tests/core/assets/`.

---
## TEST-05 - Unit Tests for Map Actions and Selection Logic

**Status:** Partially Implemented

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- `Project_QT/src/tests/editor_logic/commands/TestAddCreatureCommand.cpp`
- `Project_QT/src/tests/editor_logic/commands/TestSetHouseTileCommand.cpp`
- `Project_QT/src/tests/editor_logic/commands/TestBoundingBoxSelectCommand.cpp`
- (Other command tests for actions like copy, paste, move, specific brush strokes)

**Details/Summary:**
Some tests for commands (e.g., `TestBoundingBoxSelectCommand`) exist. However, full coverage for all map actions and selection logic, especially complex operations like copy/paste (which depends on LOGIC-03, currently "Partially Implemented"), is likely not yet complete.

---
## TEST-06 - Unit Tests for Network Protocol and Serialization

**Status:** Not Started

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- `Project_QT/src/tests/core/network/TestNetworkMessage.cpp` (Expected)
- `Project_QT/src/tests/core/network/TestMapProtocolCodec.cpp` (Expected)

**Details/Summary:**
No specific unit test files for network protocol classes (`NetworkMessage`, `MapProtocolCodec`) were identified in `Project_QT/src/tests/core/network/`.

---
## TEST-07 - Rendering Integration Tests

**Status:** Blocked/Issue

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- `Project_QT/src/tests/rendering/TestMapViewRendering.cpp` (Expected)

**Details/Summary:**
This task is blocked by incomplete core rendering functionalities RENDER-02 (Basic Tile Rendering) and RENDER-03 (Sprite Rendering). Integration tests for rendering cannot be effectively created until these are implemented.

---
## TEST-08 - Live Collaboration Integration Tests

**Status:** Blocked/Issue

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- `Project_QT/src/tests/network/TestLiveCollaboration.cpp` (Expected)

**Details/Summary:**
This task is blocked by incomplete networking functionalities NET-02 (Live Server) and NET-03 (Live Client). End-to-end tests for live collaboration require these components to be functional.

---
## TEST-09 - UI Tests (Main Window, Dialogs, Palettes)

**Status:** Blocked/Issue

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- `Project_QT/src/tests/ui/TestMainWindowInteractions.cpp` (Expected)
- `Project_QT/src/tests/ui/TestItemFinderDialog.cpp` (Expected)
- (Various other UI component tests)

**Details/Summary:**
This task is blocked by the fact that many UI elements (specific dialogs, palettes beyond MainWindow and ItemFinderDialog) are "Not Started" or "Partially Implemented". UI tests require functional UI components.

---
## TEST-10 - Cross-Platform UI and System Tests

**Status:** Blocked/Issue

**Original wxWidgets Files:**
- N/A

**Inferred Qt6 Files:**
- N/A (This is more of a process/infrastructure task for CI)

**Details/Summary:**
This task is blocked by TEST-09 (UI Tests) and the general incomplete state of many UI components. Cross-platform testing would typically occur after individual UI components and the overall application are more stable.

---
## UI-01 - Implement MainWindow and Menu Structure

**Status:** Largely Implemented

**Original wxWidgets Files:**
- `wxwidgets/mainframe.h`
- `wxwidgets/mainframe.cpp`
- `wxwidgets/toolbar.cpp` (for menu/toolbar items)
- `wxwidgets/editor.cpp` (Menu item logic)

**Inferred Qt6 Files:**
- `Project_QT/src/ui/MainWindow.h`
- `Project_QT/src/ui/MainWindow.cpp`
- `Project_QT/src/resources/menubar.xml` (Defines menu structure)
- `Project_QT/src/main.cpp` (Instantiates MainWindow)

**Details/Summary:**
`MainWindow.h/cpp` exists and likely uses `menubar.xml` to define its menu structure. Basic window shell and menu setup appear to be in place. Radio menu items for modes were noted as a TODO in the YAML.

---
## UI-02 - Implement Toolbars and Palettes Framework

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/mainframe.h` (Toolbar creation)
- `wxwidgets/mainframe.cpp`
- `wxwidgets/palette.h`
- `wxwidgets/palette.cpp`
- `wxwidgets/toolbar.cpp`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/MainWindow.h` (Toolbar/docking setup)
- `Project_QT/src/ui/MainWindow.cpp`
- `Project_QT/src/ui/widgets/DockWidgetBase.h` (Expected for common palette features)

**Details/Summary:**
While Qt provides docking capabilities used by `MainWindow`, a specific framework or base classes for managing the creation, layout, and state of various toolbars and palettes (beyond what Qt offers by default) was not confirmed.

---
## UI-04 - Implement Properties Dialogs (Item, Tile, Map)

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/dialogs/itempropertiesdlg.cpp`
- `wxwidgets/dialogs/itempropertiesdlg.h`
- `wxwidgets/dialogs/tileflagsdlg.cpp`
- `wxwidgets/dialogs/tileflagsdlg.h`
- `wxwidgets/dialogs/mapstatisticsdlg.cpp`
- `wxwidgets/dialogs/mapstatisticsdlg.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/dialogs/ItemPropertiesDialog.h` (Expected)
- `Project_QT/src/ui/dialogs/TileFlagsDialog.h` (Expected)
- `Project_QT/src/ui/dialogs/MapPropertiesDialog.h` (Expected)

**Details/Summary:**
Specific Qt6 dialogs for editing item properties, tile flags, or map properties/statistics were not found in `Project_QT/src/ui/dialogs/`.

---
## UI-05 - Implement Brush & Material Editor Dialogs

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/dialogs/brusheditordlg.cpp`
- `wxwidgets/dialogs/brusheditordlg.h`
- `wxwidgets/dialogs/edit_material.cpp`
- `wxwidgets/dialogs/edit_material.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/dialogs/BrushEditorDialog.h` (Expected)
- `Project_QT/src/ui/dialogs/MaterialEditorDialog.h` (Expected)

**Details/Summary:**
Dialogs for creating or editing brush definitions (e.g., parameters for procedural brushes if any) or material properties were not found in `Project_QT/src/ui/dialogs/`.

---
## UI-06 - Implement Creature Palette & Editor Dialog

**Status:** Not Started

**Original wxWidgets Files:**
- `wxwidgets/creature_palette.cpp` (Palette part)
- `wxwidgets/creature_palette.h`
- `wxwidgets/dialogs/editcreaturedlg.cpp` (Editor dialog part)
- `wxwidgets/dialogs/editcreaturedlg.h`

**Inferred Qt6 Files:**
- `Project_QT/src/ui/palettes/CreaturePaletteWidget.h` (Palette - covered by PALETTE-Creature)
- `Project_QT/src/ui/dialogs/CreatureEditorDialog.h` (Expected for editing creature properties)

**Details/Summary:**
The creature editing dialog (for modifying creature types or instances placed on map) was not found. The palette part is covered by PALETTE-Creature, also "Not Started".
---
