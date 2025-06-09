# Suggested Task Execution Order

This order is based on a topological sort of the dependencies declared in the 82 currently known and processed YAML task files.

## Starting Point Tasks (No prerequisites within the processed set):
- BUILD-00 (Create Root CMakeLists.txt and Basic Project Structure)
- CORE-01 (Port Core Data Structures - Position, Item, Tile)
- START (Project Start and Overall WBS Definition)

## Full Suggested Execution Order:
1. BUILD-00 (Create Root CMakeLists.txt and Basic Project Structure)
2. CORE-01 (Port Core Data Structures - Position, Item, Tile)
3. START (Project Start and Overall WBS Definition)
4. BUILD-01 (Setup Initial CMake Build System for Qt6 Project)
5. CORE-02 (Port Asset Database & Parsers (Items, Creatures, Sprites, Client Versions))
6. CORE-03 (Port Map Data Structure & Non-Rendering Logic)
7. CORE-06 (Port Settings & Preferences System)
8. CORE-BRUSH-FRAMEWORK (Port Base Brush Class, Implement BrushManager, and Define BrushSettings)
9. DOCS-01 (Generate Developer API Documentation (Doxygen))
10. LOGIC-04 (Port Waypoint System)
11. NET-01 (Isolate and Port Network Protocol Definition and Serialization Logic)
12. RENDER-01 (Implement Core OpenGL Viewport and Navigation (MapView))
13. TEST-01 (Develop Unit Tests for Core Qt6 Data Structures)
14. UI-01 (Port Main Application Window (QMainWindow) and Dynamic Menu Bar from XML)
15. BUILD-02 (Implement Packaging and Deployment)
16. CORE-04 (Port Action & History (Undo/Redo) System)
17. CORE-07-MapIO (Port OTBM Map I/O System)
18. CORE-08-CreatureOutfit (Port Creature Instance and Outfit Classes)
19. CORE-11-WaypointSystem (Port Waypoint System Data Structures)
20. CORE-14-MaterialSystem (Port Material System Data Structures)
21. DOCS-02 (Create User Manual and Feature Documentation)
22. LOGIC-01 (Implement Core Drawing, Deletion, and Modification Logic Controller)
23. NET-02 (Port Live Collaboration Server Logic to Qt Network)
24. REFACTOR-01 (Decouple UI State and Services from Global Access (Refactor Conceptual GuiManager))
25. RENDER-02 (Implement Basic Tile Rendering (Colored Quads with Ghosting))
26. TEST-02 (Develop Unit Tests for Asset Loading and Parsing Logic)
27. TEST-06 (Develop Unit Tests for Network Protocol Message Handling)
28. UI-02 (Port Main Application Toolbars and Comprehensive Palette System)
29. UTIL-01-JsonReplacement (Replace json_spirit with Qt JSON for ClientVersion Settings)
30. CORE-05 (Port Selection & Copy/Paste System)
31. CORE-09-HouseSystem (Port House System Data Structures)
32. CORE-10-SpawnSystem (Port Spawn System Data Structures)
33. CORE-12-ComplexItemSystem (Port Complex Item System Data Structures)
34. CORE-13-TownSystem (Port Town System Data Structures)
35. CORE-15-MapRegionSystem (Port Map Region Logic)
36. LOGIC-02 (Implement Bounding-Box Selection Logic)
37. NET-03 (Port Live Collaboration Client and UI Integration to Qt)
38. PALETTE-BrushList (Implement Brush List Palette)
39. REFACTOR-02 (Define and Execute Performance & Memory Profiling Strategy for Qt Application)
40. RENDER-03 (Implement Sprite Rendering using Texture Atlases)
41. TEST-03 (Develop Unit Tests for OTBM and OTMM File I/O)
42. UI-05 (Port Brush & Material Editor)
43. UI-06 (Port Creature Palette and Placed Creature Editor Dialog)
44. UI-EditorWindow (Implement Editor Window / Map Document Area)
45. UI-MapViewWidget (Implement Map View Widget (Interactive UI Shell))
46. BRUSH-LOGIC-Carpet (Port Carpet Brush Logic)
47. BRUSH-LOGIC-Creature (Port Creature Brush Logic)
48. BRUSH-LOGIC-Doodad (Port Doodad Brush Logic)
49. BRUSH-LOGIC-Eraser (Port Eraser Brush Logic)
50. BRUSH-LOGIC-Ground (Port Ground Brush Logic)
51. BRUSH-LOGIC-House (Port House Brush Logic)
52. BRUSH-LOGIC-HouseExit (Port House Exit Brush/Tool Logic)
53. BRUSH-LOGIC-RAW (Port RAW Brush Logic)
54. BRUSH-LOGIC-Spawn (Port Spawn Brush Logic)
55. BRUSH-LOGIC-Table (Port Table Brush Logic)
56. BRUSH-LOGIC-Wall (Port Wall Brush Logic)
57. BRUSH-LOGIC-Waypoint (Port Waypoint Brush/Tool Logic)
58. FINAL-01 (Integrate Core Map Logic with Main Qt UI)
59. LOGIC-03 (Implement Cut, Copy, Paste, Delete, and Drag-Move Logic)
60. LOGIC-05 (Port House System Logic & Data Management)
61. LOGIC-07 (Port Creature & Spawn System (Data and Brushes))
62. LOGIC-09 (Port Map-Wide Tools and Operations (Cleanup, Properties, Statistics))
63. PALETTE-Item (Implement General Item Palette/Browser)
64. RENDER-04-LightingSystem (Port Lighting System)
65. TEST-04 (Develop Unit Tests for Brush Application and Material System Logic)
66. UI-07 (Port House Palette, Waypoint Palette, and EditHouseDialog)
67. UI-10 (Define RAW Items Palette and Terrain Brushes Palette)
68. UI-MinimapView (Implement Minimap View Widget)
69. FINAL-02 (Implement File Menu Operations (New, Open, Save, Save As, Close))
70. REFACTOR-03 (Implement Rendering Optimizations Based on Profiling Report)
71. TEST-05 (Develop Unit Tests for Map Actions and Selection Logic)
72. TEST-07 (Develop Integration Tests for Map Rendering System)
73. UI-09 (Port Live Server Control Panel)
74. UI-DIALOGS-LIVE-CONNECT (Implement Live Collaboration Connection Dialog)
75. FINAL-03 (Implement Edit Menu Operations & Link Preferences Dialog)
76. FINAL-04 (Implement View Menu & Common Tools Menu Operations)
77. TEST-08 (Develop Integration Tests for Live Collaboration Server and Client)
78. UI-04 (Port Item, Creature, and Spawn Properties Dialogs)
79. UI-08 (Port Spawn Creation Settings and Spawn Properties Editor)
80. FINAL-05 (Implement Live Collaboration Client Functionality)
81. FINAL-06 (Implement About Dialog & Welcome Screen)
82. TEST-09 (Develop UI Tests for Key Editor Features and Workflows)
83. FINAL-07 (Apply Qlementine Styling to Application)
84. TEST-10 (Execute and Document Cross-Platform UI Compatibility Tests)


## Tasks with Unresolved Dependencies (Meta or Pointing to Missing Files):
- LOGIC-01: depends on BRUSH-LOGIC-ALL

## Additional Necessary Tasks (Potentially Missing YAML Files)
The following task components were anticipated based on project structure or naming conventions but do not have corresponding YAML files in the set of 82 processed files. They should be created and integrated:
- PALETTE-Creature
- PALETTE-House
- PALETTE-RAW
- PALETTE-Terrain
- PALETTE-Waypoints
- UI-MenuBar
- UI-StatusBar
- UI-TabBar
- UI-ToolBar

### Likely Prerequisites for These Additional Tasks (General Notes):
- All `BRUSH-LOGIC-*` tasks would generally depend on `CORE-BRUSH-FRAMEWORK` and relevant `CORE-*` data tasks.
- All `PALETTE-*` tasks would generally depend on `UI-02` (Palette System) and the data/logic tasks for the items they display.
- Specific `UI-*` component tasks would depend on `UI-01` (MainWindow) and relevant `RENDER-*` or `LOGIC-*` tasks.
- `START.yaml` (if still missing) would likely be one of the very first tasks.
