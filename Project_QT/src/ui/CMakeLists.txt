# CMakeLists.txt for the UI module of RME-Qt6

# Add subdirectories if they also have CMakeLists.txt and define sources
add_subdirectory(widgets)

# Consolidate all UI sources
set(RME_UI_LIB_SOURCES
    MainWindow.cpp  # Added MainWindow source directly
    MainWindowActions.cpp
    MainToolBar.cpp
    MainToolBar.h
    dialogs/ItemFinderDialogQt.cpp
    dialogs/ItemFinderDialogQt.h
    dialogs/ItemPropertiesDialog.cpp
    dialogs/ItemPropertiesDialog.h
    dialogs/CreaturePropertiesDialog.cpp
    dialogs/CreaturePropertiesDialog.h
    dialogs/SpawnPropertiesDialog.cpp
    dialogs/SpawnPropertiesDialog.h
    dialogs/BrushMaterialEditorDialog.cpp
    dialogs/BrushMaterialEditorDialog.h
    dialogs/NewTilesetDialog.cpp
    dialogs/NewTilesetDialog.h
    dialogs/AddItemToTilesetDialog.cpp
    dialogs/AddItemToTilesetDialog.h
    palettes/BasePalettePanel.cpp
    palettes/BasePalettePanel.h
    palettes/CreaturePalettePanel.h
    palettes/DockManager.cpp
    palettes/DockManager.h
    palettes/ItemPalettePanel.cpp
    palettes/ItemPalettePanel.h
    palettes/PropertiesPanel.h
    
    # PALETTE-BrushList: Brush Palette Implementation
    palettes/BrushPalettePanel.cpp
    palettes/BrushPalettePanel.h
    palettes/BrushCategoryTab.cpp
    palettes/BrushCategoryTab.h
    palettes/BrushListWidget.cpp
    palettes/BrushListWidget.h
    palettes/BrushGridWidget.cpp
    palettes/BrushGridWidget.h
    palettes/BrushIconWidget.cpp
    palettes/BrushIconWidget.h
    palettes/BrushPreviewGenerator.cpp
    palettes/BrushPreviewGenerator.h
    palettes/BrushTooltipWidget.cpp
    palettes/BrushTooltipWidget.h
    
    # PALETTE-BrushList Phase 3: Advanced Features
    palettes/BrushFilterManager.cpp
    palettes/BrushFilterManager.h
    palettes/BrushOrganizer.cpp
    palettes/BrushOrganizer.h
    palettes/AdvancedSearchWidget.cpp
    palettes/AdvancedSearchWidget.h
    palettes/BrushContextMenu.cpp
    palettes/BrushContextMenu.h
    
    # NET-03: Live Collaboration UI Components
    dialogs/LiveConnectionDialog.cpp
    dialogs/LiveConnectionDialog.h
    widgets/LiveCollaborationPanel.cpp
    widgets/LiveCollaborationPanel.h
    
    # FINAL-03: Preferences Dialog
    dialogs/PreferencesDialog.cpp
    dialogs/PreferencesDialog.h
    
    # FINAL-06: About and Welcome Dialogs
    dialogs/AboutDialog.cpp
    dialogs/AboutDialog.h
    dialogs/WelcomeDialog.cpp
    dialogs/WelcomeDialog.h
    
    # Missing Implementation: Map Properties Dialog
    dialogs/MapPropertiesDialog.cpp
    dialogs/MapPropertiesDialog.h
    
    ${RME_UI_WIDGETS_SOURCES}
    # Add other UI component sources here if defined in this directory
    # or from other subdirectories like 'dialogs', 'palettes', etc.
)

# QRC resource file processing
set(RME_UI_QRC_FILES
    ../resources/resources.qrc
)
qt_add_resources(RME_UI_LIB_QrcSrc ${RME_UI_QRC_FILES})
list(APPEND RME_UI_LIB_SOURCES ${RME_UI_LIB_QrcSrc})

# Define the UI library
add_library(rme_ui_lib STATIC ${RME_UI_LIB_SOURCES})

# Public include directories for this library
# This allows other targets linking against rme_ui_lib to find its headers
# e.g., #include "widgets/MapView.h"
target_include_directories(rme_ui_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Exposes Project_QT/src/ui/ and its subdirectories like widgets/
)

# Link against necessary Qt modules and other internal libraries
target_link_libraries(rme_ui_lib PRIVATE
    Qt6::Core       # For QString, QPointF, QObject, signals/slots etc.
    Qt6::Gui        # For QMouseEvent, QWheelEvent, QKeyEvent, QMatrix4x4 etc.
    Qt6::OpenGL     # For QOpenGLWidget
    Qt6::Widgets    # For QWidget, and general Qt Widgets module
    Qt6::Xml        # Added Qt6::Xml for QXmlStreamReader
    Qt6::Network    # For network dialogs and widgets
    rme_core_lib    # For mapcore::Position and other core data structures
    rme_network_lib # For QtLiveClient and network components
)

# Optional: If using .ui files that need uic processing
# qt_wrap_ui(RME_UI_LIB_UISrc ${UI_FILES})
# list(APPEND RME_UI_LIB_SOURCES ${RME_UI_LIB_UISrc})

# Optional: If using QRC resource files
# qt_add_resources(RME_UI_LIB_QrcSrc ${QRC_FILES}) # This line is now handled above
# list(APPEND RME_UI_LIB_SOURCES ${RME_UI_LIB_QrcSrc})

# Ensure rme_core_lib is found (it's defined in Project_QT/src/core/CMakeLists.txt)
# This is typically handled if src/core is added as a subdirectory before src/ui
# in the parent src/CMakeLists.txt.
