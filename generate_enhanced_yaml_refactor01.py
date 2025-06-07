import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "REFACTOR-01",
    "section": "Post-Migration Refactoring & Optimization",
    "title": "Decouple Systems from Global Managers",
    "original_input_files": "gui.cpp, all files that include `gui.h`",
    "analyzed_input_files": [
        "wxwidgets/gui.h", # For understanding original g_gui responsibilities
        "wxwidgets/gui.cpp", # For understanding original g_gui responsibilities
        "Conceptual: All ported Qt files that would interact with the GuiManager singleton (from UI-03)"
    ],
    "dependencies": [
        "FINAL-04" # Ensures most UI components and theme management are in place
    ],
    "current_functionality_summary": """\
The application's UI state and access to shared UI-related services were planned to be centralized in a `GuiManager` singleton (as per task `UI-03`), which itself was intended as a replacement for the original wxWidgets-era `g_gui` global object. This `GuiManager` would typically handle state like the active brush, brush settings (size, shape), current editor mode (draw, select), etc. This task aims to eliminate this global singleton pattern in favor of dependency injection for better modularity and testability.\
""",
    "qt6_migration_steps": """\
1. Thoroughly identify all responsibilities that were conceptually assigned to the `GuiManager` singleton (from WBS task `UI-03`). These typically include managing:
   - The active brush instance.
   - Current brush settings (size, shape, specific data like item ID for RAWBrush or CreatureType for CreatureBrush).
   - Current editor mode (e.g., `DRAWING_MODE`, `SELECTION_MODE`).
   - Potentially other shared UI states or access to common UI utility functions.
2. Design and implement smaller, focused service classes to encapsulate these responsibilities. Examples:
   - `BrushManager`: Manages the active brush object, its current type, size, shape, and any associated data specific to that brush. It should provide methods to get/set the active brush and its properties, and emit signals when these change.
   - `EditorStateManager` (or `ViewStateManager`): Manages editor-wide states such as the current drawing/selection mode, the active floor being viewed, potentially current zoom level if not managed solely by `MapView`. It should also emit signals on state changes.
   - Evaluate if any other distinct services are needed based on the full scope originally planned for `GuiManager`.
3. Instantiate these new manager/service classes within the `MainWindow` class (created in `UI-01`). `MainWindow` will own these service instances (e.g., as member variables: `m_brushManager = new BrushManager(this);`).
4. Implement Dependency Injection:
   - Modify the constructors of key UI components and controller classes (`MapView`, `EditorController`, palette widgets, toolbars) to accept pointers or references to the manager/service instances they require. For example, `EditorController`'s constructor might become `EditorController(Map* map, ActionQueue* queue, Selection* sel, BrushManager* brushManager, EditorStateManager* stateManager, ...)`.
   - `MainWindow` will be responsible for passing its owned service instances when it creates these child widgets or controller objects.
5. Perform a codebase-wide refactoring:
   - Remove the `GuiManager` singleton class definition and all its instances/usages.
   - Update all classes that previously would have called `GuiManager::instance()->someMethod()` to now use the methods of the service instances that were injected into them (e.g., `m_brushManager->getActiveBrush()`).
6. Utilize Qt's signal/slot mechanism for communication between these decoupled components. For instance, if a palette widget changes the active brush via `BrushManager::setActiveBrush()`, `BrushManager` should emit a signal (e.g., `activeBrushChanged(Brush* newBrush)`). `MapView` or other components can connect to this signal to update their state or display (e.g., changing the mouse cursor).
7. Ensure that `EditorController` is properly provided with all necessary manager/service instances (BrushManager, EditorStateManager) and `mapcore` components (Map, ActionQueue, Selection) through its constructor or dedicated setter methods to perform its operations.\
""",
    "definition_of_done": """\
The global `GuiManager` singleton (conceptually introduced in UI-03 to replace g_gui) is eliminated from the application architecture, and its responsibilities are handled by focused service classes managed by MainWindow and provided to other components via dependency injection.
Key requirements:
- Responsibilities like active brush management and editor state are now handled by new, non-global service classes (e.g., `BrushManager`, `EditorStateManager`).
- These service classes are instantiated and owned by `MainWindow`.
- Other components (`MapView`, `EditorController`, palettes, toolbars, etc.) receive instances of these services through their constructors or dedicated setters (dependency injection).
- All direct or indirect reliance on a global `GuiManager` (or `g_gui`) for these services is removed from the codebase.
- The application remains fully functional with this refactored architecture, demonstrating improved modularity and reduced global state.
- Communication between these decoupled components is achieved using Qt's signal/slot mechanism where appropriate.\
""",
    "boilerplate_coder_ai_prompt": """\
Refactor the application to eliminate the global `GuiManager` singleton (which was planned in `UI-03` as a replacement for the old `g_gui`). Instead, use dependency injection for managing UI-related services. (Depends on `FINAL-04` and many UI tasks being complete).
1.  **Create New Service Classes (if not already partially done in `UI-03`):**
    -   `BrushManager(QObject* parent = nullptr)`: Manages `Brush* m_activeBrush`, `BrushShape m_currentShape`, `int m_currentSize`, and any data for specialized brushes (e.g., current `ItemType*` for RAW, `CreatureType*` for Creature).
        -   Provide public methods: `setActiveBrush(Brush*)`, `setBrushShape(BrushShape)`, `setBrushSize(int)`, `getActiveBrush()`, etc.
        -   Emit signals on change: `void activeBrushChanged(Brush* newBrush);`, `void brushSettingsChanged();`.
    -   `EditorStateManager(QObject* parent = nullptr)`: Manages `EditorMode m_currentMode` (e.g., enum `DRAW_MODE`, `SELECTION_MODE`, `FILL_MODE`), `int m_currentFloor`.
        -   Provide methods to set/get these states.
        -   Emit signals on change: `void editorModeChanged(EditorMode newMode);`, `void currentFloorChanged(int newFloor);`.
2.  **In `MainWindow.h/.cpp` (from `UI-01`):**
    -   Add member pointers: `BrushManager* m_brushManager;`, `EditorStateManager* m_editorStateManager;`.
    -   In `MainWindow` constructor: `m_brushManager = new BrushManager(this);`, `m_editorStateManager = new EditorStateManager(this);`.
3.  **Update Constructors/Setters for Dependency Injection:**
    -   `EditorController` constructor should now take `BrushManager*` and `EditorStateManager*` (along with `Map*`, `ActionQueue*`, `Selection*`).
    -   `MapView` constructor should take `BrushManager*`, `EditorStateManager*`, etc., as needed for rendering cursors, visual feedback for modes, etc.
    -   Palette widgets (from `UI-PALETTE-*` tasks) should take `BrushManager*` to call its setters when the user selects a new brush or changes settings.
    -   Toolbars with mode-switching buttons should interact with `EditorStateManager*`.
4.  **Replace Global Access:**
    -   Search for all usages of `GuiManager::instance()` (or `g_gui` if any remnants exist) and replace them with calls to methods on the injected manager instances.
    -   For example, if `MapView` needed the active brush for cursor display, it would now use `m_injectedBrushManager->getActiveBrush()`.
5.  **Connect Signals/Slots:**
    -   Example: `MapView` might connect to `m_brushManager->activeBrushChanged()` to update its cursor. Palettes connect their UI controls' signals to slots that call `m_brushManager->setActiveBrush()`, etc.
6.  Remove the `GuiManager.h/.cpp` files and any global instance.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/REFACTOR-01.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
