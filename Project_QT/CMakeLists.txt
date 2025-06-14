cmake_minimum_required(VERSION 3.16)
project(RME-Qt6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatically run MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Comment placeholder for CMAKE_PREFIX_PATH adjustments if needed for non-standard Qt/Qlementine installs
# list(APPEND CMAKE_PREFIX_PATH "/path/to/qt6" "/path/to/qlementine_install")

# Find Qt6 package with necessary components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL Concurrent)

# Add Qlementine library (assuming it's two levels up from this Project_QT dir)
# The second argument ensures qlementine builds in its own isolated build folder.
add_subdirectory(../../qlementine qlementine_build)

# Define the source directory (contains main app and core lib)
add_subdirectory(src)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# Output directories for binaries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Resource Handling ---
# Copy critical XML data files to the build directory for runtime access
# Assuming XML directory is one level above Project_QT (i.e., at REPO_ROOT/XML)
set(XML_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../XML)
set(XML_DEST_DIR ${CMAKE_BINARY_DIR}/data/XML)

if(EXISTS ${XML_SOURCE_DIR})
    message(STATUS "XML source directory found at: ${XML_SOURCE_DIR}")
    file(COPY_DIRECTORY ${XML_SOURCE_DIR}/ DESTINATION ${XML_DEST_DIR}/) # trailing slash matters for contents
    message(STATUS "XML files will be copied to: ${XML_DEST_DIR}")

    # Installation rules for XML data files
    # Installs REPO_ROOT/XML/* to <prefix>/data/XML/*
    install(DIRECTORY ../XML/ DESTINATION data TYPE DIRECTORY)
    message(STATUS "XML files will be installed to: <prefix>/data/XML")
else()
    message(WARNING "XML source directory NOT found at: ${XML_SOURCE_DIR} - XML files will not be copied/installed.")
endif()
# --- End Resource Handling ---

# Enable CTest testing framework
enable_testing()

# Add the main tests directory
option(BUILD_TESTING "Build the tests" ON)
if(BUILD_TESTING)
    add_subdirectory(src/tests) # Path to tests dir relative to Project_QT
endif()

# --- CPack Integration ---
# General CPack settings
set(CPACK_PACKAGE_NAME "RME-Qt")
set(CPACK_PACKAGE_VERSION_MAJOR "0") # Adjust as per project versioning
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_VENDOR "Remere's Map Editor Team") # Or appropriate name
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A modern map editor for OTBM maps using Qt6")
set(CPACK_PACKAGE_CONTACT "https://github.com/rme-mapeditor/rme-qt") # Official project URL or contact
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/COPYING") # Assuming a COPYING or LICENSE file
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
endif()
set(CPACK_PACKAGE_INSTALL_DIRECTORY "RME-Qt") # Default installation directory component

# Windows specific (WIX)
if(WIN32)
    set(CPACK_GENERATOR "WIX")
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icons/RME-Qt.ico") # Icon for installer
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icons/RME-Qt.ico") # Icon for Add/Remove Programs
    # Generate a new GUID for your project: e.g., using https://www.guidgenerator.com/
    set(CPACK_WIX_UPGRADE_GUID "E7A2A00F-0204-436A-826B-8A6E72A1DBA5") # Replace with a persistent unique GUID (Example GUID)
    set(CPACK_WIX_UI_REF "WixUI_InstallDir") # Standard UI allowing directory choice
endif()

# macOS specific (Bundle & DragNDrop for DMG)
if(APPLE)
    set(CPACK_GENERATOR "Bundle;DragNDrop")
    # MACOSX_BUNDLE properties are set on the rme_qt_app target in src/CMakeLists.txt
    # CPack should pick those up for the Bundle generator.
    # For DragNDrop (DMG):
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icons/RME-Qt.icns") # Icon for the DMG itself
endif()

# Linux specific (DEB & RPM)
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB;RPM") # Build both, or user can select one via `cpack -G DEB`
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "RME Team <contact@example.com>") # Replace with actual maintainer
    set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base-gui (>= 6.2.0), libqt6opengl6 (>= 6.2.0)") # Example
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Editors")
    set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase-gui >= 6.2.0, libqt6-qtopengl >= 6.2.0") # Example
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
        set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    endif()
endif()

# --- Installation Rules (ensure these are correctly defined for RME_Qt target and resources) ---
# The actual rme_qt_app target is defined in src/CMakeLists.txt.
# CPack needs install() rules in this root file, or it needs to be configured to find them.
# The install rules from src/CMakeLists.txt will be used by CPack if that subdirectory is processed.
# However, for global assets like icons and desktop files, rules are often here.

# Ensure the executable target from src/CMakeLists.txt (rme_qt_app) is installed.
# This rule might be redundant if src/CMakeLists.txt has its own install rule for rme_qt_app.
# If src/CMakeLists.txt has `install(TARGETS rme_qt_app ...)`, this one is not strictly needed
# unless we want to override or specify different components for packaging.
# For now, assume src/CMakeLists.txt installs its own target.
# install(TARGETS rme_qt_app
#     RUNTIME DESTINATION bin
#     BUNDLE DESTINATION .
# )

if(UNIX AND NOT APPLE) # Linux specific installations
    # Install Linux application icon
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icons/RME-Qt-256.png"
        DESTINATION share/pixmaps RENAME "${CPACK_PACKAGE_NAME}.png")

    # Install .desktop file
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/RME-Qt.desktop"
        DESTINATION share/applications)
endif()

# --- Qt Runtime Bundling (using install(CODE) for CPack post-processing) ---
if(WIN32)
    # Qt6_BIN_DIR should be available if Qt6 was found correctly
    install(CODE "
        message(STATUS \"Running windeployqt for rme_qt_app...\")
        execute_process(
            COMMAND \"${Qt6_BIN_DIR}/windeployqt.exe\"
                --qmldir \"${CMAKE_CURRENT_SOURCE_DIR}/src\" # Path to QML sources, if any
                --verbose 1
                --dir \"${CMAKE_INSTALL_PREFIX}/bin\"
                \"${CMAKE_INSTALL_PREFIX}/bin/rme_qt_app.exe\"
            RESULT_VARIABLE _windeployqt_result
        )
        if(NOT _windeployqt_result EQUAL 0)
            message(FATAL_ERROR \"windeployqt failed with exit code: \${_windeployqt_result}\")
        endif()
    ")
elseif(APPLE)
    # Qt6_BIN_DIR should point to Qt's bin directory
    install(CODE "
        message(STATUS \"Running macdeployqt for RME-Qt.app...\")
        execute_process(
            COMMAND \"${Qt6_BIN_DIR}/macdeployqt\"
                \"${CMAKE_INSTALL_PREFIX}/RME-Qt.app\" # Path to RME-Qt.app in CPack staging area
                # -dmg # Optional: creates a DMG automatically by macdeployqt
            RESULT_VARIABLE _macdeployqt_result
        )
        if(NOT _macdeployqt_result EQUAL 0)
            message(FATAL_ERROR \"macdeployqt failed with exit code: \${_macdeployqt_result}\")
        endif()
    ")
else() # Linux
    message(STATUS \"Linux: Relying on RPATH settings for Qt dependencies.\")
endif()

# This line must be at the very end of the root CMakeLists.txt.
include(CPack)
