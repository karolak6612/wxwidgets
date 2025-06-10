# CMakeLists.txt for core sprite management unit tests

# Qt6 Gui is needed for QImage used in SpriteData/SpriteManager.
# Qt6 Core for base Qt types.
# Qt6 Test for QTest framework.
# rme_core_lib is the library being tested.
find_package(Qt6 REQUIRED COMPONENTS Core Gui Test)

add_executable(TestCoreSprites_SpriteManager TestSpriteManager.cpp)
target_link_libraries(TestCoreSprites_SpriteManager PRIVATE Qt6::Test Qt6::Core Qt6::Gui rme_core_lib)
add_test(NAME TestCoreSprites_SpriteManager COMMAND TestCoreSprites_SpriteManager)

# Note on sample data:
# Tests programmatically create sample data files in CWD. See assets/CMakeLists.txt for more details.
