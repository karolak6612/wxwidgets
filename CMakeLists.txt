# --- CPack Integration ---
# Ensure CPack is included after all install rules and main target definitions.
# General CPack settings
set(CPACK_PACKAGE_NAME "RME-Qt")
set(CPACK_PACKAGE_VERSION_MAJOR "0") # Adjust as per project versioning
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_VENDOR "Remere's Map Editor Team") # Or appropriate name
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A modern map editor for OTBM maps using Qt6")
set(CPACK_PACKAGE_CONTACT "https://github.com/rme-mapeditor/rme-qt") # Official project URL or contact

# Assuming COPYING or LICENSE file exists at CMAKE_CURRENT_SOURCE_DIR (project root)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
endif()
set(CPACK_PACKAGE_INSTALL_DIRECTORY "RME-Qt") # Default installation directory component

# Windows specific (WIX)
if(WIN32)
    set(CPACK_GENERATOR "WIX")
    # Using placeholder paths as user will create these files
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icons/RME-Qt.ico")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icons/RME-Qt.ico")
    set(CPACK_WIX_UPGRADE_GUID "PLACEHOLDER-REPLACE-THIS-WITH-A-REAL-GUID") # Placeholder GUID
    set(CPACK_WIX_UI_REF "WixUI_InstallDir")
endif()

# macOS specific (Bundle & DragNDrop for DMG)
if(APPLE)
    set(CPACK_GENERATOR "Bundle;DragNDrop")
    set(CPACK_BUNDLE_NAME "RME-Qt")
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icons/RME-Qt.icns")
endif()

# Linux specific (DEB & RPM)
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "RME Team <contact@example.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base-gui (>= 6.2.0), libqt6opengl6 (>= 6.2.0)") # Example
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Editors")
    set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase-gui >= 6.2.0, libqt6-qtopengl >= 6.2.0") # Example
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
        set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    endif()
endif()

# --- Installation Rules ---
install(TARGETS rme_qt_app
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/XML/"
    DESTINATION data/XML
    FILES_MATCHING PATTERN "*.xml" PATTERN "*.otb" PATTERN "*.otfi"
)

if(UNIX AND NOT APPLE)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icons/RME-Qt-256.png"
        DESTINATION share/pixmaps RENAME "${CPACK_PACKAGE_NAME}.png")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/RME-Qt.desktop"
        DESTINATION share/applications)
endif()

# --- Qt Runtime Bundling (using install(CODE) for CPack post-processing) ---
if(WIN32)
    install(CODE "
        if(NOT Qt6_BIN_DIR)
            message(FATAL_ERROR \"Qt6_BIN_DIR is not set. Cannot run windeployqt.\")
        endif()
        message(STATUS \"Running windeployqt for RME-Qt... Executable: $<TARGET_FILE:rme_qt_app>\")
        execute_process(
            COMMAND \"${Qt6_BIN_DIR}/windeployqt.exe\"
                --qmldir \"\${CMAKE_CURRENT_SOURCE_DIR}\"
                --verbose 1
                --dir \"\${CMAKE_INSTALL_PREFIX}/bin\"
                \"$<TARGET_FILE:rme_qt_app>\"
            RESULT_VARIABLE _windeployqt_result
        )
        if(NOT _windeployqt_result EQUAL 0)
            message(FATAL_ERROR \"windeployqt failed with exit code: \${_windeployqt_result}\")
        endif()
    ")
elseif(APPLE)
    install(CODE "
        if(NOT Qt6_BIN_DIR)
            message(FATAL_ERROR \"Qt6_BIN_DIR is not set. Cannot run macdeployqt.\")
        endif()
        message(STATUS \"Running macdeployqt for RME-Qt.app... Bundle: $<TARGET_BUNDLE_DIR:rme_qt_app>\")
        execute_process(
            COMMAND \"${Qt6_BIN_DIR}/macdeployqt\"
                \"$<TARGET_BUNDLE_DIR:rme_qt_app>\"
                -dmg
            RESULT_VARIABLE _macdeployqt_result
        )
        if(NOT _macdeployqt_result EQUAL 0)
            message(FATAL_ERROR \"macdeployqt failed with exit code: \${_macdeployqt_result}\")
        endif()
    ")
else()
    message(STATUS "Linux: Relying on RPATH settings for Qt dependencies.")
endif()

include(CPack)
