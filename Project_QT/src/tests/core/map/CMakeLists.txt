# CMakeLists.txt for core map structure unit tests

find_package(Qt6 REQUIRED COMPONENTS Test Core) # Core for QCoreApplication, QDir, QFile etc. in tests

# Test executables
add_executable(TestCoreMap_QuadTree TestQuadTree.cpp)
target_link_libraries(TestCoreMap_QuadTree PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreMap_QuadTree COMMAND TestCoreMap_QuadTree)

add_executable(TestCoreMap_BaseMap TestBaseMap.cpp)
target_link_libraries(TestCoreMap_BaseMap PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreMap_BaseMap COMMAND TestCoreMap_BaseMap)

add_executable(TestCoreMap_MapIterator TestMapIterator.cpp)
target_link_libraries(TestCoreMap_MapIterator PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreMap_MapIterator COMMAND TestCoreMap_MapIterator)

add_executable(TestCoreMap_Map TestMap.cpp)
target_link_libraries(TestCoreMap_Map PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreMap_Map COMMAND TestCoreMap_Map)

add_executable(TestCoreMap_WaypointData TestWaypointData.cpp)
target_link_libraries(TestCoreMap_WaypointData PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreMap_WaypointData COMMAND TestCoreMap_WaypointData)

# Note: The sample data files are created programmatically within each test's initTestCase().
# This makes tests self-contained for this phase.
# For CI/CD or more complex data, copying test data via CMake install rules
# or setting WORKING_DIRECTORY for tests would be preferred.

# Test executable for Map spawn functionality
add_executable(TestCoreMap_Spawns TestMapSpawns.cpp)

target_link_libraries(TestCoreMap_Spawns PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib # Link against the main core library
)

# Enable automatic MOC, UIC, RCC handling for Qt
# Required because TestMapSpawns uses Q_OBJECT
set_target_properties(TestCoreMap_Spawns PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreMap_Spawns COMMAND TestCoreMap_Spawns)

# Test executable for Map town functionality
add_executable(TestCoreMap_Towns TestMapTowns.cpp)

target_link_libraries(TestCoreMap_Towns PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib # Link against the main core library
)

# Enable automatic MOC, UIC, RCC handling for Qt
set_target_properties(TestCoreMap_Towns PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreMap_Towns COMMAND TestCoreMap_Towns)

# Test executable for Map query functionality
add_executable(TestCoreMap_Queries TestMapQueries.cpp)

target_link_libraries(TestCoreMap_Queries PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib # Link against the main core library
)

# Enable automatic MOC, UIC, RCC handling for Qt
# Required because TestMapQueries uses Q_OBJECT
set_target_properties(TestCoreMap_Queries PROPERTIES AUTOMOC ON)

add_test(NAME TestCoreMap_Queries COMMAND TestCoreMap_Queries)
