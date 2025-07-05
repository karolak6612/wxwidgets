# CMakeLists.txt for the network module of RME-Qt6

# Define the network library sources
set(RME_NETWORK_LIB_SOURCES
    QtLiveClient.cpp
)

# Define the network library
add_library(rme_network_lib STATIC ${RME_NETWORK_LIB_SOURCES})

# Public include directories for this library
target_include_directories(rme_network_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Exposes Project_QT/src/network/
)

# Link against Qt6 modules and core library
target_link_libraries(rme_network_lib PRIVATE
    Qt6::Core
    Qt6::Network
    rme_core_lib    # For core data structures and network protocol
)

# Make sure the network library can find core headers
target_include_directories(rme_network_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core # For core headers
)