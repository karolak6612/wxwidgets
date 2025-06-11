cmake_minimum_required(VERSION 3.16)
project(RMELiveServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

# Add Project_QT as a subdirectory to make its targets (like rme_core_lib) available.
# Adjust the path if the directory structure is different.
# This assumes Project_RMELiveServer and Project_QT are sibling directories under a common root
# where this CMakeLists.txt (for RMELiveServer) is being processed from.
# If Project_QT is already built and installed, find_package(RME_CORE_LIB) might be an alternative.
# For a monorepo structure, add_subdirectory is common.
get_filename_component(CURRENT_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
add_subdirectory(${CURRENT_PROJECT_DIR}/../Project_QT Project_QT_build)

# Add include directory for RMELiveServer sources (e.g., src/)
# This allows #include "qtliveserver.h" etc. from main.cpp if they are in src/
target_include_directories(RMELiveServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
# Also need to include directories from rme_core_lib for its headers
# rme_core_lib should export its include directories via target_include_directories(rme_core_lib PUBLIC ...)
# If not, they might need to be added explicitly here, e.g.:
# target_include_directories(RMELiveServer PRIVATE ${CURRENT_PROJECT_DIR}/../Project_QT/src/core) # Example

# Define the executable
add_executable(RMELiveServer
    src/main.cpp
    src/qtliveserver.cpp # Will be created in next step
    src/qtlivepeer.cpp   # Will be created in next step
)

# Link against Qt modules and the mapcore library
target_link_libraries(RMELiveServer PRIVATE
    Qt6::Core
    Qt6::Network
    rme_core_lib # From Project_QT
)

# Install rules (optional for now, can be added later)
# install(TARGETS RMELiveServer DESTINATION bin)
