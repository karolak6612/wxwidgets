# Collect all source files
file(GLOB_RECURSE CORE_SOURCES
    "core/*.cpp"
)

file(GLOB_RECURSE UI_SOURCES
    "ui/*.cpp"
)

file(GLOB_RECURSE EDITOR_LOGIC_SOURCES
    "editor_logic/*.cpp"
)

file(GLOB_RECURSE NETWORK_SOURCES
    "network/*.cpp"
)

# Remove any incomplete/test files that shouldn't be compiled
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*_missing_parts\\.cpp$")
list(FILTER UI_SOURCES EXCLUDE REGEX ".*_missing_parts\\.cpp$")
list(FILTER EDITOR_LOGIC_SOURCES EXCLUDE REGEX ".*_missing_parts\\.cpp$")

# Create the main executable
add_executable(RME_Qt6
    main.cpp
    resources/resources.qrc
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${EDITOR_LOGIC_SOURCES}
    ${NETWORK_SOURCES}
)

# Link Qt6 components
target_link_libraries(RME_Qt6 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::Network
    Qt6::Concurrent
)

# Set include directories
target_include_directories(RME_Qt6 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/editor_logic
    ${CMAKE_CURRENT_SOURCE_DIR}/network
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6OpenGL_INCLUDE_DIRS}
    ${Qt6Network_INCLUDE_DIRS}
    ${Qt6Concurrent_INCLUDE_DIRS}
)

# Set target properties
set_target_properties(RME_Qt6 PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Add compiler definitions
target_compile_definitions(RME_Qt6 PRIVATE
    QT_NO_KEYWORDS  # Avoid conflicts with Qt keywords
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(RME_Qt6 PRIVATE WIN32_LEAN_AND_MEAN)
    
    # Add icon resource if available
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../app_icons/editor_icon.rc")
        target_sources(RME_Qt6 PRIVATE "../app_icons/editor_icon.rc")
    endif()
endif()

if(APPLE)
    # macOS-specific settings
    set_target_properties(RME_Qt6 PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
endif()

# Add tests subdirectory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory(tests)
endif()