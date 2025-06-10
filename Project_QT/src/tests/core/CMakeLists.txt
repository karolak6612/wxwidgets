# CMakeLists.txt for all core library unit tests

# Qt6 Test is essential. Core and Gui might be needed by linked rme_core_lib
# or if tests directly use QCoreApplication, QFile, QImage etc.
# Sub-CMakeLists in assets/ and sprites/ will call find_package for their specific needs too.
find_package(Qt6 REQUIRED COMPONENTS Test Core Gui)

# Tests for original core data structures (Position, Item, Tile)
add_executable(TestCore_Position TestPosition.cpp)
target_link_libraries(TestCore_Position PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCore_Position COMMAND TestCore_Position)

# MockItemTypeProvider.h is included by TestItem.cpp and TestTile.cpp,
# it doesn't need to be listed in add_executable unless it has Q_OBJECT macros.
add_executable(TestCore_Item TestItem.cpp)
target_link_libraries(TestCore_Item PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCore_Item COMMAND TestCore_Item)

add_executable(TestCore_Tile TestTile.cpp)
target_link_libraries(TestCore_Tile PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCore_Tile COMMAND TestCore_Tile)


# Add subdirectories for categorized core tests
add_subdirectory(assets)
add_subdirectory(sprites)
add_subdirectory(map)
add_subdirectory(settings) # New line for AppSettings tests

# CTest include is managed at Project_QT/src/tests/CMakeLists.txt.
# add_test() commands register tests with CTest framework.
