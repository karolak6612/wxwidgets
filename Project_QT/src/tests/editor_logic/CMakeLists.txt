# CMakeLists.txt for editor_logic unit tests

find_package(Qt6 REQUIRED COMPONENTS Test Core)

# MockBrush.h is included by TestEditorController.cpp
# It does not need to be listed in add_executable unless it has Q_OBJECT macros or its own .cpp.
# If MockBrush.h were to have Q_OBJECT, then TestEditorController.cpp might need to include MockBrush.moc,
# or MockBrush.h would need to be processed by AUTOMOC if added to a library or executable's sources.
# Since it's a test utility header, direct inclusion is fine and AUTOMOC on the test executable handles its Q_OBJECTs.
add_executable(TestEditorLogic_EditorController TestEditorController.cpp MockBrush.h)


target_link_libraries(TestEditorLogic_EditorController PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib         # For Map, SelectionManager, BrushManagerService, etc.
    rme_editor_logic_lib # For EditorController, Commands
)

# Enable automatic MOC, UIC, RCC handling for Qt
set_target_properties(TestEditorLogic_EditorController PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorLogic_EditorController COMMAND TestEditorLogic_EditorController)

# Test for SetHouseExitCommand
add_executable(TestEditorCommands_SetHouseExit commands/TestSetHouseExitCommand.cpp)

target_link_libraries(TestEditorCommands_SetHouseExit PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib        # For Map, HouseData, Tile, Position
    rme_editor_logic_lib # For SetHouseExitCommand itself
)

set_target_properties(TestEditorCommands_SetHouseExit PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorCommands_SetHouseExit COMMAND TestEditorCommands_SetHouseExit)

# Test for RecordSetSpawnCommand
add_executable(TestEditorCommands_RecordSetSpawn
    commands/TestRecordSetSpawnCommand.cpp
    ../core/brush/MockEditorController.h  # Corrected path
    ../mocks/MockMap.h                    # Path remains correct
)

target_link_libraries(TestEditorCommands_RecordSetSpawn PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib        # For Map, Tile, Position
    rme_editor_logic_lib # For RecordSetSpawnCommand, EditorController
)

set_target_properties(TestEditorCommands_RecordSetSpawn PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorCommands_RecordSetSpawn COMMAND TestEditorCommands_RecordSetSpawn)

# Test for RecordSetGroundCommand
add_executable(TestEditorCommands_RecordSetGround
    commands/TestRecordSetGroundCommand.cpp
    ../core/brush/MockEditorController.h  # Common mock, might contain Q_OBJECT
)

target_link_libraries(TestEditorCommands_RecordSetGround PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib        # For Tile, Item, Map, AssetManager
    rme_editor_logic_lib # For RecordSetGroundCommand, EditorControllerInterface
)

set_target_properties(TestEditorCommands_RecordSetGround PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorCommands_RecordSetGround COMMAND TestEditorCommands_RecordSetGround)

# Test for RecordAddRemoveItemCommand
add_executable(TestEditorCommands_RecordAddRemoveItem
    commands/TestRecordAddRemoveItemCommand.cpp
    ../core/brush/MockEditorController.h  # Common mock
)

target_link_libraries(TestEditorCommands_RecordAddRemoveItem PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib        # For Tile, Item, Map, AssetManager
    rme_editor_logic_lib # For RecordAddRemoveItemCommand, EditorControllerInterface
)

set_target_properties(TestEditorCommands_RecordAddRemoveItem PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorCommands_RecordAddRemoveItem COMMAND TestEditorCommands_RecordAddRemoveItem)

# Test for RecordModifyTileContentsCommand
add_executable(TestEditorCommands_RecordModifyTileContents
    commands/TestRecordModifyTileContentsCommand.cpp
    ../core/brush/MockEditorController.h  # Common mock
)

target_link_libraries(TestEditorCommands_RecordModifyTileContents PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib        # For Tile, Item, Map, AssetManager, Spawn, Creature
    rme_editor_logic_lib # For RecordModifyTileContentsCommand, EditorControllerInterface
)

set_target_properties(TestEditorCommands_RecordModifyTileContents PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorCommands_RecordModifyTileContents COMMAND TestEditorCommands_RecordModifyTileContents)

# Test for DeleteSelectionCommand
add_executable(TestEditorCommands_DeleteSelection
    commands/TestDeleteSelectionCommand.cpp
    # No specific mock header needed here if TestDeleteSelectionCommand.cpp includes what it needs
    # and those mocks don't require AUTOMOC processing themselves.
    # If MockEditorController.h is used and has Q_OBJECT, it could be added like:
    # ../core/brush/MockEditorController.h
)

target_link_libraries(TestEditorCommands_DeleteSelection PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib        # For Map, Tile, Item, etc.
    rme_editor_logic_lib # For DeleteSelectionCommand, EditorControllerInterface
)

set_target_properties(TestEditorCommands_DeleteSelection PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorCommands_DeleteSelection COMMAND TestEditorCommands_DeleteSelection)

# Test for ClearSelectionCommand
add_executable(TestEditorCommands_ClearSelection
    commands/TestClearSelectionCommand.cpp
    # ../core/brush/MockEditorController.h # If its Q_OBJECTs are needed through this test
)

target_link_libraries(TestEditorCommands_ClearSelection PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib        # For Map, Tile, etc.
    rme_editor_logic_lib # For ClearSelectionCommand, SelectionManager (via interface)
)

set_target_properties(TestEditorCommands_ClearSelection PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorCommands_ClearSelection COMMAND TestEditorCommands_ClearSelection)

# Test for BoundingBoxSelectCommand
add_executable(TestEditorCommands_BoundingBoxSelect
    commands/TestBoundingBoxSelectCommand.cpp
    # ../core/brush/MockEditorController.h # If its Q_OBJECTs are needed
)

target_link_libraries(TestEditorCommands_BoundingBoxSelect PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib        # For Map, Tile, etc.
    rme_editor_logic_lib # For BoundingBoxSelectCommand, SelectionManager (via interface)
)

set_target_properties(TestEditorCommands_BoundingBoxSelect PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorCommands_BoundingBoxSelect COMMAND TestEditorCommands_BoundingBoxSelect)

# Test for DeleteCommand (LOGIC-03 version that uses TileData)
add_executable(TestEditorCommands_DeleteLogic03
    commands/TestDeleteCommand.cpp
    # ../core/brush/MockEditorController.h # If its Q_OBJECTs are needed, and test uses it
)

target_link_libraries(TestEditorCommands_DeleteLogic03 PRIVATE
    Qt6::Test
    Qt6::Core
    rme_core_lib        # For TileData, Map, Tile, Item etc.
    rme_editor_logic_lib # For DeleteCommand itself, EditorControllerInterface
)

set_target_properties(TestEditorCommands_DeleteLogic03 PROPERTIES AUTOMOC ON)

add_test(NAME TestEditorCommands_DeleteLogic03 COMMAND TestEditorCommands_DeleteLogic03)
