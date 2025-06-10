# CMakeLists.txt for all core library unit tests

# Qt6 Test is essential. Core and Gui might be needed by linked rme_core_lib
# or if tests directly use QCoreApplication, QFile, QImage etc.
# Sub-CMakeLists in assets/ and sprites/ will call find_package for their specific needs too.
find_package(Qt6 REQUIRED COMPONENTS Test Core Gui)

# Tests for original core classes (Position, Item, Tile)
# Renamed executables to avoid potential conflicts and for clarity.
add_executable(TestCore_Position TestPosition.cpp)
target_link_libraries(TestCore_Position PRIVATE Qt6::Test Qt6::Core rme_core_lib) # Qt6::Core for QObject based test
add_test(NAME TestCore_Position COMMAND TestCore_Position)

add_executable(TestCore_Item TestItem.cpp MockItemTypeProvider.h) # MockItemTypeProvider.h if it had Q_OBJECT
target_link_libraries(TestCore_Item PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCore_Item COMMAND TestCore_Item)

add_executable(TestCore_Tile TestTile.cpp MockItemTypeProvider.h) # MockItemTypeProvider.h if it had Q_OBJECT
target_link_libraries(TestCore_Tile PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCore_Tile COMMAND TestCore_Tile)


# Add subdirectories for categorized core tests
add_subdirectory(assets)
add_subdirectory(sprites)

# CTest include is typically managed at a higher level (e.g., src/tests/CMakeLists.txt).
# If include(CTest) was here, it's removed to avoid multiple inclusions if already in parent.
# add_test() commands register tests with CTest framework if it's already included.
