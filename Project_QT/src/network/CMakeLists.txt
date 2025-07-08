# Network library sources
set(NETWORK_SOURCES
    QtLiveClient.cpp
)

# Create network library
add_library(rme_network_lib STATIC ${NETWORK_SOURCES})

# Link dependencies
target_link_libraries(rme_network_lib PUBLIC
    Qt6::Core
    Qt6::Network
    rme_core_lib
)

# Set include directories
target_include_directories(rme_network_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set target properties
set_target_properties(rme_network_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)