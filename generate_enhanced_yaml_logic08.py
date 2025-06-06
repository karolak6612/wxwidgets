import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "LOGIC-08",
    "section": "Brush Functionality",
    "title": "Integrate Drawing Modes & Advanced Brushes",
    "original_input_files": "editor.cpp, gui.cpp, all `*_brush.cpp` files",
    "analyzed_input_files": [
        "wxwidgets/editor.cpp",
        "wxwidgets/gui.cpp",
        "wxwidgets/brush.cpp", # Contains base logic and some brush types
        "wxwidgets/ground_brush.cpp",
        "wxwidgets/wall_brush.cpp",
        "wxwidgets/doodad_brush.cpp",
        "wxwidgets/creature_brush.cpp",
        "wxwidgets/house_brush.cpp",
        "wxwidgets/spawn_brush.cpp",
        "wxwidgets/raw_brush.cpp",
        "wxwidgets/carpet_brush.cpp",
        "wxwidgets/table_brush.cpp",
        "wxwidgets/waypoint_brush.cpp",
    ],
    "dependencies": [
        "LOGIC-01",      # For EditorController and basic drawing action structure
        "UI-PALETTE-02" # For UI elements that set brush size/shape
    ],
    "current_functionality_summary": """\
Drawing modes (point, drag-smear, shapes like square/circle) are handled by `MapCanvas` in conjunction with settings from `g_gui` (current brush, size, shape). `MapCanvas` determines affected tiles and calls `Editor::drawInternal`.
`Editor::drawInternal` then invokes the active brush's `draw`/`undraw` method.
Advanced brushes (`GroundBrush`, `WallBrush`, `TableBrush`, `CarpetBrush`) contain specific logic (e.g., `doBorders`, `doWalls`) to automatically connect pieces or adjust neighboring tiles. This logic is typically called after the primary draw operation on a tile or set of tiles.\
""",
    "qt6_migration_steps": """\
1. Ensure `BrushManager` (from `UI-03` or `REFACTOR-01`, replacing `g_gui`'s brush state role) accurately stores and provides the active brush, brush size (e.g., 1 for 1x1, 3 for 3x3), and brush shape (e.g., `POINT`, `SQUARE`, `CIRCLE`). UI palettes (`UI-PALETTE-02`) will interact with `BrushManager`.
2. Refine `EditorController::processBrushAction` (from `LOGIC-01`) or related methods:
   a. It must accept `BrushSettings` (shape, size, specific brush parameters like item ID for RAWBrush) from `BrushManager`.
   b. Implement a helper within `EditorController` or `MapView`: `PositionVector getTilesAffectedByBrush(const Position& centerPos, BrushShape shape, int size)` which calculates all map positions to be affected based on the brush's current shape and size settings.
3. Implement Drag-and-Draw (Smearing):
   a. In `MapView::mouseMoveEvent`: If drawing mode is active, the left mouse button is held down, the active brush's `canSmear()` method (from `mapcore`) returns true, and the mouse cursor has moved to a new tile:
      i. Call `editorController->processBrushAction(newMouseMapPos, isCtrlPressed, currentBrushSettings)` for the single new tile (implicitly `POINT` shape for smearing).
      ii. The `ActionQueue` should be configured (as per `CORE-05` and `AppSettings`) to merge these rapid, small actions into larger `BatchAction`s if `AppSettings::getGroupActions()` is true, for a smoother undo experience.
4. Implement Shape Drawing:
   a. When `EditorController::processBrushAction` is called (typically from `MapView::mousePressEvent` for placing shapes):
      i.  `PositionVector affectedTiles = getTilesAffectedByBrush(targetPos, brushSettings.shape, brushSettings.size);`
      ii. Create a `BatchAction` (or use the current one if smearing and grouping actions).
      iii. Create an `Action` for the primary drawing of all `affectedTiles`.
      iv. For each `pos` in `affectedTiles`:
          Get/create `Tile* tile = map->getOrCreateTile(pos);`.
          Add a `Change` for the tile's 'before' state to the `Action`.
          Call `activeBrush->draw(map, tile, &brushSettings.params)` or `undraw()`.
          Add a `Change` for the tile's 'after' state to the `Action`.
      v.  Add this `Action` to the `BatchAction`.
5. Integrate Advanced Brush Logic (Auto-Bordering/Connections):
   a. After the primary drawing `Action` (from step 4.iv) is added to the `BatchAction` and conceptually committed (its changes are now part of the 'current state' for the next action in the batch):
      i.  If the `activeBrush` requires secondary updates (e.g., `GroundBrush`, `WallBrush`, `TableBrush`, `CarpetBrush`):
          Create a new `Action* secondaryAction = actionQueue->createAction(batch);`.
          Collect a list of all `affectedTiles` from the primary action AND their relevant neighbors into a `tilesToUpdate 주변Tiles` list (ensure uniqueness).
          For each `tileToUpdate` in `tilesToUpdate 주변Tiles`:
              Add `Change` for `tileToUpdate`'s 'before' state to `secondaryAction`.
              If `activeBrush->isGround()` or `activeBrush->isEraser()`: call `tileToUpdate->borderize(map);` (using `mapcore`'s `Tile` method).
              If `activeBrush->isWall()`: call `tileToUpdate->wallize(map);`.
              If `activeBrush->isTable()`: call `tileToUpdate->tableize(map);`.
              If `activeBrush->isCarpet()`: call `tileToUpdate->carpetize(map);`.
              Add `Change` for `tileToUpdate`'s 'after' state to `secondaryAction`.
          Add `secondaryAction` to the `BatchAction`.
   b. Finally, add the `BatchAction` to the `ActionQueue`.
6. Verify that all special brush behaviors (e.g., `DoodadBrush` placing composites, `WallBrush` connecting different wall types, door placement from `DoorBrush`) are correctly handled through their `draw` methods, which are invoked by `EditorController`.\
""",
    "definition_of_done": """\
All brush drawing modes (drag-and-draw for smearable brushes, point-click, and shape drawing like square/circle) are functional in the `MapView`. Advanced logic for special brushes (e.g., `GroundBrush` auto-bordering, `WallBrush` connections, `TableBrush`/`CarpetBrush` pattern matching) is correctly applied when these brushes are used.
Key requirements:
- Dragging the mouse with an active brush that supports smearing continuously applies the brush to newly entered tiles.
- Selecting brush shapes (square, circle) and sizes via `BrushManager`/UI correctly defines the area of effect for brush application.
- `GroundBrush` correctly applies auto-borders to itself and its neighbors.
- `WallBrush` correctly connects wall segments, forming appropriate corners and junctions.
- `TableBrush` and `CarpetBrush` correctly form continuous surfaces by selecting appropriate item variants based on neighbors.
- `DoodadBrush` correctly places its defined single items or composite structures.
- All drawing operations, including secondary effects like bordering, are correctly encapsulated in `Action`s / `BatchAction`s and are undoable/redoable via the `ActionQueue`.
- The core logic is managed by `EditorController`, using settings from `BrushManager` and invoking brush implementations from `mapcore`.\
""",
    "boilerplate_coder_ai_prompt": """\
Implement advanced brush drawing modes and integrate special brush logic (auto-bordering, connections) within the `EditorController`. This builds upon `LOGIC-01` (basic drawing action) and `UI-PALETTE-02` (for UI controls that set brush size/shape via `BrushManager`).
1.  **Refine `EditorController::processBrushAction` (or `handleMapClick`):**
    a.  It must accept `BrushSettings` (containing shape, size, and any brush-specific parameters like itemID for RAWBrush) from `BrushManager`.
    b.  Implement `PositionVector getTilesInBrushArea(const Position& centerMapPos, const BrushSettings& settings)`:
        - If `settings.shape == POINT` (or for smearing), returns just `centerMapPos`.
        - If `settings.shape == SQUARE` or `settings.shape == CIRCLE`, calculates all tiles in an NxN area or a circle of `settings.size` radius around `centerMapPos`.
2.  **Main Drawing Action:**
    a.  When a drawing operation is initiated (e.g., mouse click):
        `PositionVector primaryTiles = getTilesInBrushArea(clickedMapPos, currentBrushSettings);`
        `BatchAction* batch = actionQueue->createBatch(DRAW_ACTION_ID);`
        `Action* mainDrawAction = actionQueue->createAction(batch);`
    b.  For each `pos` in `primaryTiles`:
        - Get/create `Tile* tile = map->getOrCreateTile(pos);`.
        - `mainDrawAction->addChange(new Change(tile->deepCopy())); // Before state`
        - If undraw operation (e.g. Ctrl-click): `activeBrush->undraw(map, tile);`
        - Else (draw): `activeBrush->draw(map, tile, &currentBrushSettings.params);` // Pass specific params like item ID
        - `mainDrawAction->addChange(new Change(tile->deepCopy())); // After state`
    c.  `batch->addAction(mainDrawAction);` // This should also commit the action within the batch context
3.  **Secondary Effects Action (Bordering, Connections):**
    a.  If `activeBrush` requires updates to neighbors (e.g., Ground, Wall, Table, Carpet brushes):
        - `Action* secondaryAction = actionQueue->createAction(batch);`
        - Create a `PositionSet tilesToUpdateEffects;` Add all `primaryTiles` to it. Also add all direct neighbors of `primaryTiles` to this set.
        - For each `posToEffect` in `tilesToUpdateEffects`:
            - `Tile* tile = map->getTile(posToEffect); if (!tile) continue;`
            - `secondaryAction->addChange(new Change(tile->deepCopy())); // Before effect`
            - If `activeBrush->isGround()` or `activeBrush->isEraser()`: `tile->borderize(map);`
            - Else if `activeBrush->isWall()`: `tile->wallize(map);`
            - Else if `activeBrush->isTable()`: `tile->tableize(map);`
            - Else if `activeBrush->isCarpet()`: `tile->carpetize(map);`
            - `secondaryAction->addChange(new Change(tile->deepCopy())); // After effect`
        - `batch->addAction(secondaryAction);`
4.  `actionQueue->addBatch(batch);` // Add the complete batch (primary draw + secondary effects)
5.  **Smearing (Drag-to-Draw) in `MapView::mouseMoveEvent`:**
    - If LMB is down, in drawing mode, `activeBrush->canSmear()`, and mouse is on a *new* tile:
        - Call `editorController->processBrushAction(newMouseMapPos, isCtrlClick, currentBrushSettings)` (but ensure `getTilesInBrushArea` for smearing always returns a single point, effectively).
        - `ActionQueue` should group these if `AppSettings::getGroupActions()` is true.
6.  Ensure special brush behaviors (e.g., `DoodadBrush` composite placement) are handled correctly by their `draw` methods.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/LOGIC-08.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
