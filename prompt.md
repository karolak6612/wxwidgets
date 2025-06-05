Please, create a report based of this prompt. Create a single or multiple .md files for taskmaster AI to create tasks based on the prompt below. Please, read all .cpp and .h files from "C:\Users\karol\Documents\REPOMIX\wxwidgets" to make sure every .cpp and .h files are covered that this prompt below missed. Dont be afraid to complete this report if it laks some objectives :


```yaml
system_prompt_context: |
  ---

  ### The Advanced AI Prompt (For AI-to-AI Task Generation)

  **[PROMPT STARTS HERE]**

  **You are a hyper-specialized Senior Systems Architect.** Your function is to create comprehensive **System Modernization Specifications** for AI-driven development workflows. You excel at analyzing complex legacy C++ codebases and generating structured, machine-parsable data that an AI Project Manager can use to create atomic tasks for an AI coding assistant.

  You are an expert in both the legacy tech stack (`C++`, `wxWidgets`, `ImGui`, `SFML`) and the target stack (`Qt 6`). You also possess encyclopedic knowledge of the Tibia private server ecosystem, including Remere's Map Editor (RME), OTClient, and TheForgottenServer (TFS), and their associated file formats (`.otbm`, `.otb`, `.dat`, `.spr`, `.xml`).

  You will be given the full source code for a custom, unmaintained Tibia map editor. ALL SOURCE CODE IS C:\Users\karol\Documents\REPOMIX\wxwidgets

  ---

  ### **Primary Directive**

  Analyze the provided source code and produce a two-part **System Modernization Specification**.

  **CRITICAL: Target Audience for this Document**
  Your output is **NOT** for a human developer. It is a formal specification for a downstream AI, **"Gemini 2.5 Pro (Taskmaster)"**. This Taskmaster AI's sole purpose is to parse your output and generate a sequence of precise, isolated coding tasks for a Coder AI. Therefore, your output must prioritize:

  *   **Structure over prose.** Use tables, lists, and formal notation.
  *   **Explicitness over implication.** State every dependency; leave nothing to inference.
  *   **Granularity over generality.** Break down components to the lowest reasonable level.
  *   **Machine-Parsability.** Use clear headings and consistent formatting.

  ---

  ### **Part A: Technical Audit & System Analysis (The "AS-IS" State)**

  This is the factual foundation. Produce a definitive report on the current state of the source code.

  **1. Dependency Manifest (YAML Format)**
  Scan all build files (`CMakeLists.txt`, `Makefiles`, `.vcproj`, etc.) and headers to generate a dependency list in YAML format.

  ```yaml
  dependencies:
    - name: wxWidgets
      version: # (e.g., 3.1.x, or "Unknown")
      purpose: Core UI Framework, Event Loop, Main Window
      integration_files: ["mainframe.h", "app.cpp"]
    - name: SFML
      version: # (e.g., 2.5.x, or "Unknown")
      purpose: 2D Hardware-Accelerated Rendering for the Map Viewport
      integration_files: ["sfml_canvas.cpp", "render.h"]
    - name: pugixml
      version: # (e.g., 1.11, or "Unknown")
      purpose: Parsing of .xml files (items.xml, etc.)
      integration_files: ["xml_loader.cpp"]
    - name: zlib
      version: # (e.g., 1.2.11, or "Unknown")
      purpose: Decompression of .otbm map data streams
      integration_files: ["io_otbm.cpp"]
    # (Continue for EVERY identified dependency)
  ```

  **2. Architecture & Code Structure Analysis**
  *   **Architecture Pattern:** Identify the primary pattern (e.g., Model-View-Controller, Model-View-Presenter, Big Ball of Mud).
  *   **Language Standard:** C++ Standard (e.g., `C++98`, `C++11`, `C++17`).
  *   **Threading Model:** (e.g., `Single-Threaded`, `Multi-Threaded via std::thread`, `Multi-Threaded via wxThreads`).
  *   **Build System:** (e.g., `CMake 3.x`, `GNU Make`, `Visual Studio 2017 Solution`).

  **3. Core Functionality to File Mapping**
  Create a table that maps critical application features to the files that implement them. This is for the Taskmaster AI to locate relevant code.



  ---

  ### **Part B: The QT6 Migration Specification (The "TO-BE" State)**

  Using the "AS-IS" data from Part A, generate the following actionable plan.

  **1. Target Technology Stack**
  Define the replacement for each legacy component.

  *   **UI Framework:** `Qt 6 Widgets`
  *   **2D Rendering Engine:** `QOpenGLWidget` with direct `OpenGL` calls for performance.
  *   **XML Parsing:** `Qt XML` (QXmlStreamReader/Writer).
  *   **Compression:** Retain `zlib` (or use `Qt's qCompress/qUncompress` if applicable).
  *   **Build System:** `CMake` (version 3.16+).

  **2. Work Breakdown Structure (WBS) & Task Dependency Graph**
  This is the most critical section for the Taskmaster AI. Present the migration as a series of work packages. Each package is a self-contained unit of work.



  *CRITICAL NOTE. THERE IS NO CMAKELIST.TXT. ITS MISSING AND WILL NEED TO BE RECREATED*.

  **[PROMPT ENDS HERE]**"
report:
  title: "System Modernization Specification"
  part_a:
    title: "Part A: Technical Audit & System Analysis (The \"AS-IS\" State)"
    dependency_manifest:
      dependencies:
        - name: "wxWidgets"
          version: "Unknown (feature set implies 3.x series)"
          purpose: "The core UI and application framework. Provides all windowing, dialogs, UI controls, event handling, AUI for dockable panes, GL canvas, and some utility classes like wxString, wxFileName, and wxTimer. It is pervasively used across all UI-related files."
          integration_files:
            - "main.h"
            - "application.h/cpp"
            - "gui.h/cpp"
            - "map_display.h/cpp"
            - "palette_window.h/cpp"
            - "common_windows.h/cpp"
            - "dcbutton.h/cpp"
            - "etc."
        - name: "pugixml"
          version: "Unknown (Bundled in `ext/`)"
          purpose: "Primary XML parser for all game data configuration files (`items.xml`, `creatures.xml`, `materials.xml`, `doodads.xml`, `walls.xml`, `menubar.xml`)."
          integration_files:
            - "ext/pugixml.hpp"
            - "ext/pugixml.cpp"
            - "materials.cpp"
            - "items.cpp"
            - "creatures.cpp"
            - "main_menubar.cpp"
            - "border_editor_window.cpp"
        - name: "Boost C++ Libraries"
          version: "1.38.0 or newer (inferred from Spirit Classic syntax)"
          purpose: "Bundled in `json/` for the `json_spirit` JSON parser. Primarily used for a deprecated RME-Net feature but is still compiled. No active use in core editor functionality."
          integration_files:
            - "json/json_spirit_reader_template.h"
            - "json/json_spirit_value.h"
        - name: "OpenGL Utility Toolkit (GLUT)"
          version: "System-provided"
          purpose: "Used exclusively for rendering bitmap text (via `glutBitmapCharacter`) within the OpenGL context, specifically for drawing map tooltips."
          integration_files:
            - "map_drawer.cpp"
        - name: "zlib"
          version: "System-provided (via wxWidgets)"
          purpose: "Provides gzip decompression functionality. It is not linked directly but is invoked through `wxZlibInputStream` for handling compressed map streams. The Qt migration will require a direct link or use of Qt's built-in compression."
          integration_files:
            - "iomap_otbm.cpp"
        - name: "libarchive"
          version: "System-provided (Conditionally compiled via `#ifdef OTGZ_SUPPORT`)"
          purpose: "Handles the reading of `.otgz` archives, which are gzipped tarballs containing map files."
          integration_files:
            - "iomap_otbm.cpp"
    architecture_and_code_structure_analysis:
      architecture_pattern: "UI-Driven with a Global Singleton (\"God Object\"). The design lacks a structured architectural pattern like MVC or MVVM. Most application logic is initiated directly from UI event handlers (`wxEVT_*`). There is a pervasive global object, `g_gui`, which acts as a service locator, state manager, and event mediator, creating tight coupling between all components of the application. Data models (`Tile`, `Item`) have some knowledge of rendering and brushes, violating separation of concerns."
      language_standard: "C++11. The codebase uses `nullptr`, `auto`, range-based `for` loops, lambdas, and smart pointers (`std::unique_ptr`)."
      threading_model: "Multi-Threaded (Mixed). The system uses `wxThread` for UI-centric background work (e.g., `SelectionThread`) and a `std::thread` implementation for the core networking loop (`NetworkConnection`), indicating a hybrid approach to concurrency."
      build_system: "Missing. No `CMakeLists.txt` or any other build script is present in the provided files. The build system must be reconstructed."
    core_functionality_to_file_mapping:
      - category: "Application & UI Shell"
        features:
          - name: "Application Entry & Main Loop"
            files:
              - "main.h"
              - "application.h/cpp"
              - "gui.h/cpp (g_gui object)"
              - "gui_ids.h"
              - "definitions.h"
              - "welcome_dialog.h/cpp"
          - name: "Main Window Frame, Menu & Toolbar"
            files:
              - "application.h/cpp (MainFrame)"
              - "main_menubar.h/cpp"
              - "main_toolbar.h/cpp"
              - "artprovider.h/cpp"
              - "hotkey_manager.h/cpp"
              - "dcbutton.h/cpp"
              - "pngfiles.h/cpp"
          - name: "Docking, Tabs, & Window Management"
            files:
              - "editor_tabs.h/cpp"
              - "map_tab.h/cpp"
              - "map_window.h/cpp"
              - "gui.cpp (uses wxAuiManager)"
      - category: "Core Data Models"
        features:
          - name: "Core Map Structure"
            files:
              - "map.h/cpp"
              - "tile.h/cpp"
              - "position.h"
              - "basemap.h/cpp"
              - "map_region.h/cpp"
              - "map_allocator.h"
          - name: "Item & Creature Models"
            files:
              - "item.h/cpp"
              - "items.h/cpp"
              - "creature.h/cpp"
              - "creatures.h/cpp"
              - "complexitem.h/cpp"
              - "outfit.h"
              - "item_attributes.h/cpp"
          - name: "Spawn, House, Town, Waypoint Models"
            files:
              - "spawn.h/cpp"
              - "house.h/cpp"
              - "town.h/cpp"
              - "waypoints.h/cpp"
          - name: "Undo/Redo & Action System"
            files:
              - "action.h/cpp"
      - category: "File I/O & Parsers"
        features:
          - name: "Generic I/O Handlers"
            files:
              - "filehandle.h/cpp"
              - "iomap.h/cpp"
          - name: "OTBM Format R/W"
            files:
              - "iomap_otbm.h/cpp"
          - name: "OTMM Format R/W"
            files:
              - "iomap_otmm.h/cpp"
          - name: "Game Data Parsers (.xml)"
            files:
              - "materials.h/cpp"
              - "tileset.h/cpp"
              - "extension.h/cpp"
          - name: "JSON & OTML Parsers"
            files:
              - "json.h"
              - "all files in json/"
              - "otml.h"
          - name: "Client Version Management"
            files:
              - "client_version.h/cpp"
      - category: "Rendering"
        features:
          - name: "OpenGL Canvas"
            files:
              - "map_display.h/cpp (MapCanvas)"
          - name: "Map & Entity Drawing Logic"
            files:
              - "map_drawer.h/cpp"
          - name: "Asset & Sprite Management (.dat/.spr)"
            files:
              - "graphics.h/cpp (GraphicManager)"
              - "sprites.h"
          - name: "Visual Effects (Lighting)"
            files:
              - "light_drawer.h/cpp"
      - category: "Editor Logic & Tooling"
        features:
          - name: "Editor Session Manager"
            files:
              - "editor.h/cpp"
          - name: "Selection System"
            files:
              - "selection.h/cpp"
          - name: "Copy/Paste Buffer"
            files:
              - "copybuffer.h/cpp"
          - name: "Brush System (Core & Implementations)"
            files:
              - "brush.h/cpp"
              - "brush_enums.h"
              - "brush_tables.cpp"
              - "and all *_brush.h/cpp files (e.g., ground_brush, wall_brush)"
      - category: "Dialogs & UI Panels"
        features:
          - name: "Palette System (Base Classes)"
            files:
              - "palette_window.h/cpp"
              - "palette_common.h/cpp"
          - name: "Specific Palette Implementations"
            files:
              - "palette_brushlist.h/cpp"
              - "palette_creature.h/cpp"
              - "palette_house.h/cpp"
              - "palette_waypoints.h/cpp"
          - name: "Properties Windows"
            files:
              - "properties_window.h/cpp"
              - "old_properties_window.h/cpp"
              - "container_properties_window.h/cpp"
          - name: "Tools & Utility Dialogs"
            files:
              - "find_*.h/cpp"
              - "replace_*.h/cpp"
              - "browse_tile_window.h/cpp"
              - "add_*.h/cpp"
              - "island_*.h/cpp"
              - "border_*.h/cpp"
              - "ground_validation_dialog.h/cpp"
          - name: "Minimap"
            files:
              - "minimap_window.h/cpp"
          - name: "Preferences & Settings"
            files:
              - "preferences.h/cpp"
              - "settings.h/cpp"
              - "automagic_settings.h/cpp"
      - category: "Networking & Live Edit"
        features:
          - name: "Core Networking & Packets"
            files:
              - "net_connection.h/cpp"
              - "live_packets.h"
              - "live_socket.h/cpp"
              - "rme_net.h/cpp"
          - name: "Live Editing Logic"
            files:
              - "live_action.h/cpp"
              - "live_tab.h/cpp"
              - "live_client.h/cpp"
              - "live_server.h/cpp"
              - "live_peer.h/cpp"

  part_b:
    title: "Part B: The QT6 Migration Specification (The \"TO-BE\" State)"
    target_technology_stack:
      ui_framework: "Qt 6 Widgets (`QtWidgets`)"
      2d_rendering_engine: "`QOpenGLWidget` with direct OpenGL calls"
      xml_parsing: "Qt XML (`QXmlStreamReader`/`QXmlStreamWriter`)"
      compression: "Qt's `qCompress`/`qUncompress` (wraps `zlib`)"
      build_system: "`CMake` (version 3.16+)"
      language_standard: "C++17"
    work_breakdown_structure:
      - section: "Build, Deployment, & Documentation"
        id: "BUILD-01"
        title: "Finalize CMake Build System"
        input_files: "`CMakeLists.txt` from `FINAL-02`"
        dependencies: "FINAL-02"
        definition_of_done: |
          The root CMake build system is finalized. It includes platform-specific logic, dependency finding (`find_package` for Qt6, zlib), setting rpaths for macOS/Linux, and defines build configurations (Debug, Release, RelWithDebInfo).
        boilerplate_coder_ai_prompt: |
          Refine the root `CMakeLists.txt`. Use `find_package(Qt6 REQUIRED Widgets Gui OpenGL)` to locate Qt. Add conditional logic for platform-specific compiler flags and library paths. Implement `CMAKE_INSTALL_RPATH` for macOS and Linux builds.
      - section: "Build, Deployment, & Documentation"
        id: "BUILD-02"
        title: "Implement Packaging and Deployment"
        input_files: "artprovider.cpp, application.cpp (for icon)"
        dependencies: "BUILD-01"
        definition_of_done: |
          The CMake build includes `CPack` configurations to generate platform-native installers: an MSI installer for Windows, a DMG image for macOS, and a `.deb` package for Debian-based Linux. The application icon is correctly bundled.
        boilerplate_coder_ai_prompt: |
          Integrate CPack into the CMake project. Write a `CPackConfig.cmake` file. Define components for the application executable and the `data/` directory. Configure generators for `WIX` (Windows), `DragNDrop` (macOS), and `DEB` (Linux). Set `CPACK_PACKAGE_ICON` using the application icon.
      - section: "Core Migration Tasks"
        id: "CORE-01"
        title: "Isolate Core Data Models"
        input_files: "position.h, tile.h, item.h, item_attributes.h, complexitem.h, creature.h, outfit.h, spawn.h, house.h, town.h, waypoints.h, basemap.h, map_region.h, map.h"
        dependencies: "None"
        definition_of_done: |
          A static library (`mapcore`) compiles via CMake. Contains all data model classes. All `wxWidgets` includes and dependencies are removed. All classes must use `std::string` instead of `wxString`.
        boilerplate_coder_ai_prompt: |
          Refactor the C++ classes from the input files into a new CMake static library named `mapcore`. Remove all `wxWidgets` dependencies, replacing types like `wxString` with `std::string`. Ensure the library compiles cleanly as a self-contained unit.
      - section: "Core Migration Tasks"
        id: "CORE-02"
        title: "Port Asset Database & Parsers"
        input_files: "items.h/cpp, creatures.h/cpp, ext/pugixml.hpp, ext/pugixml.cpp, client_version.h/cpp, graphics.h/cpp"
        dependencies: "CORE-01"
        definition_of_done: |
          The `mapcore` library can parse `items.otb`, `items.xml`, `creatures.xml`, and client version data into memory. The `pugixml` dependency is replaced with `QXmlStreamReader`.
        boilerplate_coder_ai_prompt: |
          Implement `ItemDatabase`, `CreatureDatabase`, and `ClientVersion` classes within `mapcore`. Port the parsing logic from the input files. Replace `pugixml` usage with `QXmlStreamReader`. Replace C-style file I/O with `QFile`.
      - section: "Core Migration Tasks"
        id: "CORE-03"
        title: "Port OTBM/OTMM File I/O"
        input_files: "iomap_otbm.h/cpp, iomap_otmm.h/cpp, filehandle.h/cpp, iomap.h/cpp"
        dependencies: "CORE-02"
        definition_of_done: |
          The `mapcore` library can fully serialize and deserialize `.otbm` and `.otmm` files, populating and reading from the `Map` object defined in `CORE-01`. All `wx` dependencies are removed.
        boilerplate_coder_ai_prompt: |
          Port the `IOMap*` classes to `mapcore`. Replace all `wx` file classes and `FILE*` handles with `QFile` and `QDataStream`. Create unit tests to verify that a saved map can be re-loaded with data integrity.
      - section: "Core Migration Tasks"
        id: "CORE-04"
        title: "Port Brush & Materials System"
        input_files: "brush.h/cpp (and all `*_brush.*`), materials.h/cpp, tileset.h/cpp"
        dependencies: "CORE-02"
        definition_of_done: |
          The Brush and Material/Tileset systems are ported to `mapcore`. All brush types are functional but without UI counterparts. XML loading logic is ported to use `QXmlStreamReader`.
        boilerplate_coder_ai_prompt: |
          Port all brush classes and the `Materials`/`Tileset` system to `mapcore`. Replace `pugixml` parsing logic with `QXmlStreamReader`. Brushes should operate on the `CORE-01` data models.
      - section: "Core Migration Tasks"
        id: "CORE-05"
        title: "Port Action, Selection, Copy/Paste"
        input_files: "action.h/cpp, selection.h/cpp, copybuffer.h/cpp, threads.h"
        dependencies: "CORE-04"
        definition_of_done: |
          The Undo/Redo (`ActionQueue`), `Selection`, and `CopyBuffer` systems are ported to `mapcore`. All logic is decoupled from UI events and operates on core data models. `wxThread` is replaced by `std::thread`.
        boilerplate_coder_ai_prompt: |
          Port the `Action`, `ActionQueue`, `Selection`, and `CopyBuffer` classes. Rewrite `SelectionThread` using `std::thread`. These classes must compile within `mapcore` without any UI framework includes.
      - section: "Data I/O & Management"
        id: "CORE-06"
        title: "Port Creature XML & OTB Integration"
        input_files: "creatures.h/cpp, creature_brush.h/cpp"
        dependencies: "CORE-02, CORE-04"
        definition_of_done: |
          The `CreatureDatabase` is now fully managed within `mapcore`. It can import OT's `monsters.xml` and `npcs.xml` directories and can load/save its state to `creatures.xml`. The `CreatureBrush` operates on this data.
        boilerplate_coder_ai_prompt: |
          In the `io` module of `mapcore`, write logic to recursively scan directories for monster/NPC XML files and parse them using `QXmlStreamReader`. This data must populate the `CreatureDatabase`. Implement save/load functionality for `creatures.xml` to persist custom creature data.
      - section: "Build, Deployment, & Documentation"
        id: "DOCS-01"
        title: "Generate Developer Documentation"
        input_files: "Entire `mapcore` library source"
        dependencies: "CORE-05"
        definition_of_done: |
          Doxygen is integrated into the CMake build. All public classes and methods in the `mapcore` library have Doxygen-style comments. An HTML documentation set can be generated with a `make docs` command.
        boilerplate_coder_ai_prompt: |
          *This is a documentation task. The prompt would be:* "Add Doxygen-style comments to all public headers in the `mapcore` library. Document the purpose of each class, method, and all parameters. Integrate Doxygen with CMake to generate HTML output."
      - section: "Build, Deployment, & Documentation"
        id: "DOCS-02"
        title: "Create Initial User Manual"
        input_files: "`main_menubar.xml` (for features)"
        dependencies: "FINAL-04"
        definition_of_done: |
          A basic user manual is created in Markdown format. It includes sections covering installation, a tour of the new UI, and instructions for using the core drawing and editing tools.
        boilerplate_coder_ai_prompt: |
          *This is a documentation task for a technical writer, not a coding task.*
      - section: "Core Migration Tasks"
        id: "FINAL-01"
        title: "Port Settings & Preferences"
        input_files: "settings.h/cpp, preferences.h/cpp"
        dependencies: "UI-01"
        definition_of_done: |
          The application uses `QSettings` to save and load all user preferences to a platform-appropriate location. The Preferences dialog (`QDialog`) successfully modifies these settings.
        boilerplate_coder_ai_prompt: |
          Refactor the `Settings` class to use `QSettings` as its backend instead of `wxConfig`. Port the `PreferencesWindow` to a `QDialog`, connecting all widgets to `QSettings` read/write calls.
      - section: "Core Migration Tasks"
        id: "FINAL-02"
        title: "Create Final CMake Build System"
        input_files: "(all files)"
        dependencies: "ALL"
        definition_of_done: |
          A root `CMakeLists.txt` is created that correctly builds the `mapcore` static library and the final `RME-Qt` executable. It must locate and link all dependencies (Qt6, OpenGL).
        boilerplate_coder_ai_prompt: |
          Write a top-level `CMakeLists.txt`. Add `mapcore` as a subdirectory library. Create an executable target for the main application and link it against `Qt6::Widgets`, `Qt6::OpenGL`, and `mapcore`.
      - section: "Finalization"
        id: "FINAL-03"
        title: "Port Application Settings Logic"
        input_files: "settings.h/cpp, preferences.h/cpp"
        dependencies: "UI-DIALOGS-02"
        definition_of_done: |
          The application fully replaces `g_settings` with a new `Settings` singleton class that uses `QSettings` for persistence. All editor options from the legacy system are migrated and functional.
        boilerplate_coder_ai_prompt: |
          Create a new singleton class, `AppSettings`. Use `QSettings` as the storage backend. Port all key-value pairs from `Config::Key` enum. Replace all calls to `g_settings` throughout the new codebase with calls to the `AppSettings` singleton.
      - section: "Finalization"
        id: "FINAL-04"
        title: "Implement Theme and Style Management"
        input_files: "dark_mode_manager.h/cpp, application.h/cpp (`FixVersionDiscrapencies`)"
        dependencies: "UI-01"
        definition_of_done: |
          The application supports light and dark themes using Qt Style Sheets (`.qss`). A \"Dark Mode\" checkbox in the preferences dialog toggles the active theme. Custom colors must be supported.
        boilerplate_coder_ai_prompt: |
          Implement a `ThemeManager` class. Create two `.qss` files: `light.qss` and `dark.qss`. In `ThemeManager`, add a method `applyTheme(ThemeType)` that loads a QSS file and applies it to the `QApplication` instance. Connect the preferences dialog to this manager.
      - section: "Finalization"
        id: "FINAL-05"
        title: "Port About and Welcome Dialogs"
        input_files: "about_window.h/cpp, welcome_dialog.h/cpp"
        dependencies: "UI-01"
        definition_of_done: |
          The \"About\" and \"Welcome\" dialogs are re-implemented using `QDialog`. The easter-egg games in `about_window.cpp` (`Tetris`, `Snake`) are to be ported as simple `QWidget`-based games.
        boilerplate_coder_ai_prompt: |
          Re-create the `AboutDialog` and `WelcomeDialog` as Qt dialogs. The embedded games should be re-implemented within simple `QWidget`s using `QPainter` for rendering and `QTimer` for the game loop.
      - section: "Final Polish"
        id: "FINAL-06"
        title: "Implement Status & Progress UI"
        input_files: "gui.cpp (`CreateLoadBar`, `SetStatusText`)"
        dependencies: "UI-01"
        definition_of_done: |
          A `QStatusBar` in `MainWindow` correctly displays mouse coordinates and other info. Long-running operations trigger a non-modal `QProgressDialog` that accurately reflects the progress of the task.
        boilerplate_coder_ai_prompt: |
          In `MainWindow`, use the `QStatusBar` to display messages. Create a wrapper class for long-running operations that shows a `QProgressDialog` and updates it via Qt signals emitted from the worker thread.
      - section: "Final Polish"
        id: "FINAL-07"
        title: "Re-implement Drag-and-Drop and IPC"
        input_files: "application.cpp (`MacOpenFiles`), process_com.h/cpp"
        dependencies: "UI-01, CORE-03"
        definition_of_done: |
          The application can open `.otbm` files by dragging them onto the main window. If another instance is running, the file path is passed to the existing instance for it to open (inter-process communication).
        boilerplate_coder_ai_prompt: |
          Override `dragEnterEvent` and `dropEvent` on `MainWindow`. For IPC, use `QLocalServer` and `QLocalSocket` to create a single-instance lock and pass command-line arguments to the running instance.
      - section: "Editor Behavior"
        id: "LOGIC-01"
        title: "Implement Core Drawing & Deletion Logic"
        input_files: "editor.h/cpp, brush.h/cpp (and all specific `*_brush.cpp` files)"
        dependencies: "UI-EVENT-01, CORE-05"
        definition_of_done: |
          In `DRAWING_MODE`, left-clicking on the map canvas with an active brush executes the brush's `draw()` method, creates an `Action` containing the tile changes, and pushes it to the `ActionQueue`. The `MapView` updates immediately. CTRL+Click executes the `undraw()` method.
        boilerplate_coder_ai_prompt: |
          Create a controller class (`EditorController`) to manage editor state. In `MapView::mousePressEvent`, if in drawing mode, call a method on the controller (e.g., `controller->performDraw(position, brush)`). This controller method must create an `Action`, execute the appropriate `brush->draw()` or `brush->undraw()`, and push the completed `Action` to the `ActionQueue`.
      - section: "Editor Behavior"
        id: "LOGIC-02"
        title: "Implement Bounding-Box Selection"
        input_files: "selection.h/cpp, map_display.cpp"
        dependencies: "UI-EVENT-01, CORE-05"
        definition_of_done: |
          In `SELECTION_MODE`, dragging the mouse on the `MapView` creates a visual bounding box. Releasing the mouse populates the `Selection` object with all `Tile`s and/or `Item`s within that box. This operation is undoable.
        boilerplate_coder_ai_prompt: |
          In `MapView`'s mouse event handlers, detect a drag operation in `SELECTION_MODE`. In `paintGL`, draw a 2D rectangle from the drag-start point to the current cursor position. On `mouseReleaseEvent`, calculate the map area covered by the rectangle and populate the `Selection` object from `CORE-05` via an `Action`.
      - section: "Editor Behavior"
        id: "LOGIC-03"
        title: "Port Cut, Copy, Paste & Move"
        input_files: "copybuffer.h/cpp, editor.cpp, main_menubar.cpp, selection.h/cpp"
        dependencies: "LOGIC-02"
        definition_of_done: |
          Cut, Copy, Paste, and selection drag-and-move operations are fully functional. These actions must be correctly integrated with the `CopyBuffer` and the `ActionQueue` to support undo/redo. Paste must enter a visual \"pasting\" mode.
        boilerplate_coder_ai_prompt: |
          Implement slots in `MainWindow` for `cut`, `copy`, and `paste` actions. Connect them to the respective `QAction`s. Port the logic from `CopyBuffer` and the `moveSelection` function in `editor.cpp`. For pasting, the `MapView` must enter a state to show a ghost of the paste buffer under the cursor.
      - section: "House System"
        id: "LOGIC-05"
        title: "Implement House Management in Core"
        input_files: "house.h/cpp"
        dependencies: "CORE-01"
        definition_of_done: |
          The `mapcore` library contains a `Houses` manager class that can add, remove, and query `House` objects. The `House` data model and its relationship with `Tile`s is fully implemented without UI dependencies.
        boilerplate_coder_ai_prompt: |
          Port the `House` and `Houses` classes to `mapcore`. Refactor the logic to operate solely on the core data models (`Map`, `Tile`). Ensure house data is correctly serialized and deserialized in the `.otbm` I/O tasks. Unit tests must verify house creation and tile assignment.
      - section: "Editor Behavior"
        id: "LOGIC-06"
        title: "Integrate House & Waypoint Brushes"
        input_files: "house_brush.h/cpp, house_exit_brush.h/cpp, waypoint_brush.h/cpp"
        dependencies: "UI-PALETTE-03"
        definition_of_done: |
          When the house or exit brush is active, clicks on the `MapView` correctly assign house tiles or move the house exit. This creates an undoable `Action`. The same applies to placing waypoints.
        boilerplate_coder_ai_prompt: |
          In `MapView::mousePressEvent`, when the active brush is `HouseBrush`, `HouseExitBrush`, or `WaypointBrush`, invoke a controller method to apply the change. The controller must create the appropriate `Change` and wrap it in an `Action`.
      - section: "Editor Behavior"
        id: "LOGIC-07"
        title: "Port Map & Selection Context Menus"
        input_files: "map_display.h/cpp (`MapPopupMenu`), main_menubar.cpp"
        dependencies: "LOGIC-01, LOGIC-02"
        definition_of_done: |
          Right-clicking on the `MapView` brings up a `QMenu` with context-sensitive actions. The menu content must match the logic in `MapPopupMenu::Update`, enabling/disabling items based on the selected object(s).
        boilerplate_coder_ai_prompt: |
          Override `MapView::contextMenuEvent`. Inside the handler, create a `QMenu`. Populate the menu with `QAction`s based on the type and properties of the item(s) under the cursor, using the `Selection` object. The logic from `MapPopupMenu` must be fully replicated.
      - section: "Brush Functionality"
        id: "LOGIC-08"
        title: "Integrate Drawing Modes & Advanced Brushes"
        input_files: "editor.cpp, gui.cpp, all `*_brush.cpp` files"
        dependencies: "LOGIC-01, UI-PALETTE-02"
        definition_of_done: |
          All brush drawing modes (drag-and-draw, square/circle shape) are functional in the `MapView`. Special brushes (wall, table, carpet, door, etc.) apply their automatic bordering and connection logic correctly when used.
        boilerplate_coder_ai_prompt: |
          Port the logic from `MapCanvas::OnMouseActionClick` and `MapCanvas::OnMouseActionRelease`. In the `MapView`'s mouse event handlers, check the `GuiManager` for the active brush and its settings (size, shape) and create a `PositionVector` of tiles to be affected, then pass this to the controller to execute a drawing `Action`.
      - section: "Brush Functionality"
        id: "LOGIC-09"
        title: "Finalize All Tool Implementations"
        input_files: "editor.cpp"
        dependencies: "LOGIC-08"
        definition_of_done: |
          All remaining tools not explicitly covered, such as \"Fill\" and the full suite of \"Map Cleanup\" operations, are fully functional and accessible through the main menu.
        boilerplate_coder_ai_prompt: |
          Port the logic for each remaining tool from `editor.cpp` and `main_menubar.cpp`. Implement each tool's functionality as a method in `EditorController`. Connect the corresponding `QAction` from the main menu to a slot that invokes the controller method.
      - section: "Core Migration Tasks"
        id: "NET-01"
        title: "Isolate and Port Network Protocol"
        input_files: "live_packets.h, net_connection.h/cpp, live_socket.h/cpp"
        dependencies: "CORE-01"
        definition_of_done: |
          A `network` module is created in `mapcore`. It contains the logic for serializing and deserializing network messages (`NetworkMessage`) and defining packet structures, independent of any I/O framework.
        boilerplate_coder_ai_prompt: |
          Refactor `NetworkMessage` and all packet structures/enums into a new `network` module within the `mapcore` library. Ensure this module has no `wxWidgets` or `Boost.Asio` dependencies.
      - section: "Core Migration Tasks"
        id: "NET-02"
        title: "Port Live Server Logic"
        input_files: "live_server.h/cpp, live_peer.h/cpp"
        dependencies: "NET-01, CORE-05"
        definition_of_done: |
          A standalone, headless server application is created using Qt Console. It re-implements the logic from `LiveServer` and `LivePeer` using `QTcpServer` and `QTcpSocket`.
        boilerplate_coder_ai_prompt: |
          Create a new `QCoreApplication` project. Port the `LiveServer` logic. Use `QTcpServer`'s `newConnection()` signal to accept clients. Create a `Peer` class to manage each `QTcpSocket`.
      - section: "Core Migration Tasks"
        id: "NET-03"
        title: "Port Live Client & UI Integration"
        input_files: "live_client.h/cpp, live_tab.h/cpp, live_action.h/cpp"
        dependencies: "NET-01, UI-01, CORE-05"
        definition_of_done: |
          The RME application can connect to the new server. All network communication is handled via `QTcpSocket`. UI updates from the network thread must be posted to the main thread's event loop.
        boilerplate_coder_ai_prompt: |
          Port `LiveClient` to the Qt application. Replace Boost.Asio with `QTcpSocket`. Use signals like `readyRead()` and `disconnected()`. For UI updates, emit a Qt signal from the network thread and connect it to a slot in a `MainWindow` object using `Qt::QueuedConnection`.
      - section: "Post-Migration Refactoring & Optimization"
        id: "REFACTOR-01"
        title: "Decouple Systems from Global Managers"
        input_files: "gui.cpp, all files that include `gui.h`"
        dependencies: "FINAL-04"
        definition_of_done: |
          The `GuiManager` singleton from `UI-03` is eliminated. Its responsibilities (e.g., managing the active brush) are moved into smaller, focused classes. A dependency injection or modern service locator pattern is used in `MainWindow` to provide these services to widgets that need them.
        boilerplate_coder_ai_prompt: |
          Refactor the `GuiManager` singleton. Move brush management logic to a `BrushManager` class. Move UI state logic to a `ViewStateManager` class. `MainWindow` will instantiate these managers and provide pointers or references to child widgets through their constructors, removing the need for a global accessor.
      - section: "Post-Migration Refactoring & Optimization"
        id: "REFACTOR-02"
        title: "Performance & Memory Profiling"
        input_files: "The new, fully functional Qt application."
        dependencies: "FINAL-04, TEST-04"
        definition_of_done: |
          The application is profiled using platform-native tools (e.g., Instruments on macOS, Valgrind/Cachegrind on Linux, VS Profiler on Windows). Major bottlenecks in rendering, data loading, and memory usage are identified and documented in a report.
        boilerplate_coder_ai_prompt: |
          *This is an analysis task for a senior developer/architect, not a direct coding task.*
      - section: "Post-Migration Refactoring & Optimization"
        id: "REFACTOR-03"
        title: "Optimize Rendering Hotspots"
        input_files: "MapView and TextureManager implementation, Profiling Report from `REFACTOR-02`"
        dependencies: "REFACTOR-02"
        definition_of_done: |
          The identified rendering bottlenecks are addressed. This may include implementing more advanced culling (frustum, occlusion), optimizing shader code, and refining the texture atlas management strategy to reduce state changes.
        boilerplate_coder_ai_prompt: |
          Based on the profiling report, optimize the `MapView::paintGL` method. Implement chunk-based frustum culling to avoid iterating over off-screen map areas. If fill-rate is an issue, batch sprite rendering into as few draw calls as possible.
      - section: "Core Migration Tasks"
        id: "RENDER-01"
        title: "Implement OpenGL Viewport"
        input_files: "map_display.h/cpp"
        dependencies: "UI-01, CORE-03"
        definition_of_done: |
          A `MapView` class (`QOpenGLWidget`, `QOpenGLFunctions`) is the central widget. It implements pan (mouse drag), zoom (wheel), and floor change controls.
        boilerplate_coder_ai_prompt: |
          Create `MapView` inheriting from `QOpenGLWidget` and `QOpenGLFunctions`. Add it to `MainWindow`. Implement `mousePressEvent`, `mouseMoveEvent`, and `wheelEvent` to control a 2D camera model.
      - section: "Core Migration Tasks"
        id: "RENDER-02"
        title: "Render Map Data"
        input_files: "map_drawer.h/cpp"
        dependencies: "RENDER-01"
        definition_of_done: |
          `MapView::paintGL` iterates through tiles in the current view and renders them as colored quads. Implements the \"ghosting\" logic for different floors.
        boilerplate_coder_ai_prompt: |
          In `MapView::paintGL`, get visible tiles from a `Map` object. For each tile, calculate its screen position and render a `glDrawArrays(GL_QUADS, ...)` colored based on the tile's properties (e.g., house, pz).
      - section: "Core Migration Tasks"
        id: "RENDER-03"
        title: "Implement Sprite Rendering"
        input_files: "graphics.h/cpp, sprites.h"
        dependencies: "RENDER-02"
        definition_of_done: |
          `MapView` creates and manages a texture atlas for all game sprites. It correctly renders items and creatures from the map data models by drawing textured quads.
        boilerplate_coder_ai_prompt: |
          Create a `TextureManager` class. In `MapView`, during initialization, use the `TextureManager` to build a texture atlas from the loaded sprite data. In `paintGL`, bind the atlas and draw sprites using appropriate texture coordinates.
      - section: "Core Migration Tasks"
        id: "RENDER-04"
        title: "Port All Other Drawing Features"
        input_files: "map_drawer.h/cpp, light_drawer.h/cpp"
        dependencies: "RENDER-03"
        definition_of_done: |
          All remaining visual features from `MapDrawer` are ported: selection boxes, spawn icons, house highlights, tooltips (using `QToolTip` or custom drawing), grid, etc.
        boilerplate_coder_ai_prompt: |
          Replicate all drawing functions from `MapDrawer` in `MapView::paintGL`. Replace `glutBitmapCharacter` with `QPainter::drawText` for tooltips. `LightDrawer` logic must be ported to a fragment shader.
      - section: "Quality Assurance & Testing"
        id: "TEST-01"
        title: "Establish Unit Testing Framework for Core"
        input_files: "N/A (Based on `mapcore` library API)"
        dependencies: "CORE-05"
        definition_of_done: |
          A testing framework (e.g., CTest with Google Test) is integrated into the CMake build. Unit tests are written for critical functions in `mapcore`, including `Map` manipulation, `Tile` state changes, and `ItemDatabase` lookups. Code coverage for the core library is reported.
        boilerplate_coder_ai_prompt: |
          Integrate the Google Test framework with the CMake project. Write unit tests for the `Map` class in the `mapcore` library. Tests must cover adding/removing tiles, finding tiles by position, and verifying map dimensions. Configure CTest to run these tests.
      - section: "Quality Assurance & Testing"
        id: "TEST-02"
        title: "Implement Regression Test Suite for I/O"
        input_files: "Sample `.otbm`, `.otb`, `.xml`, `.dat`, `.spr` files."
        dependencies: "CORE-03"
        definition_of_done: |
          A suite of regression tests is created. Each test loads a sample legacy data file (`.otbm`, `.otb`, etc.), saves it back out using the new system, and performs a binary comparison or structural validation to ensure a byte-for-byte or logically equivalent output.
        boilerplate_coder_ai_prompt: |
          Using the `io` module from `mapcore`, write a series of tests that load a provided set of sample map and asset files. After loading, immediately save the data back to a temporary file. Compare the temporary file with the original to ensure data integrity and format correctness.
      - section: "Quality Assurance & Testing"
        id: "TEST-03"
        title: "Develop Integration & UI Test Plan"
        input_files: "(All UI files for reference)"
        dependencies: "UI-DIALOGS-03"
        definition_of_done: |
          A formal test plan document is created outlining manual testing procedures for all UI features. This includes step-by-step instructions for testing every menu item, dialog interaction, brush behavior, and palette function.
        boilerplate_coder_ai_prompt: |
          *This is a documentation task for the Project Manager, not a coding task.*
      - section: "Quality Assurance & Testing"
        id: "TEST-04"
        title: "Execute Manual Regression Test Plan"
        input_files: "Test plan document from `TEST-03`."
        dependencies: "TEST-03"
        definition_of_done: |
          All tests in the plan are executed on the new Qt application on Windows, Linux, and macOS target platforms. A bug report is generated for any deviations from the legacy application's behavior.
        boilerplate_coder_ai_prompt: |
          *This is a manual testing task for a QA agent, not a coding task.*
      - section: "Core Migration Tasks"
        id: "TOOLS-01"
        title: "Port Borderize & Randomize Tools"
        input_files: "editor.cpp (`borderize*`, `randomize*`), borderize_window.h/cpp"
        dependencies: "LOGIC-02"
        definition_of_done: |
          Menu actions for \"Borderize\" and \"Randomize\" (on both selection and full map) are functional. The `BorderizeWindow` is recreated as a `QDialog` and successfully orchestrates the operation in chunks.
        boilerplate_coder_ai_prompt: |
          Create a `BorderizeDialog` class inheriting from `QDialog`, replicating the UI of `BorderizeWindow` with Qt widgets. Connect the `Map > Borderize` `QAction` to a slot that shows this dialog. Implement the tile iteration and brush application logic from `editor.cpp`.
      - section: "Core Migration Tasks"
        id: "TOOLS-02"
        title: "Port Find/Replace & Cleanup Tools"
        input_files: "find_item_window.h/cpp, replace_items_window.h/cpp, result_window.h/cpp, map.cpp (`clean*`)"
        dependencies: "UI-01, CORE-03"
        definition_of_done: |
          The Find/Replace Items dialog is ported to `QDialog`, and its \"Execute\" functionality correctly searches and replaces items via the `ActionQueue`. The Map Cleanup and Remove Items dialogs are functional.
        boilerplate_coder_ai_prompt: |
          Re-create the `FindItemDialog`, `ReplaceItemsDialog`, and `SearchResultWindow` as Qt widgets. `SearchResultWindow` should be a dockable widget. Connect all UI signals and slots to port the complex find/replace logic.
      - section: "Core Migration Tasks"
        id: "TOOLS-03"
        title: "Port Map & Object Properties"
        input_files: "properties_window.h/cpp, old_properties_window.h/cpp, container_properties_window.h/cpp, gui.cpp"
        dependencies: "UI-PALETTE-01, CORE-02"
        definition_of_done: |
          A non-modal, dockable \"Properties\" widget is created. When an item is selected on the map, this widget is populated with its properties. Editing a property and clicking \"Apply\" creates an undoable `Action`.
        boilerplate_coder_ai_prompt: |
          Create a `PropertiesWidget` inheriting `QWidget`. Place it in a `QDockWidget`. In the `Selection` class, add a Qt signal `selectionChanged(Item*)`. Connect this signal to a slot in `PropertiesWidget` that populates the UI with the item's data. Connect the Apply button to a slot that creates and pushes an `Action`.
      - section: "Core Migration Tasks"
        id: "TOOLS-04"
        title: "Port Generator Dialogs"
        input_files: "island_generator_dialog.h/cpp, monster_generator_dialog.h/cpp, border_editor_window.h/cpp"
        dependencies: "UI-01, CORE-04"
        definition_of_done: |
          The Island Generator, Monster Generator, and Border Editor dialogs are ported to Qt. Their \"Generate\" or \"Save\" actions correctly modify the in-memory map data via the `ActionQueue`.
        boilerplate_coder_ai_prompt: |
          For each dialog, create a new `QDialog` subclass. Replicate the UI layout using `QGridLayout` and `QVBoxLayout`. Port the generation logic and connect it to the dialog's `accepted()` signal, ensuring the changes are wrapped in an `Action`.
      - section: "Core Migration Tasks"
        id: "UI-01"
        title: "Create Qt Application Shell & Main Window"
        input_files: "application.h/cpp, gui.h/cpp"
        dependencies: "None"
        definition_of_done: |
          A new Qt 6 Widgets application launches a `QMainWindow`. This `MainWindow` will act as the new GUI handler, replacing the global `g_gui` object. It includes a basic `CMakeLists.txt`.
        boilerplate_coder_ai_prompt: |
          Create a new Qt 6 Widgets CMake project. The main class should be `MainWindow` inheriting from `QMainWindow`. Create placeholder member functions that will eventually replace `g_gui` functionality.
      - section: "Core Migration Tasks"
        id: "UI-02"
        title: "Implement Menu & Toolbar System"
        input_files: "main_menubar.h/cpp, main_toolbar.h/cpp"
        dependencies: "UI-01"
        definition_of_done: |
          The `MainWindow` populates its `QMenuBar` and `QToolBar` widgets based on the structure and actions defined in `menubar.xml`. `QAction` objects are created but connected to stub slots.
        boilerplate_coder_ai_prompt: |
          Write a parser using `QXmlStreamReader` to read `menubar.xml`. Dynamically create `QMenu` and `QAction` objects and add them to the `MainWindow` menu bar and toolbars. Map string actions to a `std::map`.
      - section: "UI Polish & Completeness"
        id: "UI-03"
        title: "Implement Application Global State Manager"
        input_files: "gui.h/cpp"
        dependencies: "UI-01"
        definition_of_done: |
          A new singleton class, `GuiManager`, is created to manage global UI state such as the active brush, brush shape/size, and action ID. It replaces the role of the `g_gui` global object. `QObject`-based signals are used for state changes.
        boilerplate_coder_ai_prompt: |
          Create a `GuiManager` class inheriting from `QObject`. Move all state-related members from the legacy `GUI` class into it (e.g., `current_brush`, `brush_size`, `action_id`). Convert state changes into Qt signals (e.g., `activeBrushChanged(Brush*)`). Create a global instance.
      - section: "UI Polish & Completeness"
        id: "UI-04"
        title: "Port Minimap Window"
        input_files: "minimap_window.h/cpp"
        dependencies: "UI-01, RENDER-02"
        definition_of_done: |
          A `Minimap` widget is created as a `QDockWidget`. It renders a top-down view of the current floor using the same sprite data as `MapView`. Clicks on the minimap navigate the main `MapView`. Caching must be implemented for performance.
        boilerplate_coder_ai_prompt: |
          Create a `MinimapWidget` (`QWidget`). In its `paintEvent`, render a scaled-down representation of the `Map` by drawing colored pixels for each tile based on its `minimapColor`. Override `mousePressEvent` to calculate the clicked map coordinate and emit a signal to navigate `MapView`.
      - section: "Core Migration Tasks"
        id: "UI-DIALOGS-01"
        title: "Port All Modal Dialogs"
        input_files: "All `*_window.h/cpp` and `*_dialog.h/cpp` files."
        dependencies: "UI-01"
        definition_of_done: |
          All modal dialogs (Properties, Find Item, Replace, Preferences, etc.) are recreated as `QDialog` subclasses. The UI layout and functionality are replicated using `QtWidgets`.
        boilerplate_coder_ai_prompt: |
          For each dialog in the input files, create a new class inheriting `QDialog`. Replicate the layout using `QLayout` (e.g., `QFormLayout`, `QVBoxLayout`). Connect signals from buttons (`clicked()`) to slots.
      - section: "House System"
        id: "UI-DIALOGS-02"
        title: "Port House & Town Editing Dialogs"
        input_files: "palette_house.h/cpp (`EditHouseDialog`), common_windows.h/cpp (`EditTownsDialog`)"
        dependencies: "UI-PALETTE-03"
        definition_of_done: |
          The \"Edit House\" and \"Towns\" dialogs are re-implemented as `QDialog` subclasses. All functionality for creating, editing, removing, and moving houses between towns is ported and functional.
        boilerplate_coder_ai_prompt: |
          Re-create the `EditHouseDialog` and `EditTownsDialog` using `QDialog`. Replicate all UI elements with their `Qt` equivalents (`QLineEdit`, `QSpinBox`, etc.). Connect UI signals to slots that modify the `House` and `Town` objects within the `Map`, wrapping changes in an `Action`.
      - section: "UI Polish & Completeness"
        id: "UI-DIALOGS-03"
        title: "Port Remaining Utility Dialogs"
        input_files: "add_item_window.h/cpp, add_tileset_window.h/cpp, ground_validation_dialog.h/cpp"
        dependencies: "UI-01"
        definition_of_done: |
          All remaining utility dialogs (`Add Item`, `Add Tileset`, `Ground Validation`) are re-implemented as `QDialog` subclasses. Their functionality is ported to interact with the new `mapcore` data models and `ActionQueue`.
        boilerplate_coder_ai_prompt: |
          For each dialog in the input files, create a new `QDialog` subclass. Replicate the UI and logic. Connect the \"OK\" or \"Apply\" button's `clicked()` signal to a slot that creates and pushes an `Action` to the `ActionQueue` to perform the modification.
      - section: "Core Migration Tasks"
        id: "UI-EVENT-01"
        title: "Port Map View User Input & Mode Switching"
        input_files: "map_display.h/cpp, gui.h/cpp, editor.h/cpp"
        dependencies: "RENDER-03, CORE-04"
        definition_of_done: |
          Mouse events (clicks, drags, wheel) on `MapView` are correctly translated to map coordinates and floor levels. The application state can switch between `DRAWING_MODE` and `SELECTION_MODE`. All keydown events (`OnKeyDown`) for camera and brush control are functional.
        boilerplate_coder_ai_prompt: |
          In the `MapView` class, override `mousePressEvent`, `mouseMoveEvent`, `mouseReleaseEvent`, and `wheelEvent`. Re-implement the coordinate translation logic from `MapCanvas::ScreenToMap`. Implement a state machine or enum in a controller class to manage `DRAWING` vs `SELECTION` modes, mirroring the logic in `gui.cpp`. Connect `QAction`s to toggle these modes.
      - section: "Core Migration Tasks"
        id: "UI-PALETTE-01"
        title: "Create Base Palette & Brush Panels"
        input_files: "palette_window.h/cpp, palette_common.h/cpp"
        dependencies: "UI-01, CORE-04"
        definition_of_done: |
          A dockable palette is created with a `QTabWidget`. A base `BrushPanel` class using `QListView` with a custom delegate (`QStyledItemDelegate`) is implemented to display brush icons and names.
        boilerplate_coder_ai_prompt: |
          Create a `PaletteDock` (`QDockWidget`) in `MainWindow`. Add a `QTabWidget`. Create a base `BrushPaletteWidget` containing a `QListView`. Implement a `QStyledItemDelegate` to custom-draw brush entries.
      - section: "Core Migration Tasks"
        id: "UI-PALETTE-02"
        title: "Port Specific Palettes"
        input_files: "palette_brushlist.h/cpp, palette_creature.h/cpp, palette_house.h/cpp, palette_waypoints.h/cpp"
        dependencies: "UI-PALETTE-01"
        definition_of_done: |
          All individual palettes (Terrain, Doodad, Creature, etc.) are ported as separate widgets and added as tabs to the main palette `QTabWidget`. UI elements for each are replicated with Qt equivalents.
        boilerplate_coder_ai_prompt: |
          For each palette type, create a new `QWidget` inheriting from `BrushPaletteWidget`. Replicate the specific UI controls (e.g., sliders, checkboxes) from the legacy files using Qt widgets.
      - section: "House System"
        id: "UI-PALETTE-03"
        title: "Implement House & Town Palette"
        input_files: "palette_house.h/cpp"
        dependencies: "UI-PALETTE-01, LOGIC-05"
        definition_of_done: |
          A new `HousePalette` widget is added to the palette `QTabWidget`. It displays towns in a `QComboBox` and a list of houses (`QListWidget`) for the selected town. Users can select a house to activate a house brush.
        boilerplate_coder_ai_prompt: |
          Create a `HousePaletteWidget` for the palette `QTabWidget`. Populate a `QComboBox` with `Town` data and a `QListWidget` with `House` data from the `Map` object. Selecting a house must activate the `HouseBrush` and update the application's current brush state.

  project_conclusion:
    title: "Project Conclusion & Final Definition of Done"
    description: "This concludes the exhaustive System Modernization Specification. The successful completion of all tasks in the Work Breakdown Structure will result in a fully modernized application that replaces the legacy `wxWidgets` implementation with a `Qt 6`-based system."
    final_deliverable:
      - "A cross-platform C++17 application, built with CMake, that replicates all data handling, editing, rendering, and tooling features of the original application."
      - "The new system will feature a decoupled architecture with a distinct core logic library and a Qt-based UI."
      - "It will be fully testable, deployable via native installers, and documented for future development and user support."
      - "All tasks must be completed and validated."
```