# RME-Qt6 Migration: AI-Auditor Workflow

This document outlines the workflow, coding mandates, and task tracking for the Remere's Map Editor (RME) migration to Qt6, facilitated by an AI Coder (Jules) and reviewed by an Auditor (User).

-   **Participants**:
    -   Coder: Jules (AI)
    -   Auditor: User (Human Reviewer)
-   **Objective**: To define a standardized workflow and clear mandates for porting RME functionalities to Qt6, ensuring quality, consistency, and adherence to project goals.
-   **References**:
    -   Task Definitions: `enhanced_wbs_yaml_files/` directory. Each YAML file details a specific Work Breakdown Structure (WBS) item.
    -   Task Sequence: `tasks_execution_order.md` provides a suggested order for tackling WBS items based on dependencies.
-   **Project Paths**:
    -   Original wxWidgets Source Code: `wxwidgets/`
    -   New Qt6 Source Code Target: `Project_QT/src/` (Jules will place new/ported code within this path, organized into subdirectories like `core`, `ui`, `editor_logic`, `network`, `tests`, etc., as appropriate for each task.)

## Core Coding Mandates for Jules (AI Coder)

These mandates must be adhered to for all coding tasks:

1.  **M1: Scope Adherence & Feature Parity**:
    *   Implement only the functionality specified in the assigned WBS YAML file.
    *   Aim for feature parity with the original wxWidgets version as described in the YAML, unless explicitly told otherwise. Do not add new features or remove existing ones without instruction.
    *   Address all points in the "qt6_migration_steps" and ensure the "definition_of_done" criteria are met.

2.  **M2: Completeness & Robustness**:
    *   Ensure all necessary headers, source files, and build system changes (CMake) are provided.
    *   Implement basic error handling (e.g., for file I/O, network issues, invalid input) where appropriate.
    *   Code should be complete and functional for the defined scope. Avoid placeholder or stubbed-out logic unless the WBS item is specifically for interface definition.

3.  **M3: Code Location & Structure**:
    *   Place new Qt6 source code in the agreed project path: `Project_QT/src/`.
    *   Organize code into logical subdirectories (e.g., `Project_QT/src/mapcore/`, `Project_QT/src/ui/dialogs/`, `Project_QT/src/editor/`).
    *   Follow consistent naming conventions for files and classes (e.g., Qt naming conventions, or as established by the Auditor).

4.  **M4: Qt6/C++ Best Practices**:
    *   Utilize modern C++17 features where appropriate.
    *   Employ Qt6 classes, signals/slots, and idioms correctly.
    *   Prioritize clarity, readability, and maintainability.
    *   Add Doxygen-compatible comments for all public APIs (classes, methods, enums) in header files.

5.  **M5: Reference Original Code & Analyze Existing Qt Code**:
    *   Use the provided original wxWidgets code snippets (`original_input_files` in YAML) as a reference for understanding behavior and logic.
    *   When modifying existing Qt code (e.g., in `FINAL-*` integration tasks), understand the existing structure before making changes.

6.  **M6: Modularity & File Size**:
    *   Strive for modular design. Create new classes/files as needed to encapsulate functionality.
    *   If a single generated file (e.g., a complex dialog's source) is anticipated to be excessively large (e.g., >1000 lines), notify the Auditor. This might indicate a need to break down the task or class.

7.  **M7: Dependency Management**:
    *   If a task creates new components that other future tasks might depend on, clearly define these outputs (e.g., new class names, library names). If new YAML files are needed for sub-components, propose their creation.
    *   Use CMake for managing dependencies between components/libraries within the project.

8.  **M8: Testing**:
    *   For `CORE-*`, `LOGIC-*`, `NET-*`, and `RENDER-*` tasks that implement non-trivial logic, provide basic unit tests using the Qt Test framework. Place these in an appropriate `tests/` subdirectory.
    *   `TEST-*` WBS items are dedicated to more comprehensive testing but core logic should have initial developer-level tests.

## Standard Task Workflow

1.  **Assignment**: Auditor assigns a WBS task ID to Jules.
2.  **Implementation**: Jules analyzes the YAML, references original code, and implements the Qt6 solution according to the mandates. Jules will use the available tools (`ls`, `read_files`, `create_file_with_block`, `overwrite_file_with_block`, `delete_file`, `rename_file`, `replace_with_git_merge_diff`, `run_in_bash_session`, `view_text_website`) to interact with the codebase.
3.  **Submission**: Upon completing the task, Jules signals readiness:
    `TASK_COMPLETE: <TaskID> - <Brief summary of implementation>. Branch: <branch_name_if_applicable>. Ready for review.`
    (Jules will submit a detailed report using `submit_subtask_report` tool, the above is a conceptual signal).
4.  **Review**: Auditor reviews the submitted code, CMake changes, and any accompanying tests or documentation.
5.  **Feedback**:
    *   If approved: `APPROVED: <TaskID>`
    *   If revisions are needed: `REWORK: <TaskID> - [Specific, actionable feedback items]`
6.  **Iteration**: If REWORK, Jules addresses the feedback and re-submits.

## Notes on Special Task Types

-   **DOCS-\***: Jules will generate Markdown content or Doxygen configuration/comments.
-   **TEST-\***: Jules will generate C++ Qt Test code, or provide instructions for manual testing procedures if UI testing.
-   **REFACTOR-\***: Jules will modify existing Qt6 code based on specific instructions in the YAML.
-   **BUILD-\***: Jules will primarily generate/modify CMakeLists.txt files and related build scripts.
-   **FINAL-\***: These are integration tasks. Jules will modify existing Qt6 code to connect different components, ensuring they work together as described in the YAML.

## Communication Guidelines

-   Jules should be verbose in thought processes before each tool use.
-   Auditor will provide clarifications via updated YAML descriptions or direct instructions if Jules is blocked or misinterprets a task.
-   Focus on clear, actionable instructions and feedback.

---

## Task Kanban Board

All tasks from `tasks_execution_order.md` are listed here. This board will be updated as tasks progress.

### Backlog (To Do)
- [ ] CORE-06 (Port Settings & Preferences System)
- [ ] CORE-BRUSH-FRAMEWORK (Port Base Brush Class, Implement BrushManager, and Define BrushSettings)
- [ ] DOCS-01 (Generate Developer API Documentation (Doxygen))
- [ ] LOGIC-04 (Port Waypoint System)
- [ ] NET-01 (Isolate and Port Network Protocol Definition and Serialization Logic)
- [ ] RENDER-01 (Implement Core OpenGL Viewport and Navigation (MapView))
- [ ] TEST-01 (Develop Unit Tests for Core Qt6 Data Structures)
- [ ] UI-01 (Port Main Application Window (QMainWindow) and Dynamic Menu Bar from XML)
- [ ] BUILD-02 (Implement Packaging and Deployment)
- [ ] CORE-04 (Port Action & History (Undo/Redo) System)
- [ ] CORE-07-MapIO (Port OTBM Map I/O System)
- [ ] CORE-08-CreatureOutfit (Port Creature Instance and Outfit Classes)
- [ ] CORE-11-WaypointSystem (Port Waypoint System Data Structures)
- [ ] CORE-14-MaterialSystem (Port Material System Data Structures)
- [ ] DOCS-02 (Create User Manual and Feature Documentation)
- [ ] LOGIC-01 (Implement Core Drawing, Deletion, and Modification Logic Controller)
- [ ] NET-02 (Port Live Collaboration Server Logic to Qt Network)
- [ ] REFACTOR-01 (Decouple UI State and Services from Global Access (Refactor Conceptual GuiManager))
- [ ] RENDER-02 (Implement Basic Tile Rendering (Colored Quads with Ghosting))
- [ ] TEST-02 (Develop Unit Tests for Asset Loading and Parsing Logic)
- [ ] TEST-06 (Develop Unit Tests for Network Protocol Message Handling)
- [ ] UI-02 (Port Main Application Toolbars and Comprehensive Palette System)
- [ ] UTIL-01-JsonReplacement (Replace json_spirit with Qt JSON for ClientVersion Settings)
- [ ] CORE-05 (Port Selection & Copy/Paste System)
- [ ] CORE-09-HouseSystem (Port House System Data Structures)
- [ ] CORE-10-SpawnSystem (Port Spawn System Data Structures)
- [ ] CORE-12-ComplexItemSystem (Port Complex Item System Data Structures)
- [ ] CORE-13-TownSystem (Port Town System Data Structures)
- [ ] CORE-15-MapRegionSystem (Port Map Region Logic)
- [ ] LOGIC-02 (Implement Bounding-Box Selection Logic)
- [ ] NET-03 (Port Live Collaboration Client and UI Integration to Qt)
- [ ] PALETTE-BrushList (Implement Brush List Palette)
- [ ] REFACTOR-02 (Define and Execute Performance & Memory Profiling Strategy for Qt Application)
- [ ] RENDER-03 (Implement Sprite Rendering using Texture Atlases)
- [ ] TEST-03 (Develop Unit Tests for OTBM and OTMM File I/O)
- [ ] UI-05 (Port Brush & Material Editor)
- [ ] UI-06 (Port Creature Palette and Placed Creature Editor Dialog)
- [ ] UI-EditorWindow (Implement Editor Window / Map Document Area)
- [ ] UI-MapViewWidget (Implement Map View Widget (Interactive UI Shell))
- [ ] BRUSH-LOGIC-Carpet (Port Carpet Brush Logic)
- [ ] BRUSH-LOGIC-Creature (Port Creature Brush Logic)
- [ ] BRUSH-LOGIC-Doodad (Port Doodad Brush Logic)
- [ ] BRUSH-LOGIC-Eraser (Port Eraser Brush Logic)
- [ ] BRUSH-LOGIC-Ground (Port Ground Brush Logic)
- [ ] BRUSH-LOGIC-House (Port House Brush Logic)
- [ ] BRUSH-LOGIC-HouseExit (Port House Exit Brush/Tool Logic)
- [ ] BRUSH-LOGIC-RAW (Port RAW Brush Logic)
- [ ] BRUSH-LOGIC-Spawn (Port Spawn Brush Logic)
- [ ] BRUSH-LOGIC-Table (Port Table Brush Logic)
- [ ] BRUSH-LOGIC-Wall (Port Wall Brush Logic)
- [ ] BRUSH-LOGIC-Waypoint (Port Waypoint Brush/Tool Logic)
- [ ] FINAL-01 (Integrate Core Map Logic with Main Qt UI)
- [ ] LOGIC-03 (Implement Cut, Copy, Paste, Delete, and Drag-Move Logic)
- [ ] LOGIC-05 (Port House System Logic & Data Management)
- [ ] LOGIC-07 (Port Creature & Spawn System (Data and Brushes))
- [ ] LOGIC-09 (Port Map-Wide Tools and Operations (Cleanup, Properties, Statistics))
- [ ] PALETTE-Item (Implement General Item Palette/Browser)
- [ ] RENDER-04-LightingSystem (Port Lighting System)
- [ ] TEST-04 (Develop Unit Tests for Brush Application and Material System Logic)
- [ ] UI-07 (Port House Palette, Waypoint Palette, and EditHouseDialog)
- [ ] UI-10 (Define RAW Items Palette and Terrain Brushes Palette)
- [ ] UI-MinimapView (Implement Minimap View Widget)
- [ ] FINAL-02 (Implement File Menu Operations (New, Open, Save, Save As, Close))
- [ ] REFACTOR-03 (Implement Rendering Optimizations Based on Profiling Report)
- [ ] TEST-05 (Develop Unit Tests for Map Actions and Selection Logic)
- [ ] TEST-07 (Develop Integration Tests for Map Rendering System)
- [ ] UI-09 (Port Live Server Control Panel)
- [ ] UI-DIALOGS-LIVE-CONNECT (Implement Live Collaboration Connection Dialog)
- [ ] FINAL-03 (Implement Edit Menu Operations & Link Preferences Dialog)
- [ ] FINAL-04 (Implement View Menu & Common Tools Menu Operations)
- [ ] TEST-08 (Develop Integration Tests for Live Collaboration Server and Client)
- [ ] UI-04 (Port Item, Creature, and Spawn Properties Dialogs)
- [ ] UI-08 (Port Spawn Creation Settings and Spawn Properties Editor)
- [ ] FINAL-05 (Implement Live Collaboration Client Functionality)
- [ ] FINAL-06 (Implement About Dialog & Welcome Screen)
- [ ] TEST-09 (Develop UI Tests for Key Editor Features and Workflows)
- [ ] FINAL-07 (Apply Qlementine Styling to Application)
- [ ] TEST-10 (Execute and Document Cross-Platform UI Compatibility Tests)

### In Progress
- [ ] CORE-03 (Port Map Data Structure & Non-Rendering Logic)

### In Review
- None

### Completed
- [x] BUILD-00 (Create Root CMakeLists.txt and Basic Project Structure) - Branch: feat/build-00-initial-structure
- [x] CORE-01 (Port Core Data Structures - Position, Item, Tile) - Branch: feat/core-01-data-structures
- [x] START (Project Start and Overall WBS Definition) - Branch: docs/start-task-acknowledged
- [x] BUILD-01 (Setup Initial CMake Build System for Qt6 Project) - Branch: feat/build-01-cmake-setup
- [x] CORE-02 (Port Asset Database & Parsers (Items, Creatures, Sprites, Client Versions)) - Branch: feat/core-02-asset-pipeline

### Blocked / Needs Discussion
- None
---
The "Tasks with Unresolved Dependencies" section has been removed from the Kanban board part as the `LOGIC-01` dependency on `BRUSH-LOGIC-ALL` was resolved by removing it, and the new task list in `tasks_execution_order.md` (which this Kanban is based on) reflects this. The specific brush logic tasks are now dependencies of `LOGIC-01`.
