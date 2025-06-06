import yaml

# Custom representer for multiline strings
def str_presenter(dumper, data):
    if len(data.splitlines()) > 1:  # check for multiline
        return dumper.represent_scalar('tag:yaml.org,2002:str', data, style='|')
    return dumper.represent_scalar('tag:yaml.org,2002:str', data)

yaml.add_representer(str, str_presenter)

yaml_content = {
    "id": "FINAL-02",
    "section": "Core Migration Tasks",
    "title": "Create Final CMake Build System",
    "original_input_files": "(all files)",
    "analyzed_input_files": [
        "`mapcore` library structure (from CORE-01+)",
        "Qt application source structure (conceptual)",
        "All previously ported modules"
    ],
    "dependencies": "ALL",  # Representing a final aggregation task
    "current_functionality_summary": """\
No existing top-level CMake build system is present. This task is to create the primary `CMakeLists.txt` file that will orchestrate the build of the entire Qt6-based application, including the `mapcore` static library and the main executable linking against Qt6 and other dependencies.\
""",
    "qt6_migration_steps": """\
1. Create a root `CMakeLists.txt` file.
2. Set `cmake_minimum_required(VERSION 3.16)`.
3. Define the project: `project(RME_Qt LANGUAGES CXX)`.
4. Set C++ standard: `set(CMAKE_CXX_STANDARD 17)` and `set(CMAKE_CXX_STANDARD_REQUIRED ON)`.
5. Enable Qt's automatic meta-object tools: `set(CMAKE_AUTOMOC ON)`, `set(CMAKE_AUTORCC ON)`, `set(CMAKE_AUTOUIC ON)`.
6. Use `find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui OpenGL)` to locate Qt6. Handle case where Qt6 might not be found.
7. Use `find_package(ZLIB REQUIRED)` if zlib is used directly (not just via Qt's compression).
8. Add the `mapcore` library:
   - If `mapcore` has its own `CMakeLists.txt` (preferred, from `CORE-01`), use `add_subdirectory(mapcore)`.
   - Otherwise, define the `mapcore` static library target here using `add_library(mapcore STATIC ...)` and list all its source files. (The WBS implies `mapcore` will have its own CMake setup from CORE-01).
9. Define the main application executable target: `add_executable(RME-Qt ...)` listing all its Qt UI source files (e.g., `main.cpp`, `mainwindow.cpp`, etc. - these will be created in UI tasks). For initial setup, placeholder sources can be used.
10. Set include directories for `RME-Qt`: `target_include_directories(RME-Qt PUBLIC ${Qt6::Core_INCLUDE_DIRS} ${Qt6::Widgets_INCLUDE_DIRS} ${Qt6::Gui_INCLUDE_DIRS} ${Qt6::OpenGL_INCLUDE_DIRS} path/to/mapcore/headers ${CMAKE_CURRENT_BINARY_DIR})`.
11. Link `RME-Qt` against necessary libraries: `target_link_libraries(RME-Qt PRIVATE mapcore Qt6::Core Qt6::Widgets Qt6::Gui Qt6::OpenGL ZLIB::zlib)`. (Adjust ZLIB linking based on `find_package` result, e.g. `${ZLIB_LIBRARIES}` if not an imported target).
12. Ensure the build system can generate project files (e.g., Makefiles, Ninja files) and compile both `mapcore` and the `RME-Qt` executable.\
""",
    "definition_of_done": """\
A root `CMakeLists.txt` file is created that correctly configures and builds the `mapcore` static library and the final `RME-Qt` executable.
Key requirements:
- CMake minimum version is set to 3.16. C++17 is enabled.
- Qt6 (Core, Widgets, Gui, OpenGL components) and ZLIB (if direct) are found using `find_package`.
- The `mapcore` static library is correctly defined and built (preferably via `add_subdirectory`).
- The `RME-Qt` executable target is defined, including its source files (can be placeholders initially).
- `RME-Qt` correctly links against `mapcore`, Qt6 components, and other dependencies.
- Qt's AUTOMOC, AUTORCC, AUTOUIC are enabled.
- The entire project (library and executable) compiles successfully.\
""",
    "boilerplate_coder_ai_prompt": """\
Write a top-level `CMakeLists.txt` for the RME-Qt project.
1. Set `cmake_minimum_required(VERSION 3.16)`.
2. Define `project(RME_Qt LANGUAGES CXX)`.
3. Set `CMAKE_CXX_STANDARD 17` and `CMAKE_CXX_STANDARD_REQUIRED ON`.
4. Enable `CMAKE_AUTOMOC ON`, `CMAKE_AUTORCC ON`, `CMAKE_AUTOUIC ON`.
5. Use `find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui OpenGL)`.
6. (Optional, if used directly by application code, not just by mapcore) `find_package(ZLIB REQUIRED)`.
7. Assume `mapcore` is a subdirectory (e.g., `./mapcore/`) with its own `CMakeLists.txt` defining a static library target named `mapcore`. Add it using `add_subdirectory(mapcore)`.
8. Define an executable target `RME-Qt`. For now, use placeholder source files like `main.cpp` and `mainwindow.cpp` for this target (e.g. `add_executable(RME-Qt main.cpp mainwindow.cpp)`). These files will be created and populated by later UI-related tasks.
9. Use `target_include_directories(RME-Qt PUBLIC ...)` to include headers from Qt6 components (e.g., `${Qt6::Widgets_INCLUDE_DIRS}`), public headers from `mapcore` (e.g., `${CMAKE_SOURCE_DIR}/mapcore/include` or similar, depending on `mapcore`'s structure), and `${CMAKE_CURRENT_BINARY_DIR}` for MOC generated files.
10. Use `target_link_libraries(RME-Qt PRIVATE mapcore Qt6::Core Qt6::Widgets Qt6::Gui Qt6::OpenGL)`. If ZLIB was found and is needed directly by the application, add `ZLIB::zlib` (or `${ZLIB_LIBRARIES}`).
11. Ensure the script can generate a build system (e.g., Makefiles or Ninja project files) without errors.\
"""
}

output_file_path = "enhanced_wbs_yaml_files/FINAL-02.yaml"

with open(output_file_path, 'w') as f:
    yaml.dump(yaml_content, f, sort_keys=False, width=1000)

print(f"Generated {output_file_path}")
