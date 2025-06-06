import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "CORE-05",
    "section": "Core Migration Tasks",
    "title": "Port Action, Selection, Copy/Paste",
    "original_input_files": "action.h/cpp, selection.h/cpp, copybuffer.h/cpp, threads.h",
    "analyzed_input_files": [
        "wxwidgets/action.h",
        "wxwidgets/action.cpp",
        "wxwidgets/selection.h",
        "wxwidgets/selection.cpp",
        "wxwidgets/copybuffer.h",
        "wxwidgets/copybuffer.cpp",
        "wxwidgets/threads.h"
    ],
    "dependencies": [
        "CORE-04"
    ],
    "current_functionality_summary": """\
These files implement essential editor functionalities:
- `action.h/cpp`: Provides the Undo/Redo system (`ActionQueue`, `BatchAction`, `Action`, `Change`) by storing deep copies of modified tiles or entity states. `DirtyList` tracks changes for optimized updates.
- `selection.h/cpp`: Manages user selections (`Selection`) on the map. Selection changes are themselves undoable actions. `SelectionThread` uses `wxThread` for backgrounded area selection.
- `copybuffer.h/cpp`: Handles copy, cut, and paste operations for map segments, using an internal `BaseMap` to store copied data and leveraging the action system.
- `threads.h`: Contains simple C++ wrappers around `wxThread`.
These systems are currently coupled with an `Editor` class and wxWidgets.\
""",
    "qt6_migration_steps": """\
1. Integrate `Action`, `BatchAction`, `ActionQueue`, `Change`, `DirtyList`, `Selection`, and `CopyBuffer` classes into the `mapcore` library, maintaining them as separate `.h/.cpp` files.
2. Refactor `SelectionThread`: Replace `wxThread` with `std::thread`. This includes updating thread creation, management, and the mechanism for communicating results (e.g., selected tiles or the completed selection `Action`) back to the main thread (consider `std::promise`/`std::future` or a thread-safe queue).
3. Evaluate `threads.h`: Remove the `wxThread` wrappers. If a similar abstraction over `std::thread` is desired for `mapcore`, reimplement it; otherwise, remove `threads.h` and use `std::thread` directly in `SelectionThread`.
4. Decouple from `Editor` class and UI:
   - Modify constructors and methods in `ActionQueue`, `BatchAction`, `Action`, `Selection`, etc., to operate on `Map&` (from `CORE-01`) and other necessary `mapcore` components (like a `Selection*` object if passed) instead of an `Editor&` reference.
   - Remove all direct calls to UI elements or global UI managers (e.g., `g_gui` for status updates). If `mapcore` needs to signal such state changes, this should be done via callbacks or an event system for the UI layer to subscribe to.
5. Ensure consistent use of standard C++ containers (`std::vector`, `std::deque`, `std::set`) and types (`std::string`).
6. Verify that the deep copying mechanisms (`Tile::deepCopy`, `Item::deepCopy`, etc.) used by `Action` and `CopyBuffer` correctly utilize the methods from the `mapcore` data models (from `CORE-01`).
7. Confirm that these systems operate correctly within the `mapcore` library, are fully decoupled from wxWidgets and specific UI logic, and compile cleanly.\
""",
    "definition_of_done": """\
The Undo/Redo (`ActionQueue`), `Selection`, and `CopyBuffer` systems are successfully ported to the `mapcore` library and are UI-independent.
Key requirements:
- All specified classes are part of `mapcore`, structured as modular `.h/.cpp` files.
- `SelectionThread` uses `std::thread` instead of `wxThread`, with appropriate mechanisms for thread communication.
- `wxThread` wrappers from `threads.h` are removed or adapted for `std::thread`.
- Direct dependencies on any `Editor` class (if it implies UI coupling) and UI singletons (like `g_gui`) are removed. Operations utilize `Map&` and other `mapcore` data structures.
- All wxWidgets types are replaced with standard C++ equivalents.
- The systems correctly use data models from `CORE-01` and their `deepCopy` mechanisms.
- The `mapcore` library, including these ported systems, compiles successfully.\
""",
    "boilerplate_coder_ai_prompt": """\
Your task is to port the Action/Undo-Redo system (`ActionQueue`, `BatchAction`, `Action`, `Change`, `DirtyList`), Selection system (`Selection`, `SelectionThread`), and Copy/Paste system (`CopyBuffer`) into the `mapcore` static library. These classes should be maintained as separate `.h/.cpp` files within `mapcore`. This task depends on `CORE-04`.
1. For `SelectionThread`: Replace `wxThread` with `std::thread`. Adapt thread management (creation, joining) and how results are passed back to the caller (e.g., use `std::promise` and `std::future`).
2. Remove or refactor `threads.h`: If these wrappers are thin, consider using `std::thread` directly in `SelectionThread`. If a generic thread wrapper is needed for `mapcore`, ensure it's based on `std::thread`.
3. Decouple from UI: Remove dependencies on any `Editor` class that might be UI-coupled. These systems should operate on `Map&` (from `CORE-01`) and other relevant `mapcore` data. Remove any calls to `g_gui` or UI update functions (e.g., status bar text).
4. Replace any remaining wxWidgets types with standard C++ types (`std::string`, `std::vector`, etc.).
5. Verify that deep copying logic (e.g., `Tile::deepCopy()`) from `CORE-01` data models is correctly used.
6. Ensure all ported classes compile cleanly within `mapcore` and are independent of wxWidgets.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/CORE-05.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
