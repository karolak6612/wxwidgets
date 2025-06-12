# CMakeLists.txt for core derived item unit tests

find_package(Qt6 REQUIRED COMPONENTS Test Core)

set(TEST_DERIVED_ITEM_SOURCES
    TestContainerItem.cpp
    TestTeleportItem.cpp
    TestDoorItem.cpp
    TestDepotItem.cpp
    TestPodiumItem.cpp
)

foreach(TEST_SOURCE_FILE ${TEST_DERIVED_ITEM_SOURCES})
    # Extract the base name for the executable and test name
    string(REPLACE ".cpp" "" TEST_BASE_NAME ${TEST_SOURCE_FILE})
    set(EXECUTABLE_NAME "TestCoreItems_${TEST_BASE_NAME}") # e.g., TestCoreItems_TestContainerItem

    add_executable(${EXECUTABLE_NAME} ${TEST_SOURCE_FILE})

    target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        Qt6::Test
        Qt6::Core
        rme_core_lib # Link against the main core library
    )

    # Enable automatic MOC, UIC, RCC handling for Qt
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES AUTOMOC ON)

    add_test(NAME ${EXECUTABLE_NAME} COMMAND ${EXECUTABLE_NAME})
endforeach()
