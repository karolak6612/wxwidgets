# Add subdirectories for different modules/libraries
add_subdirectory(core)
add_subdirectory(network) # NET-03: Live collaboration client
add_subdirectory(editor_logic) # New line
add_subdirectory(ui)
add_subdirectory(profiling) # REFACTOR-02 Visual Studio profiling

# Define application source files
# Qt6-only implementation - no wxWidgets dependencies
set(APP_SOURCES
    main.cpp
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
    rme_network_lib    # Our network library
    rme_editor_logic_lib # Our new editor logic library
    rme_ui_lib         # Our new UI library
)

# Include directories for the application
target_include_directories(rme_qt_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # For main.cpp etc. in Project_QT/src/
    # ../../XML                 # If C++ code needs to include files relative to XML structure
)

# Source groups for IDE organization
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files/App" FILES main.cpp)

# Qt resource files
target_sources(rme_qt_app PRIVATE
    resources/resources.qrc
)

# Install the application executable
install(TARGETS rme_qt_app
    RUNTIME DESTINATION bin    # For Windows, Linux executables
    BUNDLE DESTINATION .      # For macOS .app bundle (installs to RME-Qt.app directly in install root)
    LIBRARY DESTINATION lib    # If it were a shared library
    ARCHIVE DESTINATION lib    # If it were a static library
)
