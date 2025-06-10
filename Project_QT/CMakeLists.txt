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
