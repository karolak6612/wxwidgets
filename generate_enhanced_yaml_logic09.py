import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "LOGIC-09",
    "section": "Brush Functionality", # As per WBS in prompt.md
    "title": "Finalize All Tool Implementations",
    "original_input_files": "editor.cpp", # Also implies map.cpp and main_menubar.cpp for context
    "analyzed_input_files": [
        "wxwidgets/editor.cpp",
        "wxwidgets/map.cpp", # For map-wide operations often initiated by editor tools
        "wxwidgets/main_menubar.cpp" # To identify tool actions
    ],
    "dependencies": [
        "LOGIC-08" # Depends on basic brush drawing modes and advanced brush logic being in place
    ],
    "current_functionality_summary": """\
`editor.cpp` (along with `map.cpp` for some map-wide operations) contains logic for various map editing tools that go beyond simple brush strokes. These are often triggered by menu actions defined in `main_menubar.cpp`. Examples include an area "Fill" tool (likely using a flood-fill algorithm), and several "Map Cleanup" operations like clearing invalid house tiles, removing duplicate items, and validating ground tile configurations. These operations are generally designed to be undoable by creating `Action` objects.\
""",
    "qt6_migration_steps": """\
1. Identify all distinct "tool" functionalities from `editor.cpp`, `map.cpp` (methods called by editor), and `main_menubar.cpp` (by examining `ActionID` handlers) that are not covered by previous `LOGIC-*` tasks (basic brush drawing, selection, copy/paste, specific brush integrations). Key examples include: Fill, Clear Invalid Houses, Remove Duplicate Items, Validate Grounds.
2. For each identified tool, port its core algorithm:
   - If the tool primarily orchestrates map changes or requires UI state (like the active brush for Fill), implement its logic in an `EditorController` method.
   - If the tool performs pure data manipulation on the map, its logic might be better suited as a method within the `mapcore::Map` class itself.
3. Ensure all ported tool methods operate exclusively on `mapcore` data structures and are independent of any UI framework code.
4. Integrate every map-modifying tool with `mapcore`'s `ActionQueue`:
   - The execution of each tool must be encapsulated within an `Action` or `BatchAction`.
   - All `Tile` or other map entity modifications performed by the tool must be recorded as `Change` objects within that action to ensure undo/redo capability.
5. Specific implementation for the "Fill" tool:
   - Port or re-implement a flood-fill algorithm (referencing `MapCanvas::floodFill` from `wxwidgets/map_display.cpp` for the original logic).
   - This algorithm should take a starting `Position` and use the active brush/item settings (from `BrushManager`) to determine what to fill with and the fill boundary conditions.
   - It must generate `Change` objects for all tiles it modifies and add them to a single `Action`.
6. Specific implementation for "Map Cleanup" operations (e.g., `clearInvalidHouseTiles`, `removeDuplicateItems`, `validateGrounds`):
   - Port their respective algorithms, which typically involve iterating through map tiles or items.
   - Ensure all modifications are captured in `Change` objects and grouped into a single `Action` for the entire cleanup operation.
7. In the `MainWindow` class, create `QAction`s for these tools (likely in the main menu). Connect these `QAction`s' `triggered()` signals to slots that invoke the corresponding `EditorController` methods.
8. If a tool requires specific parameters not readily available from `BrushManager` (e.g., an item ID for a targeted cleanup), a simple `QDialog` might be necessary to gather this input from the user before executing the tool's logic.\
""",
    "definition_of_done": """\
All remaining tool functionalities, such as "Fill" and various "Map Cleanup" operations (e.g., clear invalid houses, remove duplicate items, validate grounds), are fully implemented, functional, integrated with the `ActionQueue` for undo/redo, and accessible via the main menu.
Key requirements:
- The "Fill" tool correctly applies the active brush to a contiguous, enclosed area based on a flood-fill algorithm.
- Map cleanup operations (clear invalid house tiles, remove duplicate items, validate grounds, etc.) function as intended, modifying the map data correctly.
- All these tool operations are properly encapsulated in `Action`s and are undoable/redoable via the `ActionQueue`.
- The core logic for these tools resides in `EditorController` or `mapcore`'s `Map` class and is UI-independent.
- Menu items in `MainWindow` correctly trigger these tool functionalities.\
""",
    "boilerplate_coder_ai_prompt": """\
Port the logic for remaining map editing tools (e.g., "Fill", "Clear Invalid Houses", "Remove Duplicate Items", "Validate Grounds") from `editor.cpp` and `map.cpp` into methods within `EditorController` or, if purely data-centric, into `mapcore::Map`. Ensure all operations are undoable.
1.  **Identify Tool Logic**: Review `editor.cpp` (e.g., `Editor::clearInvalidHouseTiles`, `Editor::randomizeMap`) and `map.cpp` (e.g., `Map::cleanDuplicateItems`, `Map::validateGrounds`) for methods corresponding to menu actions not yet covered.
2.  **"Fill" Tool Implementation (in `EditorController`):**
    a.  `void performFill(const Position& startPos)`:
        i.  Get `activeBrush` and `brushParams` from `BrushManager`.
        ii. Implement a flood-fill algorithm (see `wxwidgets/map_display.cpp MapCanvas::floodFill` for original logic):
            - Start at `startPos`.
            - Identify connected tiles based on a matching criterion (e.g., same ground item ID as `startPos`, or tile is empty, depending on brush type). Do not cross 'border' items defined by the active brush or other criteria.
        iii. Create an `Action* fillAction = actionQueue->createAction(FILL_ACTION_ID);`.
        iv. For each tile identified by flood fill:
            `fillAction->addChange(new Change(tile->deepCopy())); // Before state`
            `activeBrush->draw(map, tile, &brushParams); // Apply current brush`
            `fillAction->addChange(new Change(tile->deepCopy())); // After state`
        v.  `actionQueue->addAction(fillAction);`
3.  **Map Cleanup Tools (example for `EditorController::clearInvalidHouseTiles()`):**
    a.  Create `Action* cleanupAction = actionQueue->createAction(CLEANUP_ACTION_ID);`.
    b.  Iterate all tiles in `mapInstance` (from `mapcore`).
    c.  If `tile->isHouseTile()` but `mapInstance->houses.getHouse(tile->getHouseID()) == nullptr`:
        i.  `cleanupAction->addChange(new Change(tile->deepCopy())); // Before`
        ii. `tile->setHouseID(0); tile->setPZ(false); // Or other logic from original Editor::clearInvalidHouseTiles`
        iii. `cleanupAction->addChange(new Change(tile->deepCopy())); // After`
    d.  If `cleanupAction` contains changes, `actionQueue->addAction(cleanupAction);`.
    e.  Adapt similar patterns for other cleanup tools like `removeDuplicateItems` and `validateGrounds`, porting their core logic from `map.cpp`.
4.  **Menu Integration**: Ensure `QAction`s in `MainWindow` for these tools are connected to call the new `EditorController` methods.
5.  All operations must be undoable and use `mapcore` data structures.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/LOGIC-09.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
