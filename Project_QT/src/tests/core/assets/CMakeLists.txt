# CMakeLists.txt for core asset management unit tests

# Qt6 Core is needed for QCoreApplication, QDir, QFile, etc., used in tests.
# Qt6 Test is needed for the QTest framework.
# rme_core_lib is the library being tested.
find_package(Qt6 REQUIRED COMPONENTS Core Test)

add_executable(TestCoreAssets_ClientVersionManager TestClientVersionManager.cpp)
target_link_libraries(TestCoreAssets_ClientVersionManager PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreAssets_ClientVersionManager COMMAND TestCoreAssets_ClientVersionManager)

add_executable(TestCoreAssets_ItemDatabase TestItemDatabase.cpp)
target_link_libraries(TestCoreAssets_ItemDatabase PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreAssets_ItemDatabase COMMAND TestCoreAssets_ItemDatabase)

add_executable(TestCoreAssets_CreatureDatabase TestCreatureDatabase.cpp)
target_link_libraries(TestCoreAssets_CreatureDatabase PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreAssets_CreatureDatabase COMMAND TestCoreAssets_CreatureDatabase)

add_executable(TestCoreAssets_AssetManager TestAssetManager.cpp)
target_link_libraries(TestCoreAssets_AssetManager PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreAssets_AssetManager COMMAND TestCoreAssets_AssetManager)

add_executable(TestCoreAssets_MaterialManager TestMaterialManager.cpp)
target_link_libraries(TestCoreAssets_MaterialManager PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreAssets_MaterialManager COMMAND TestCoreAssets_MaterialManager)

# Note on sample data:
# The tests programmatically create their sample data files in the CWD.
# CTest by default runs executables from their build directory.
# This setup makes tests self-contained for this phase.
# For a production/CI setup, copying test data via CMake install rules
# or setting WORKING_DIRECTORY for tests might be preferred.
