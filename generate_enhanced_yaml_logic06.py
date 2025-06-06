import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "LOGIC-06",
    "section": "Editor Behavior",
    "title": "Integrate House & Waypoint Brushes",
    "original_input_files": "house_brush.h/cpp, house_exit_brush.h/cpp, waypoint_brush.h/cpp",
    "analyzed_input_files": [
        "wxwidgets/house_brush.h",
        "wxwidgets/house_brush.cpp",
        "wxwidgets/house_exit_brush.h",
        "wxwidgets/house_exit_brush.cpp",
        "wxwidgets/waypoint_brush.h",
        "wxwidgets/waypoint_brush.cpp"
    ],
    "dependencies": [
        "UI-PALETTE-03", # For selecting the active house/waypoint from UI
    ],
    "current_functionality_summary": """\
- `HouseBrush` (`house_brush.h/cpp`): Assigns/unassigns tiles to a specified house and sets/clears PZ flags. The actual update to the `House` object's tile list is expected to be handled by the `Action` system.
- `HouseExitBrush` (`house_exit_brush.h/cpp`): Marks a tile as an exit for a specified house. Its `draw/undraw` methods are not directly used; instead, a `CHANGE_MOVE_HOUSE_EXIT` action is created by higher-level editor logic.
- `WaypointBrush` (`waypoint_brush.h/cpp`): Places or moves a named waypoint. Similar to `HouseExitBrush`, its `draw/undraw` are not directly used; a `CHANGE_MOVE_WAYPOINT` action is created.
All these operations are designed to be undoable.\
""",
    "qt6_migration_steps": """\
1. Extend the `EditorController::handleMapClick` method (or a similar dispatcher introduced in `LOGIC-01`) to include specific logic when the active brush is `HouseBrush`, `HouseExitBrush`, or `WaypointBrush`.
2. **For `HouseBrush`:**
   a. The `BrushManager` (or equivalent UI state provider) must make the currently selected `House*` (from the House Palette, task `UI-PALETTE-03`) available to the `EditorController`.
   b. When `handleMapClick` is called for `HouseBrush` on a `targetPos`:
      i.  Retrieve the `Tile* tileToModify = map->getOrCreateTile(targetPos)`.
      ii. Create an `Action* action = actionQueue->createAction(ACTION_DRAW_HOUSE);` (or a generic draw action ID).
      iii. Add `Change` for `tileToModify`'s 'before' state: `action->addChange(new Change(tileToModify->deepCopy()));`.
      iv.  `House* oldHouse = map->houses.getHouse(tileToModify->getHouseID());`
      v.  If performing an "undraw" operation (e.g., Ctrl+Click):
          Call `activeBrush->asHouse()->undraw(map, tileToModify);` (this clears `house_id` and PZ flag on `tileToModify`).
          If `oldHouse`, call `oldHouse->removeTile(tileToModify);` (this uses the `mapcore` `House` method from `LOGIC-05`).
      vi. Else (draw operation):
          `activeBrush->asHouse()->setHouse(selectedHouse);`
          `activeBrush->asHouse()->draw(map, tileToModify, nullptr);` (this sets `house_id` and PZ flag on `tileToModify`).
          If `oldHouse` and `oldHouse != selectedHouse`, call `oldHouse->removeTile(tileToModify);`.
          Call `selectedHouse->addTile(tileToModify);`.
      vii.Add `Change` for `tileToModify`'s 'after' state: `action->addChange(new Change(tileToModify->deepCopy()));`.
      viii.Push the `Action` to `ActionQueue`: `actionQueue->addAction(action);`.
3. **For `HouseExitBrush`:**
   a. `BrushManager` provides the selected `House*`.
   b. When `handleMapClick` is called for `HouseExitBrush` on `targetPos`:
      i.  `if (!activeBrush->asHouseExit()->canDraw(map, targetPos)) return;`
      ii. Create an `Action* action = actionQueue->createAction(ACTION_SET_HOUSE_EXIT);`.
      iii. Create a `Change` object of type `CHANGE_MOVE_HOUSE_EXIT`. This `Change` must store `selectedHouse->getID()` and `targetPos`.
      iv. `action->addChange(changeObject);`
      v.  `actionQueue->addAction(action);`. (The `Action::commit` for `CHANGE_MOVE_HOUSE_EXIT` will call `selectedHouse->setExit(targetPos)`).
4. **For `WaypointBrush`:**
   a. `BrushManager` provides the selected `Waypoint*` name (or a new name if creating).
   b. When `handleMapClick` is called for `WaypointBrush` on `targetPos`:
      i.  `if (!activeBrush->asWaypoint()->canDraw(map, targetPos)) return;`
      ii. Create an `Action* action = actionQueue->createAction(ACTION_SET_WAYPOINT);`.
      iii. Create a `Change` object of type `CHANGE_MOVE_WAYPOINT`. Store waypoint name and `targetPos`.
      iv. `action->addChange(changeObject);`
      v.  `actionQueue->addAction(action);`. (The `Action::commit` for `CHANGE_MOVE_WAYPOINT` will update `map->waypoints` and `TileLocation` counters).
5. In `MapView::mousePressEvent` (from `UI-EVENT-01`): If active brush is one of these types, ensure necessary context (selected `House*`, waypoint name) is fetched from the relevant palette (via `BrushManager`) and passed to `EditorController`.\
""",
    "definition_of_done": """\
Clicking on the `MapView` with an active `HouseBrush`, `HouseExitBrush`, or `WaypointBrush` correctly modifies the map data through the `EditorController` and creates an undoable `Action`.
Key requirements:
- When `HouseBrush` is active, a click assigns/unassigns the target tile to/from the currently selected house (from the House Palette). This updates `Tile::house_id`, relevant tile flags (like PZ), and the `House` object's list of tiles. The operation is undoable.
- When `HouseExitBrush` is active, a click on a valid tile sets the exit of the currently selected house to that tile's position. This is recorded as an undoable `Action` (e.g., of type `CHANGE_MOVE_HOUSE_EXIT`).
- When `WaypointBrush` is active, a click on a tile creates a new waypoint or moves the currently selected waypoint (from the Waypoint Palette) to that tile's position. This is recorded as an undoable `Action` (e.g., of type `CHANGE_MOVE_WAYPOINT`).
- All interactions are managed by `EditorController` and correctly utilize `mapcore` components (`ActionQueue`, `House`, `Waypoint`, `Tile` methods).
- The `MapView` visually reflects these changes after the action is committed.\
""",
    "boilerplate_coder_ai_prompt": """\
Extend `EditorController::handleMapClick` (or a similar dispatcher) to specifically handle `HouseBrush`, `HouseExitBrush`, and `WaypointBrush`. These brushes depend on selections made in their respective palettes (task `UI-PALETTE-03`).
1.  **If `activeBrush->isHouse()`:**
    a.  Get `House* selectedHouse = brushManager->getCurrentHouseFromPalette();`. If null, return.
    b.  Get `Tile* tile = map->getOrCreateTile(mapPos);`.
    c.  Create `Action* action = actionQueue->createAction(DRAW_HOUSE_ACTION);`.
    d.  `action->addChange(new Change(tile->deepCopy()));` // 'Before' state.
    e.  `House* oldHouse = map->houses.getHouse(tile->getHouseID());`
    f.  If undrawing (e.g., Ctrl+Click):
        `activeBrush->asHouse()->undraw(map, tile);`
        `if (oldHouse) oldHouse->removeTile(tile);`
    g.  Else (drawing):
        `activeBrush->asHouse()->setHouse(selectedHouse);`
        `activeBrush->asHouse()->draw(map, tile, nullptr);`
        `if (oldHouse && oldHouse != selectedHouse) oldHouse->removeTile(tile);`
        `selectedHouse->addTile(tile);`
    h.  `action->addChange(new Change(tile->deepCopy()));` // 'After' state.
    i.  `actionQueue->addAction(action);`
2.  **Else if `activeBrush->isHouseExit()`:**
    a.  `House* selectedHouse = brushManager->getCurrentHouseFromPalette();`.
    b.  `HouseExitBrush* houseExitBrush = activeBrush->asHouseExit();`
    c.  `houseExitBrush->setHouse(selectedHouse); // Set context for the brush if it stores house ID/ptr`
    d.  `if (!selectedHouse || !houseExitBrush->canDraw(map, mapPos)) return;`
    e.  `Action* action = actionQueue->createAction(SET_HOUSE_EXIT_ACTION);`
    f.  `action->addChange(Change::Create(selectedHouse, mapPos)); // Creates a CHANGE_MOVE_HOUSE_EXIT`
    g.  `actionQueue->addAction(action);`
3.  **Else if `activeBrush->isWaypoint()`:**
    a.  `Waypoint* selectedWaypoint = brushManager->getCurrentWaypointFromPalette(); // Or get new waypoint name`
    b.  `WaypointBrush* waypointBrush = activeBrush->asWaypoint();`
    c.  `waypointBrush->setWaypoint(selectedWaypoint); // Set context for the brush`
    d.  `if (!selectedWaypoint || !waypointBrush->canDraw(map, mapPos)) return;`
    e.  `Action* action = actionQueue->createAction(SET_WAYPOINT_ACTION);`
    f.  `action->addChange(Change::Create(selectedWaypoint, mapPos)); // Creates a CHANGE_MOVE_WAYPOINT`
    g.  `actionQueue->addAction(action);`
4.  Ensure `MapView::mousePressEvent` provides necessary context (like selected house/waypoint from palettes) to `EditorController`.
5.  Verify that the `commit()` methods for `CHANGE_MOVE_HOUSE_EXIT` and `CHANGE_MOVE_WAYPOINT` correctly update the `House` and `map->waypoints` data in `mapcore`.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/LOGIC-06.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
