# CMakeLists.txt for the main tests directory (Project_QT/src/tests/)

# Ensure Qt Test component is found for all test subdirectories.
# This helps if subdirectories don't repeat it, though it's safe if they do.
find_package(Qt6 REQUIRED COMPONENTS Test)

# Enable CTest testing framework and make it available to subdirectories.
# This should be done before any add_test() commands are encountered in sub-CMakelists.
include(CTest)

# Add subdirectories for different test categories
add_subdirectory(core)
add_subdirectory(editor_logic) # New line

# Add other test categories later, e.g.:
# add_subdirectory(ui)
