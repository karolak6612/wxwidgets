# CMakeLists.txt for UI tests

# Create test for MapViewWidget
add_executable(test_mapviewwidget
    TestMapViewWidget.cpp
)

# Link against Qt Test and other required libraries
target_link_libraries(test_mapviewwidget
    PRIVATE
    Qt6::Test
    Qt6::Widgets
    Qt6::OpenGL
    rme_core_lib
    rme_ui_lib
    rme_editor_logic_lib
)

# Add the test to CTest
add_test(NAME test_mapviewwidget COMMAND test_mapviewwidget)

# Add other UI tests
add_executable(test_ui07components
    TestUI07Components.cpp
)

target_link_libraries(test_ui07components
    PRIVATE
    Qt6::Test
    Qt6::Widgets
    rme_core_lib
    rme_ui_lib
)

add_test(NAME test_ui07components COMMAND test_ui07components)

add_executable(test_ui08components
    TestUI08Components.cpp
)

target_link_libraries(test_ui08components
    PRIVATE
    Qt6::Test
    Qt6::Widgets
    rme_core_lib
    rme_ui_lib
)

add_test(NAME test_ui08components COMMAND test_ui08components)

add_executable(test_ui09components
    TestUI09Components.cpp
)

target_link_libraries(test_ui09components
    PRIVATE
    Qt6::Test
    Qt6::Widgets
    rme_core_lib
    rme_ui_lib
)

add_test(NAME test_ui09components COMMAND test_ui09components)

add_executable(test_ui10components
    TestUI10Components.cpp
)

target_link_libraries(test_ui10components
    PRIVATE
    Qt6::Test
    Qt6::Widgets
    rme_core_lib
    rme_ui_lib
)

add_test(NAME test_ui10components COMMAND test_ui10components)

add_executable(test_uieditorwindow
    TestUIEditorWindow.cpp
)

target_link_libraries(test_uieditorwindow
    PRIVATE
    Qt6::Test
    Qt6::Widgets
    rme_core_lib
    rme_ui_lib
)

add_test(NAME test_uieditorwindow COMMAND test_uieditorwindow)

# Add test for MinimapViewWidget
add_executable(test_minimapviewwidget
    TestMinimapViewWidget.cpp
)

target_link_libraries(test_minimapviewwidget
    PRIVATE
    Qt6::Test
    Qt6::Widgets
    rme_core_lib
    rme_ui_lib
)

add_test(NAME test_minimapviewwidget COMMAND test_minimapviewwidget)