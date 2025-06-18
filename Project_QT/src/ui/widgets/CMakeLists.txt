# CMakeLists.txt for UI widgets

set(RME_UI_WIDGETS_SOURCES
    MapView.cpp
    MapView.h
    SpawnSettingsWidget.cpp
    SpawnSettingsWidget.h
)

# Headers are typically handled by target_include_directories in the parent CMakeLists
# that creates the library/target using these sources.
# If we were creating a library here directly:
# add_library(rme_mapview_widget STATIC ${RME_UI_WIDGETS_SOURCES})
# target_include_directories(rme_mapview_widget PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_link_libraries(rme_mapview_widget PRIVATE Qt6::OpenGL Qt6::Widgets rme_core_lib)
