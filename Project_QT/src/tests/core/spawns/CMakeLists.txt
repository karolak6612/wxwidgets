# CMakeLists.txt for core spawn system unit tests

find_package(Qt6 REQUIRED COMPONENTS Test Core)

# Test executable for SpawnData
add_executable(TestCore_SpawnData TestSpawnData.cpp)

target_link_libraries(TestCore_SpawnData PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib # Link against the main core library
)

# Enable automatic MOC, UIC, RCC handling for Qt
# Required because TestSpawnData uses Q_OBJECT
set_target_properties(TestCore_SpawnData PROPERTIES AUTOMOC ON)

add_test(NAME TestCore_SpawnData COMMAND TestCore_SpawnData)
