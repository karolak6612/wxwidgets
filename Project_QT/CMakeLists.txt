cmake_minimum_required(VERSION 3.16)
project(RME_Qt6 VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's MOC, RCC, and UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    OpenGL
    Network
    Concurrent
)

# Apply standard Qt project setup (enables AUTOMOC, AUTORCC, AUTOUIC globally)
qt_standard_project_setup()

# Set global include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add the main source directory
add_subdirectory(src)

# Enable testing
enable_testing()

# Set installation directories
if(WIN32)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory")
endif()

# Installation rules
install(TARGETS RME_Qt6
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install Qt6 libraries on Windows
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "execute_process(COMMAND ${WINDEPLOYQT_EXECUTABLE} \${CMAKE_INSTALL_PREFIX}/bin/RME_Qt6.exe)")
    endif()
endif()