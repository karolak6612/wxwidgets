# Suggested Task Execution Order

This order is based on a topological sort of the dependencies declared in the 69 YAML task files identified as existing and parsable.

## Starting Points (Tasks with no defined dependencies among the 69 files):

- BUILD-00 (Create Root CMakeLists.txt and Basic Project Structure)
- CORE-01 (Port Core Data Structures - Position, Item, Tile)
- CORE-06 (Port Settings & Preferences System)
- CORE-BRUSH-FRAMEWORK (Port Base Brush Class, Implement BrushManager, and Define BrushSettings)
- NET-01 (Isolate and Port Network Protocol Definition and Serialization Logic)
- RENDER-01 (Implement Core OpenGL Viewport and Navigation (MapView))
- UI-01 (Port Main Application Window (QMainWindow) and Dynamic Menu Bar from XML)
- UI-02 (Port Main Application Toolbars and Comprehensive Palette System)
- UI-04 (Port Item, Creature, and Spawn Properties Dialogs)
- UI-05 (Port Brush & Material Editor)
- UI-06 (Port Creature Palette and Placed Creature Editor Dialog)
- UI-07 (Port House Palette, Waypoint Palette, and EditHouseDialog)
- UI-08 (Port Spawn Creation Settings and Spawn Properties Editor)
- UI-09 (Port Live Server Control Panel)
- UI-10 (Define RAW Items Palette and Terrain Brushes Palette)
- UTIL-01-JsonReplacement (Replace json_spirit with Qt JSON for ClientVersion Settings)

## Full Suggested Execution Order (69 tasks):

1. BUILD-00 (Create Root CMakeLists.txt and Basic Project Structure)
2. CORE-01 (Port Core Data Structures - Position, Item, Tile)
3. CORE-06 (Port Settings & Preferences System)
4. CORE-BRUSH-FRAMEWORK (Port Base Brush Class, Implement BrushManager, and Define BrushSettings)
5. NET-01 (Isolate and Port Network Protocol Definition and Serialization Logic)
6. RENDER-01 (Implement Core OpenGL Viewport and Navigation (MapView))
7. UI-01 (Port Main Application Window (QMainWindow) and Dynamic Menu Bar from XML)
8. UI-02 (Port Main Application Toolbars and Comprehensive Palette System)
9. UI-04 (Port Item, Creature, and Spawn Properties Dialogs)
10. UI-05 (Port Brush & Material Editor)
11. UI-06 (Port Creature Palette and Placed Creature Editor Dialog)
12. UI-07 (Port House Palette, Waypoint Palette, and EditHouseDialog)
13. UI-08 (Port Spawn Creation Settings and Spawn Properties Editor)
14. UI-09 (Port Live Server Control Panel)
15. UI-10 (Define RAW Items Palette and Terrain Brushes Palette)
16. UTIL-01-JsonReplacement (Replace json_spirit with Qt JSON for ClientVersion Settings)
17. BUILD-01 (Setup Initial CMake Build System for Qt6 Project)
18. CORE-02 (Port Asset Database & Parsers (Items, Creatures, Sprites, Client Versions))
19. CORE-03 (Port Map Data Structure & Non-Rendering Logic)
20. RENDER-02 (Implement Basic Tile Rendering (Colored Quads with Ghosting))
21. TEST-01 (Develop Unit Tests for Core Qt6 Data Structures)
22. BUILD-02 (Implement Packaging and Deployment)
23. CORE-04 (Port Action & History (Undo/Redo) System)
24. CORE-05 (Port Selection & Copy/Paste System)
25. CORE-07-MapIO (Port OTBM Map I/O System)
26. CORE-08-CreatureOutfit (Port Creature Instance and Outfit Classes)
27. CORE-14-MaterialSystem (Port Material System Data Structures)
28. DOCS-01 (Generate Developer API Documentation (Doxygen))
29. FINAL-02 (Implement File Menu Operations (New, Open, Save, Save As, Close))
30. LOGIC-01 (Implement Core Drawing, Deletion, and Modification Logic Controller)
31. LOGIC-04 (Port Waypoint System)
32. LOGIC-07 (Port Creature & Spawn System (Data and Brushes))
33. NET-02 (Port Live Collaboration Server Logic to Qt Network)
34. RENDER-03 (Implement Sprite Rendering using Texture Atlases)
35. RENDER-04-LightingSystem (Port Lighting System)
36. TEST-02 (Develop Unit Tests for Asset Loading and Parsing Logic)
37. TEST-04 (Develop Unit Tests for Brush Application and Material System Logic)
38. TEST-06 (Develop Unit Tests for Network Protocol Message Handling)
39. CORE-09-HouseSystem (Port House System Data Structures)
40. CORE-10-SpawnSystem (Port Spawn System Data Structures)
41. CORE-11-WaypointSystem (Port Waypoint System Data Structures)
42. CORE-12-ComplexItemSystem (Port Complex Item System Data Structures)
43. CORE-15-MapRegionSystem (Port Map Region Logic)
44. FINAL-01 (Integrate Core Map Logic with Main Qt UI)
45. LOGIC-02 (Implement Bounding-Box Selection Logic)
46. LOGIC-06 (Integrate House & Waypoint Brushes)
47. NET-03 (Port Live Collaboration Client and UI Integration to Qt)
48. TEST-03 (Develop Unit Tests for OTBM and OTMM File I/O)
49. TEST-05 (Develop Unit Tests for Map Actions and Selection Logic)
50. CORE-13-TownSystem (Port Town System Data Structures)
51. FINAL-03 (Implement Edit Menu Operations & Link Preferences Dialog)
52. FINAL-04 (Implement View Menu & Common Tools Menu Operations)
53. LOGIC-03 (Implement Cut, Copy, Paste, Delete, and Drag-Move Logic)
54. LOGIC-05 (Port House System Logic & Data Management)
55. DOCS-02 (Create User Manual and Feature Documentation)
56. FINAL-05 (Implement Live Collaboration Client Functionality)
57. FINAL-06 (Implement About Dialog & Welcome Screen)
58. LOGIC-09 (Port Map-Wide Tools and Operations (Cleanup, Properties, Statistics))
59. REFACTOR-01 (Decouple UI State and Services from Global Access (Refactor Conceptual GuiManager))
60. TEST-07 (Develop Integration Tests for Map Rendering System)
61. TEST-08 (Develop Integration Tests for Live Collaboration Server and Client)
62. TEST-09 (Develop UI Tests for Key Editor Features and Workflows)
63. REFACTOR-02 (Define and Execute Performance & Memory Profiling Strategy for Qt Application)
64. FINAL-07 (Apply Qlementine Styling to Application)
65. REFACTOR-03 (Implement Rendering Optimizations Based on Profiling Report)
66. TEST-10 (Execute and Document Cross-Platform UI Compatibility Tests)

## Tasks with Broken Dependencies (pointing outside the 69 validated files):

- **FINAL-04** depends on: UI-PALETTES-ALL
- **FINAL-07** depends on: UI-PALETTES-ALL
- **LOGIC-01** depends on: BRUSH-LOGIC-ALL

## Additional Necessary Tasks (Potentially Missing YAML Files)

The following task IDs were identified as potentially missing from the `enhanced_wbs_yaml_files` directory. Their YAML files should be created or restored. Their dependencies would need to be defined within their respective files.
- BRUSH-LOGIC-Carpet
- BRUSH-LOGIC-Creature
- BRUSH-LOGIC-Doodad
- BRUSH-LOGIC-Eraser
- BRUSH-LOGIC-Ground
- BRUSH-LOGIC-House
- BRUSH-LOGIC-HouseExit
- BRUSH-LOGIC-RAW
- BRUSH-LOGIC-Spawn
- BRUSH-LOGIC-Table
- BRUSH-LOGIC-Wall
- BRUSH-LOGIC-Waypoint
- PALETTE-BrushList
- PALETTE-Creature
- PALETTE-House
- PALETTE-Item
- PALETTE-RAW
- PALETTE-Terrain
- PALETTE-Waypoints
- START
- UI-EditorWindow
- UI-MapViewWidget
- UI-MenuBar
- UI-MinimapView
- UI-StatusBar
- UI-TabBar
- UI-ToolBar

### Likely Prerequisites for Missing Tasks (General Notes):

- All `BRUSH-LOGIC-*` tasks would generally depend on `CORE-BRUSH-FRAMEWORK` and relevant `CORE-*` data tasks (e.g., `CORE-02` for item/creature data, `CORE-14-MaterialSystem` for brush definitions).
- All `PALETTE-*` tasks would generally depend on `UI-02` (which sets up the main palette system) and the respective data/logic tasks for the items they display (e.g., `PALETTE-Creature` depends on `CORE-02` and `LOGIC-07`).
- Specific `UI-*` tasks like `UI-EditorWindow`, `UI-MapViewWidget`, etc., would depend on `UI-01` (MainWindow) and the core logic tasks they interact with (e.g., `UI-MapViewWidget` depends on `RENDER-01`).
- `START.yaml` would likely be one of the very first tasks, possibly depending only on `BUILD-00` or having no dependencies if it's just a list of all other tasks.
