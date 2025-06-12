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
