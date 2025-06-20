# UI Implementation Fix Instructions

## Overview

This document provides step-by-step instructions for fixing the identified issues in the UI implementation of the RME-Qt6 migration project. The fixes will focus on:

1. Resource path issues
2. Core data integration
3. Placeholder services
4. Integration gaps

Note: Test creation is excluded from these instructions as requested.

## Part 1: Resource Path Fixes

### 1.1. Create ResourcePathManager

Create a centralized resource path management system to handle path resolution consistently across the application.

#### Steps:
1. Create `src/core/utils/ResourcePathManager.h`
2. Create `src/core/utils/ResourcePathManager.cpp`
3. Implement methods for resolving different types of resources

### 1.2. Fix MainWindow Resource Paths

Update the resource paths in MainWindow to correctly load the menubar.xml file.

#### Steps:
1. Open `src/ui/MainWindow.cpp`
2. Locate the code that loads menubar.xml
3. Update the path to use the ResourcePathManager or Qt resource system
4. Add fallback mechanism if the resource is not found

### 1.3. Fix BrushMaterialEditorDialog Resource Paths

Update the XML file paths in BrushMaterialEditorDialog.

#### Steps:
1. Open `src/ui/dialogs/BrushMaterialEditorDialog.cpp`
2. Locate all XML file path references
3. Update paths to use the ResourcePathManager
4. Add error handling for missing files

### 1.4. Update resources.qrc

Ensure all required resources are properly included in the Qt resource system.

#### Steps:
1. Open `src/resources/resources.qrc`
2. Add missing resources:
   - menubar.xml
   - XML files for brush materials
   - Any other referenced resources

## Part 2: Core Data Integration

### 2.1. Improve MinimapViewWidget Integration

Update MinimapViewWidget to use actual map data instead of placeholders.

#### Steps:
1. Open `src/ui/widgets/MinimapViewWidget.cpp`
2. Replace placeholder map dimension values with actual map data
3. Update the `renderMinimap()` method to use real map dimensions
4. Implement proper scaling calculations based on actual map size
5. Update `widgetToMapCoords()` to use actual map coordinates

### 2.2. Complete Property Dialogs Integration

Ensure property dialogs are properly integrated with core data structures.

#### Steps:
1. Open property dialog implementations:
   - `src/ui/dialogs/ItemPropertiesDialog.cpp`
   - `src/ui/dialogs/CreaturePropertiesDialog.cpp`
   - `src/ui/dialogs/SpawnPropertiesDialog.cpp`
2. Replace placeholder data with actual data from core systems
3. Ensure proper data flow between dialogs and controllers
4. Implement validation with actual data constraints

### 2.3. Complete Brush Material Editor XML Integration

Finish XML parsing and saving for all brush types.

#### Steps:
1. Open `src/ui/dialogs/BrushMaterialEditorDialog.cpp`
2. Complete XML parsing for all brush types
3. Ensure proper saving of XML data
4. Add validation against XML schemas

## Part 3: Placeholder Services Implementation

### 3.1. Implement EditorStateService

Create a proper implementation of the EditorStateService.

#### Steps:
1. Create `src/core/editor/EditorStateService.h`
2. Create `src/core/editor/EditorStateService.cpp`
3. Implement methods based on placeholder usage in UI components
4. Connect to EditorController for state management
5. Update UI components to use the real service

### 3.2. Implement BrushStateService

Create a proper implementation of the BrushStateService.

#### Steps:
1. Create `src/core/brush/BrushStateService.h`
2. Create `src/core/brush/BrushStateService.cpp`
3. Implement methods based on placeholder usage
4. Connect to BrushManagerService
5. Update UI components to use the real service

### 3.3. Complete ItemManager Integration

Ensure all UI components use the actual ItemManager.

#### Steps:
1. Identify all UI components using placeholder item data
2. Update these components to use the actual ItemManager
3. Replace placeholder item data with real data
4. Test with large item datasets

## Part 4: Integration Gaps

### 4.1. Complete MainWindow Integration

Verify and complete the integration of MainWindow with other components.

#### Steps:
1. Open `src/ui/MainWindow.cpp` and `src/ui/MainWindow.h`
2. Verify all methods mentioned in analysis summaries are implemented
3. Test integration with all major components
4. Add missing connections if needed

### 4.2. Complete DockManager Integration

Ensure all palette panels are properly registered with DockManager.

#### Steps:
1. Open `src/ui/palettes/DockManager.cpp`
2. Ensure all palette panels are properly registered
3. Test visibility toggling for all panels
4. Verify state persistence for dock layouts

## Implementation Priority Order

1. **First Priority**: Resource Path Fixes
   - Create ResourcePathManager
   - Fix MainWindow resource paths
   - Fix BrushMaterialEditorDialog resource paths
   - Update resources.qrc

2. **Second Priority**: Core Data Integration
   - Improve MinimapViewWidget integration
   - Complete property dialogs integration
   - Complete brush material editor XML integration

3. **Third Priority**: Placeholder Services Implementation
   - Implement EditorStateService
   - Implement BrushStateService
   - Complete ItemManager integration

4. **Fourth Priority**: Integration Gaps
   - Complete MainWindow integration
   - Complete DockManager integration

## Conclusion

Following these instructions will address the key issues identified in the UI implementation status report. The focus is on fixing resource paths, integrating with core data structures, implementing placeholder services, and closing integration gaps. Once these changes are implemented, the UI layer will be fully functional and properly integrated with the rest of the application.