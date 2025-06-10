# Add subdirectories for different modules/libraries
add_subdirectory(core)
add_subdirectory(ui)

# Define application source files
# These wxwidgets sources are placeholders and will be refactored/replaced.
# Paths are relative to this CMakeLists.txt (Project_QT/src/)
set(APP_SOURCES
    main.cpp
    ../../wxwidgets/application.cpp
    ../../wxwidgets/editor.cpp
    ../../wxwidgets/map_display.cpp
)

# Define the executable
if(WIN32)
    add_executable(rme_qt_app WIN32 ${APP_SOURCES})
elseif(APPLE)
    add_executable(rme_qt_app MACOSX_BUNDLE ${APP_SOURCES})
else()
    add_executable(rme_qt_app ${APP_SOURCES})
endif()

# macOS specific bundle properties
if(APPLE)
    set_target_properties(rme_qt_app PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "${CMAKE_PROJECT_SOURCE_DIR}/packaging/icons/RME-Qt.icns" # Path relative to root project
        MACOSX_BUNDLE_BUNDLE_NAME "RME-Qt"
        # MACOSX_BUNDLE_INFO_PLIST "${CMAKE_PROJECT_SOURCE_DIR}/packaging/macos/Info.plist.in" # Optional custom Info.plist
    )
endif()

# Link against Qt6 modules, Qlementine, and our libraries
target_link_libraries(rme_qt_app PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::Concurrent
    oclero::qlementine # Qlementine library target
    rme_core_lib       # Our core library
    rme_ui_lib         # Our new UI library
)

# Include directories for the application
target_include_directories(rme_qt_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # For main.cpp etc. in Project_QT/src/
    ../../wxwidgets            # For wxwidgets headers needed by placeholder sources
    # ../../XML                 # If C++ code needs to include files relative to XML structure
)

# Source groups for IDE organization
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files/App" FILES main.cpp)
source_group(TREE ../../wxwidgets PREFIX "Source Files/wxwidgets_legacy" FILES
    ../../wxwidgets/application.cpp
    ../../wxwidgets/editor.cpp
    ../../wxwidgets/map_display.cpp
)

# Placeholder for Qt Designer .ui files and .qrc resource files
# qt_add_ui_files(UI_HEADERS_FWD UI_SOURCES_FWD forms/somedialog.ui)
# target_sources(rme_qt_app PRIVATE ${UI_SOURCES_FWD} ${UI_HEADERS_FWD})
# qt_add_resources(RESOURCE_SOURCES resources/application.qrc)
# target_sources(rme_qt_app PRIVATE ${RESOURCE_SOURCES})
