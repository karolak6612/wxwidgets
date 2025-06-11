# Suggested Task Execution Order

This order is based on a topological sort of the dependencies declared in the 82 currently known and processed YAML task files.

## Starting Point Tasks (No prerequisites within the processed set):
- BUILD-00 (Create Root CMakeLists.txt and Basic Project Structure)
- CORE-01 (Port Core Data Structures - Position, Item, Tile)
- START (Project Start and Overall WBS Definition)

## Full Suggested Execution Order:
1. BUILD-00: Create Root CMakeLists.txt and Basic Project Structure
2. CORE-01: Port Core Data Structures - Position, Item, Tile
3. START: Project Start and Overall WBS Definition
4. BUILD-01: Setup Initial CMake Build System for Qt6 Project
5. CORE-02: Port Asset Database & Parsers (Items, Creatures, Sprites, Client Versions)
6. CORE-03: Port Map Data Structure & Non-Rendering Logic
7. CORE-06: Port Settings & Preferences System
8. CORE-BRUSH-FRAMEWORK: Port Base Brush Class, Implement BrushManager, and Define BrushSettings
9. BUILD-02: Implement Packaging and Deployment
10. CORE-04: Port Action & History (Undo/Redo) System
11. CORE-07-MapIO: Port OTBM Map I/O System
12. CORE-08-CreatureOutfit: Port Creature Instance and Outfit Classes
13. CORE-11-WaypointSystem: Port Waypoint System Data Structures
14. CORE-14-MaterialSystem: Port Material System Data Structures
15. LOGIC-04: Port Waypoint System
16. NET-01: Isolate and Port Network Protocol Definition and Serialization Logic
17. RENDER-01: Implement Core OpenGL Viewport and Navigation (MapView)
18. UI-01: Port Main Application Window (QMainWindow) and Dynamic Menu Bar from XML
19. UTIL-01-JsonReplacement: Replace json_spirit with Qt JSON for ClientVersion Settings
20. CORE-05: Port Selection & Copy/Paste System
21. CORE-09-HouseSystem: Port House System Data Structures
22. CORE-10-SpawnSystem: Port Spawn System Data Structures
23. CORE-12-ComplexItemSystem: Port Complex Item System Data Structures
24. CORE-13-TownSystem: Port Town System Data Structures
25. CORE-15-MapRegionSystem: Port Map Region Logic
26. NET-02: Port Live Collaboration Server Logic to Qt Network
27. RENDER-02: Implement Basic Tile Rendering (Colored Quads with Ghosting)
28. TEST-01: Develop Unit Tests for Core Qt6 Data Structures
29. TEST-02: Develop Unit Tests for Asset Loading and Parsing Logic
30. TEST-03: Develop Unit Tests for OTBM and OTMM File I/O
31. TEST-06: Develop Unit Tests for Network Protocol Message Handling
32. UI-02: Port Main Application Toolbars and Comprehensive Palette System
33. BRUSH-LOGIC-RAW: Port RAW Brush Logic
34. BRUSH-LOGIC-Eraser: Port Eraser Brush Logic
35. BRUSH-LOGIC-Spawn: Port Spawn Brush Logic
36. BRUSH-LOGIC-Waypoint: Port Waypoint Brush/Tool Logic
37. BRUSH-LOGIC-House: Port House Brush Logic
38. BRUSH-LOGIC-HouseExit: Port House Exit Brush/Tool Logic
39. BRUSH-LOGIC-Creature: Port Creature Brush Logic
40. BRUSH-LOGIC-Ground: Port Ground Brush Logic
41. BRUSH-LOGIC-Carpet: Port Carpet Brush Logic
42. BRUSH-LOGIC-Doodad: Port Doodad Brush Logic
43. BRUSH-LOGIC-Table: Port Table Brush Logic
44. BRUSH-LOGIC-Wall: Port Wall Brush Logic
45. LOGIC-01: Implement Core Drawing, Deletion, and Modification Logic Controller
46. LOGIC-02: Implement Bounding-Box Selection Logic
47. LOGIC-05: Port House System Logic & Data Management
48. LOGIC-07: Port Creature & Spawn System (Data and Brushes)
49. NET-03: Port Live Collaboration Client and UI Integration to Qt
50. PALETTE-BrushList: Implement Brush List Palette
51. RENDER-03: Implement Sprite Rendering using Texture Atlases
52. UI-EditorWindow: Implement Editor Window / Map Document Area
53. UI-MapViewWidget: Implement Map View Widget (Interactive UI Shell)
54. DOCS-01: Generate Developer API Documentation (Doxygen)
55. FINAL-01: Integrate Core Map Logic with Main Qt UI
56. LOGIC-03: Implement Cut, Copy, Paste, Delete, and Drag-Move Logic
57. LOGIC-09: Port Map-Wide Tools and Operations (Cleanup, Properties, Statistics)
58. PALETTE-Item: Implement General Item Palette/Browser
59. RENDER-04-LightingSystem: Port Lighting System
60. REFACTOR-01: Decouple UI State and Services from Global Access (Refactor Conceptual GuiManager)
61. UI-11: Port Item Finder Dialog
62. TEST-04: Develop Unit Tests for Brush Application and Material System Logic
63. TEST-05: Develop Unit Tests for Map Actions and Selection Logic
64. UI-05: Port Brush & Material Editor
65. UI-06: Port Creature Palette and Placed Creature Editor Dialog
66. UI-07: Port House Palette, Waypoint Palette, and EditHouseDialog
67. UI-10: Define RAW Items Palette and Terrain Brushes Palette
68. UI-MinimapView: Implement Minimap View Widget
69. DOCS-02: Create User Manual and Feature Documentation
70. FINAL-02: Implement File Menu Operations (New, Open, Save, Save As, Close)
71. FINAL-03: Implement Edit Menu Operations & Link Preferences Dialog
72. TEST-07: Develop Integration Tests for Map Rendering System
73. UI-DIALOGS-LIVE-CONNECT: Implement Live Collaboration Connection Dialog
74. FINAL-04: Implement View Menu & Common Tools Menu Operations
75. UI-08: Port Spawn Creation Settings and Spawn Properties Editor
76. UI-09: Port Live Server Control Panel
77. FINAL-05: Implement Live Collaboration Client Functionality
78. FINAL-06: Implement About Dialog & Welcome Screen
79. TEST-08: Develop Integration Tests for Live Collaboration Server and Client
80. UI-04: Port Item, Creature, and Spawn Properties Dialogs
81. FINAL-07: Apply Qlementine Styling to Application
82. TEST-09: Develop UI Tests for Key Editor Features and Workflows
83. REFACTOR-02: Define and Execute Performance & Memory Profiling Strategy for Qt Application
84. TEST-10: Execute and Document Cross-Platform UI Compatibility Tests
85. REFACTOR-03: Implement Rendering Optimizations Based on Profiling Report

## Tasks with Unresolved Dependencies (Meta or Pointing to Missing Files):
- LOGIC-01: depends on BRUSH-LOGIC-ALL

## Additional Necessary Tasks (Potentially Missing YAML Files)

The following task IDs were part of an idealized comprehensive list of components. While dedicated YAML files with these exact IDs were not found among the 82 processed files, their functionality is covered by other existing, broader WBS items:

-   **PALETTE-Creature**: Covered by `UI-06.yaml` (Port Creature Palette and Placed Creature Editor Dialog).
-   **PALETTE-House**: Covered by `UI-07.yaml` (Port House Palette, Waypoint Palette, and EditHouseDialog).
-   **PALETTE-RAW**: Covered by `UI-10.yaml` (Define RAW Items Palette and Terrain Brushes Palette).
-   **PALETTE-Terrain**: Covered by `UI-10.yaml` (Define RAW Items Palette and Terrain Brushes Palette).
-   **PALETTE-Waypoints**: Covered by `UI-07.yaml` (Port House Palette, Waypoint Palette, and EditHouseDialog).
-   **UI-MenuBar**: Functionality covered by `UI-01.yaml` (Port Main Application Window (QMainWindow) and Dynamic Menu Bar from XML).
-   **UI-StatusBar**: Functionality covered by `UI-01.yaml` (Port Main Application Window (QMainWindow) and Dynamic Menu Bar from XML).
-   **UI-TabBar**: Functionality (tabbed document interface for editors) covered by `UI-EditorWindow.yaml`.
-   **UI-ToolBar**: Functionality (main application toolbars) covered by `UI-02.yaml` (Port Main Application Toolbars and Comprehensive Palette System).

Therefore, no new YAML files need to be created for these specific IDs.

### Likely Prerequisites for These Additional Tasks (General Notes):
(This sub-section provides general guidance for any *truly* new tasks of these types if identified later)
- All `BRUSH-LOGIC-*` tasks would generally depend on `CORE-BRUSH-FRAMEWORK` and relevant `CORE-*` data tasks.
- All `PALETTE-*` tasks would generally depend on `UI-02` (Palette System) and the data/logic tasks for the items they display.
- Specific `UI-*` component tasks would depend on `UI-01` (MainWindow) and relevant `RENDER-*` or `LOGIC-*` tasks.
- `START.yaml` (which was created) would likely be one of the very first tasks.
