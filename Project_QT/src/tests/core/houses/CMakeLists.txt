# CMakeLists.txt for core house system tests
find_package(Qt6 REQUIRED COMPONENTS Test Core)

add_executable(TestCoreHouses_House TestHouse.cpp)
target_link_libraries(TestCoreHouses_House PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib # For House, Map, Tile, Position, AssetManager etc.
)
set_target_properties(TestCoreHouses_House PROPERTIES AUTOMOC ON)
add_test(NAME TestCoreHouses_House COMMAND TestCoreHouses_House)

add_executable(TestCoreHouses_HousesManager TestHouses.cpp)
target_link_libraries(TestCoreHouses_HousesManager PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib # For Houses, House, Map, Tile, Position, AssetManager etc.
)
set_target_properties(TestCoreHouses_HousesManager PROPERTIES AUTOMOC ON)
add_test(NAME TestCoreHouses_HousesManager COMMAND TestCoreHouses_HousesManager)
