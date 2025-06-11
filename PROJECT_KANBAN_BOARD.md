<?xml version="1.0" encoding="UTF-8"?>
<AIAuditorWorkflow id="RME-Qt6-Migration">
  <Preamble>
    <Title># RME-Qt6 Migration: AI-Auditor Workflow</Title>
    <Introduction>
      This document outlines the workflow, coding mandates, and task tracking for the Remere's Map Editor (RME) migration to Qt6, facilitated by an AI Coder (Jules) and reviewed by an Auditor (User).
    </Introduction>
    <ProjectInfo>
      <Participants>
        <Participant role="Coder">Jules (AI)</Participant>
        <Participant role="Auditor">User (Human Reviewer)</Participant>
      </Participants>
      <Objective>
        To define a standardized workflow and clear mandates for porting RME functionalities to Qt6, ensuring quality, consistency, and adherence to project goals.
      </Objective>
      <References>
        <Reference type="Directory" path="enhanced_wbs_XML_files/">Task Definitions: `enhanced_wbs_XML_files/` directory. Each XML file details a specific Work Breakdown Structure (WBS) item.</Reference>
        <Reference type="File" path="tasks_execution_order.md">Task Sequence: `tasks_execution_order.md` provides a suggested order for tackling WBS items based on dependencies.</Reference>
      </References>
      <ProjectPaths>
        <Path type="OriginalSource">Original wxWidgets Source Code: `wxwidgets/`</Path>
        <Path type="NewSourceTarget">New Qt6 Source Code Target: `Project_QT/src/` (Jules will place new/ported code within this path, organized into subdirectories like `core`, `ui`, `editor_logic`, `network`, `tests`, etc., as appropriate for each task.)</Path>
      </ProjectPaths>
    </ProjectInfo>
  </Preamble>

  <MandatesSection>
    <Title>## Core Coding Mandates for Jules (AI Coder)</Title>
    <Introduction>These mandates must be adhered to for all coding tasks:</Introduction>
    <MandateList>
      <Mandate id="M1">
        <Title>1. M1: Scope Adherence & Feature Parity</Title>
        <Requirement>Implement only the functionality specified in the assigned WBS XML file.</Requirement>
        <Requirement>Aim for feature parity with the original wxWidgets version as described in the XML, unless explicitly told otherwise. Do not add new features or remove existing ones without instruction.</Requirement>
        <Requirement>Address all points in the "qt6_migration_steps" and ensure the "definition_of_done" criteria are met.</Requirement>
      </Mandate>
      <Mandate id="M2">
        <Title>2. M2: Completeness & Robustness</Title>
        <Requirement>Ensure all necessary headers, source files, and build system changes (CMake) are provided.</Requirement>
        <Requirement>Implement basic error handling (e.g., for file I/O, network issues, invalid input) where appropriate.</Requirement>
        <Requirement>Code should be complete and functional for the defined scope. Avoid placeholder or stubbed-out logic unless the WBS item is specifically for interface definition.</Requirement>
      </Mandate>
      <Mandate id="M3">
        <Title>3. M3: Code Location & Structure</Title>
        <Requirement>Place new Qt6 source code in the agreed project path: `Project_QT/src/`.</Requirement>
        <Requirement>Organize code into logical subdirectories (e.g., `Project_QT/src/mapcore/`, `Project_QT/src/ui/dialogs/`, `Project_QT/src/editor/`).</Requirement>
        <Requirement>Follow consistent naming conventions for files and classes (e.g., Qt naming conventions, or as established by the Auditor).</Requirement>
      </Mandate>
      <Mandate id="M4">
        <Title>4. M4: Qt6/C++ Best Practices</Title>
        <Requirement>Utilize modern C++17 features where appropriate.</Requirement>
        <Requirement>Employ Qt6 classes, signals/slots, and idioms correctly.</Requirement>
        <Requirement>Prioritize clarity, readability, and maintainability.</Requirement>
        <Requirement>Add Doxygen-compatible comments for all public APIs (classes, methods, enums) in header files.</Requirement>
      </Mandate>
      <Mandate id="M5">
        <Title>5. M5: Reference Original Code & Analyze Existing Qt Code</Title>
        <Requirement>Use the provided original wxWidgets code snippets (`original_input_files` in XML) as a reference for understanding behavior and logic.</Requirement>
        <Requirement>When modifying existing Qt code (e.g., in `FINAL-*` integration tasks), understand the existing structure before making changes.</Requirement>
      </Mandate>
      <Mandate id="M6">
        <Title>6. M6: Modularity & File Size</Title>
        <Requirement>Strive for modular design. Create new classes/files as needed to encapsulate functionality.</Requirement>
        <Requirement>If a single generated file (e.g., a complex dialog's source) is anticipated to be excessively large (e.g., >1000 lines), notify the Auditor. This might indicate a need to break down the task or class.</Requirement>
      </Mandate>
      <Mandate id="M7">
        <Title>7. M7: Dependency Management</Title>
        <Requirement>If a task creates new components that other future tasks might depend on, clearly define these outputs (e.g., new class names, library names). If new XML files are needed for sub-components, propose their creation.</Requirement>
        <Requirement>Use CMake for managing dependencies between components/libraries within the project.</Requirement>
      </Mandate>
      <Mandate id="M8">
        <Title>8. M8: Testing</Title>
        <Requirement>For `CORE-*`, `LOGIC-*`, `NET-*`, and `RENDER-*` tasks that implement non-trivial logic, provide basic unit tests using the Qt Test framework. Place these in an appropriate `tests/` subdirectory.</Requirement>
        <Requirement>`TEST-*` WBS items are dedicated to more comprehensive testing but core logic should have initial developer-level tests.</Requirement>
      </Mandate>
    </MandateList>
  </MandatesSection>

  <WorkflowProcedure name="StandardTaskWorkflow">
    <Title>## Standard Task Workflow</Title>
    <Steps>
      <Step sequence="1">
        <Title>Assignment</Title>
        <Description>Auditor assigns a WBS task ID to Jules.</Description>
      </Step>
      <Step sequence="2">
        <Title>Implementation</Title>
        <Description>Jules analyzes the XML, references original code, and implements the Qt6 solution according to the mandates. Jules will use the available tools (`ls`, `read_files`, `create_file_with_block`, `overwrite_file_with_block`, `delete_file`, `rename_file`, `replace_with_git_merge_diff`, `run_in_bash_session`, `view_text_website`) to interact with the codebase.</Description>
      </Step>
      <Step sequence="3">
        <Title>Submission</Title>
        <Description>Upon completing the task, Jules signals readiness:</Description>
        <Example>`TASK_COMPLETE: <TaskID> - <Brief summary of implementation>. Branch: <branch_name_if_applicable>. Ready for review.`
(Jules will submit a detailed report using `submit_subtask_report` tool, the above is a conceptual signal).</Example>
      </Step>
      <Step sequence="4">
        <Title>Review</Title>
        <Description>Auditor reviews the submitted code, CMake changes, and any accompanying tests or documentation.</Description>
      </Step>
      <Step sequence="5">
        <Title>Feedback</Title>
        <Outcome type="Approval">If approved: `APPROVED: <TaskID>`</Outcome>
        <Outcome type="Rework">If revisions are needed: `REWORK: <TaskID> - [Specific, actionable feedback items]`</Outcome>
      </Step>
      <Step sequence="6">
        <Title>Iteration</Title>
        <Description>If REWORK, Jules addresses the feedback and re-submits.</Description>
      </Step>
    </Steps>
  </WorkflowProcedure>

  <SpecialTaskNotes>
    <Title>## Notes on Special Task Types</Title>
    <TaskTypeInfo prefix="DOCS-*">
      <Description>Jules will generate Markdown content or Doxygen configuration/comments.</Description>
    </TaskTypeInfo>
    <TaskTypeInfo prefix="TEST-*">
      <Description>Jules will generate C++ Qt Test code, or provide instructions for manual testing procedures if UI testing.</Description>
    </TaskTypeInfo>
    <TaskTypeInfo prefix="REFACTOR-*">
      <Description>Jules will modify existing Qt6 code based on specific instructions in the XML.</Description>
    </TaskTypeInfo>
    <TaskTypeInfo prefix="BUILD-*">
      <Description>Jules will primarily generate/modify CMakeLists.txt files and related build scripts.</Description>
    </TaskTypeInfo>
    <TaskTypeInfo prefix="FINAL-*">
      <Description>These are integration tasks. Jules will modify existing Qt6 code to connect different components, ensuring they work together as described in the XML.</Description>
    </TaskTypeInfo>
  </SpecialTaskNotes>
  
  <CommunicationGuidelines>
    <Title>## Communication Guidelines</Title>
    <Guideline>Jules should be verbose in thought processes before each tool use.</Guideline>
    <Guideline>Auditor will provide clarifications via updated XML descriptions or direct instructions if Jules is blocked or misinterprets a task.</Guideline>
    <Guideline>Focus on clear, actionable instructions and feedback.</Guideline>
  </CommunicationGuidelines>

## Task Kanban Board

All tasks from `tasks_execution_order.md` are listed here. This board will be updated as tasks progress.

### Backlog (To Do)
- [ ] UTIL-01-JsonReplacement (Replace json_spirit with Qt JSON for ClientVersion Settings)
- [ ] CORE-05 (Port Selection & Copy/Paste System)
- [ ] CORE-09-HouseSystem (Port House System Data Structures)
- [ ] CORE-10-SpawnSystem (Port Spawn System Data Structures)
- [ ] CORE-12-ComplexItemSystem (Port Complex Item System Data Structures)
- [ ] CORE-13-TownSystem (Port Town System Data Structures)
- [ ] CORE-15-MapRegionSystem (Port Map Region Logic)
- [ ] RENDER-02 (Implement Basic Tile Rendering (Colored Quads with Ghosting))
- [ ] UI-02 (Port Main Application Toolbars and Comprehensive Palette System)
- [ ] BRUSH-LOGIC-RAW (Port RAW Brush Logic)
- [ ] BRUSH-LOGIC-Eraser (Port Eraser Brush Logic)
- [ ] BRUSH-LOGIC-Spawn (Port Spawn Brush Logic)
- [ ] BRUSH-LOGIC-Waypoint (Port Waypoint Brush/Tool Logic)
- [ ] BRUSH-LOGIC-House (Port House Brush Logic)
- [ ] BRUSH-LOGIC-HouseExit (Port House Exit Brush/Tool Logic)
- [ ] BRUSH-LOGIC-Creature (Port Creature Brush Logic)
- [ ] BRUSH-LOGIC-Ground (Port Ground Brush Logic)
- [ ] BRUSH-LOGIC-Carpet (Port Carpet Brush Logic)
- [ ] BRUSH-LOGIC-Doodad (Port Doodad Brush Logic)
- [ ] BRUSH-LOGIC-Table (Port Table Brush Logic)
- [ ] BRUSH-LOGIC-Wall (Port Wall Brush Logic)
- [ ] LOGIC-01 (Implement Core Drawing, Deletion, and Modification Logic Controller)
- [ ] LOGIC-02 (Implement Bounding-Box Selection Logic)
- [ ] LOGIC-05 (Port House System Logic & Data Management)
- [ ] LOGIC-07 (Port Creature & Spawn System (Data and Brushes))
- [ ] NET-03 (Port Live Collaboration Client and UI Integration to Qt)
- [ ] PALETTE-BrushList (Implement Brush List Palette)
- [ ] RENDER-03 (Implement Sprite Rendering using Texture Atlases)
- [ ] UI-EditorWindow (Implement Editor Window / Map Document Area)
- [ ] UI-MapViewWidget (Implement Map View Widget (Interactive UI Shell))
- [ ] FINAL-01 (Integrate Core Map Logic with Main Qt UI)
- [ ] LOGIC-03 (Implement Cut, Copy, Paste, Delete, and Drag-Move Logic)
- [ ] LOGIC-09 (Port Map-Wide Tools and Operations (Cleanup, Properties, Statistics))
- [ ] PALETTE-Item (Implement General Item Palette/Browser)
- [ ] RENDER-04-LightingSystem (Port Lighting System)
- [ ] REFACTOR-01 (Decouple UI State and Services from Global Access (Refactor Conceptual GuiManager))
- [ ] UI-05 (Port Brush & Material Editor)
- [ ] UI-06 (Port Creature Palette and Placed Creature Editor Dialog)
- [ ] UI-07 (Port House Palette, Waypoint Palette, and EditHouseDialog)
- [ ] UI-10 (Define RAW Items Palette and Terrain Brushes Palette)
- [ ] UI-11 (Port Item Finder Dialog) -
- [ ] UI-MinimapView (Implement Minimap View Widget)
- [ ] FINAL-02 (Implement File Menu Operations (New, Open, Save, Save As, Close))
- [ ] FINAL-03 (Implement Edit Menu Operations & Link Preferences Dialog)
- [ ] UI-DIALOGS-LIVE-CONNECT (Implement Live Collaboration Connection Dialog)
- [ ] FINAL-04 (Implement View Menu & Common Tools Menu Operations)
- [ ] UI-08 (Port Spawn Creation Settings and Spawn Properties Editor)
- [ ] UI-09 (Port Live Server Control Panel)
- [ ] FINAL-05 (Implement Live Collaboration Client Functionality)
- [ ] FINAL-06 (Implement About Dialog & Welcome Screen)
- [ ] UI-04 (Port Item, Creature, and Spawn Properties Dialogs)
- [ ] FINAL-07 (Apply Qlementine Styling to Application)
- [ ] REFACTOR-02 (Define and Execute Performance & Memory Profiling Strategy for Qt Application)
- [ ] REFACTOR-03 (Implement Rendering Optimizations Based on Profiling Report)

### In Progress
- None

### In Review
- [ ] CORE-14-MaterialSystem (Port Material System Data Structures) - Branch: feature/CORE-14-MaterialSystem-partial
- [ ] CORE-11-WaypointSystem (Port Waypoint System Data Structures) - Branch: feature/CORE-11-WaypointSystem-partial
- [ ] CORE-08-CreatureOutfit (Port Creature Instance and Outfit Classes) - Branch: feature/CORE-08-CreatureOutfit
- [ ] CORE-07-MapIO (Port OTBM Map I/O System) - Branch: feature/CORE-07-MapIO-reimplementation-completed
- [ ] CORE-04 (Port Action & History (Undo/Redo) System)

### Completed
- [x] BUILD-02 (Implement Packaging and Deployment) - Branch: feat/build-02-cpack-packaging
- [x] UI-01 (Port Main Application Window (QMainWindow) and Dynamic Menu Bar from XML) - Branch: feat/ui-01-mainwindow-reworked
- [x] RENDER-01 (Implement Core OpenGL Viewport and Navigation (MapView)) - Branch: feat/render-01-mapview-complete
- [x] LOGIC-04 (Port Waypoint System) - Branch: feat/logic-04-waypoint-system
- [x] CORE-BRUSH-FRAMEWORK (Port Base Brush Class, Implement BrushManager, and Define BrushSettings) - Branch: feat/core-brush-framework
- [x] BUILD-00 (Create Root CMakeLists.txt and Basic Project Structure) - Branch: feat/build-00-initial-structure
- [x] CORE-01 (Port Core Data Structures - Position, Item, Tile) - Branch: feat/core-01-data-structures
- [x] START (Project Start and Overall WBS Definition) - Branch: docs/start-task-acknowledged
- [x] BUILD-01 (Setup Initial CMake Build System for Qt6 Project) - Branch: feat/build-01-cmake-setup
- [x] CORE-02 (Port Asset Database & Parsers (Items, Creatures, Sprites, Client Versions)) - Branch: feat/core-02-asset-pipeline
- [x] CORE-03 (Port Map Data Structure & Non-Rendering Logic) - Branch: feat/core-03-map-data-structure
- [x] CORE-06 (Port Settings & Preferences System) - Branch: feat/core-06-settings-system-complete
- [x] CORE-04 (Port Action & History (Undo/Redo) System)

### Blocked / Needs Discussion
- [ ] NET-01 (Isolate and Port Network Protocol Definition and Serialization Logic) - Branch: feat/net-01-protocol-serialization #WAS MADE BEFORE CORE-07, COMPLETLY FUCKED UP /IO SYSTEM. TO BE REWORKED
- [ ] NET-02 (Port Live Collaboration Server Logic to Qt Network) ##WAS MADE BEFORE CORE-07, COMPLETLY FUCKED UP /IO SYSTEM. TO BE REWORKED
- [ ] NET-02 (Port Live Collaboration Server Logic to Qt Network)
- [ ] TEST-01 (Develop Unit Tests for Core Qt6 Data Structures)
- [ ] TEST-02 (Develop Unit Tests for Asset Loading and Parsing Logic)
- [ ] TEST-03 (Develop Unit Tests for OTBM and OTMM File I/O)
- [ ] TEST-06 (Develop Unit Tests for Network Protocol Message Handling)
- [ ] DOCS-01 (Generate Developer API Documentation (Doxygen))
- [ ] TEST-04 (Develop Unit Tests for Brush Application and Material System Logic)
- [ ] TEST-05 (Develop Unit Tests for Map Actions and Selection Logic)
- [ ] DOCS-02 (Create User Manual and Feature Documentation)
- [ ] TEST-07 (Develop Integration Tests for Map Rendering System)
- [ ] TEST-08 (Develop Integration Tests for Live Collaboration Server and Client)
- [ ] TEST-09 (Develop UI Tests for Key Editor Features and Workflows)
- [ ] TEST-10 (Execute and Document Cross-Platform UI Compatibility Tests)

---
The "Tasks with Unresolved Dependencies" section has been removed from the Kanban board part as the `LOGIC-01` dependency on `BRUSH-LOGIC-ALL` was resolved by removing it, and the new task list in `tasks_execution_order.md` (which this Kanban is based on) reflects this. The specific brush logic tasks are now dependencies of `LOGIC-01`.
