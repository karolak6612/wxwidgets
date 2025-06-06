import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

# Using f-string for the C++ code block, ensuring {{ and }} are used for literal braces
cpp_example_in_boilerplate = """\
// Item* relevantItem = determine_relevant_item_for_rotation(clickedTile, selection);
// bool canRotate = relevantItem && relevantItem->isRotatable(); // Assuming Item has isRotatable()
// QAction* rotateAction = contextMenu.addAction("Rotate Item");
// rotateAction->setEnabled(canRotate);
// if (canRotate) {{
//   connect(rotateAction, &QAction::triggered, this, [=]() {{
//     editorController->rotateItem(relevantItem); // This method in EditorController would create an Action
//   }});
// }}\
"""

yaml_content = {
    "id": "LOGIC-07",
    "section": "Editor Behavior",
    "title": "Port Map & Selection Context Menus",
    "original_input_files": "map_display.h/cpp (`MapPopupMenu`), main_menubar.cpp",
    "analyzed_input_files": [
        "wxwidgets/map_display.h",
        "wxwidgets/map_display.cpp",
        "wxwidgets/main_menubar.h",
        "wxwidgets/main_menubar.cpp"
    ],
    "dependencies": [
        "LOGIC-01", # For EditorController and basic drawing/action logic
        "LOGIC-02"  # For selection state
    ],
    "current_functionality_summary": """\
In the original application, `MapCanvas::OnMouseRightRelease` (from `map_display.cpp`) triggers a `MapPopupMenu`. The `MapPopupMenu::Update()` method dynamically builds this menu with context-sensitive actions based on what's under the cursor or currently selected (e.g., item type, house tile status). These menu actions are mapped to `MenuBar::ActionID` enums (defined in `main_menubar.h`), and their corresponding event handlers are typically found in `main_menubar.cpp` or `MainFrame`, performing operations via `g_gui` and the `Editor`.\
""",
    "qt6_migration_steps": """\
1. In the `MapView` class (developed in `UI-EVENT-01`), override the `contextMenuEvent(QContextMenuEvent *event)` method. This method will be responsible for constructing and displaying the context menu.
2. Inside `MapView::contextMenuEvent`:
   a. Create a `QMenu` instance (e.g., `QMenu contextMenu(this);`).
   b. Convert the event position (`event->pos()`) to map coordinates (`mapClickedPosition`).
   c. Through the `EditorController`, get the `Tile* clickedTile` at `mapClickedPosition` and the current `Selection* currentSelection` from `mapcore`.
   d. Replicate the logic from the original `MapPopupMenu::Update()` method to dynamically populate the `contextMenu`. This involves:
      i.  Checking the properties of `clickedTile` (its ground item, top items, creature, spawn, house status, etc.) and the state of `currentSelection`.
      ii. Conditionally creating `QAction` objects for relevant operations (e.g., "Rotate Item", "Edit Properties", "Select Ground Brush", "Copy", "Paste", "Delete", "Switch Door", "Go To Destination").
      iii.Setting the text for each `QAction` (e.g., `rotateAction = contextMenu.addAction("Rotate Item");`).
      iv. Setting the enabled/disabled state of each `QAction` based on the current context (e.g., `rotateAction->setEnabled(canRotateSelectedItem);`).
      v.  Connecting the `triggered()` signal of each `QAction` to an appropriate slot. These slots will typically reside in `MainWindow` or `EditorController` and will execute the logic corresponding to the original `MenuBar::ActionID` handlers.
      vi. Adding `QAction`s and `addSeparator()` calls to the `contextMenu` to build its structure.
   e. Display the context menu at the global mouse position: `contextMenu.exec(event->globalPos());`.
3. Ensure that the action-handling slots (in `MainWindow` or `EditorController`) perform the intended operations correctly, using `mapcore`'s `ActionQueue` for any operations that modify the map state to ensure they are undoable.\
""",
    "definition_of_done": """\
Right-clicking on the `MapView` displays a context-sensitive `QMenu` with actions relevant to the clicked map element or current selection, and these actions function correctly.
Key requirements:
- The `MapView::contextMenuEvent` method is correctly implemented.
- The `QMenu` is dynamically populated with appropriate `QAction`s based on the context of the right-click (e.g., clicked tile content, selected items), mirroring the logic of the original `MapPopupMenu::Update`.
- Menu actions are correctly enabled or disabled based on the context (e.g., "Rotate" is enabled only if a rotatable item is relevant).
- Selecting an action from the context menu triggers the corresponding application logic (e.g., opening a properties dialog, rotating an item, selecting a brush, performing cut/copy/paste).
- All map-modifying operations initiated from the context menu are correctly added to the `ActionQueue` and are undoable/redoable.\
""",
    "boilerplate_coder_ai_prompt": f"""\
Implement a context menu for the `MapView` widget that appears on right-click.
1.  **In `MapView.h/.cpp` (from `UI-EVENT-01`):**
    -   Override `void contextMenuEvent(QContextMenuEvent *event) override;`.
2.  **Inside `MapView::contextMenuEvent(QContextMenuEvent *event)`:**
    a.  `QMenu contextMenu(this);`
    b.  Convert `event->pos()` to `mapClickedPosition` (map coordinates).
    c.  Obtain `Tile* clickedTile = editorController->getTileAt(mapClickedPosition);` (assuming `EditorController` provides this).
    d.  Obtain `Selection* selection = editorController->getCurrentSelection();`.
    e.  **Dynamically add `QAction`s to `contextMenu` based on `clickedTile` and `selection`. This logic should mirror the conditions found in the original `MapPopupMenu::Update()` from `wxwidgets/map_display.cpp`.**
        -   Example for "Rotate Item":
            ```cpp
{cpp_example_in_boilerplate}
            ```
        -   Add actions for "Properties", "Cut", "Copy", "Paste", "Delete".
        -   Add actions to "Select Brush" (e.g., "Select Ground Brush") if `clickedTile`'s ground has an associated brush.
        -   Add any other actions present in the original `MapPopupMenu` logic, such as "Switch Door", "Go To Destination" for teleports, etc.
    f.  `contextMenu.exec(event->globalPos());`
3.  Ensure that the slots or lambda functions connected to these `QAction`s correctly call methods in `EditorController` or `MainWindow`. These methods should encapsulate the actual logic for the operation and use `mapcore`'s `ActionQueue` if the operation modifies the map.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/LOGIC-07.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
