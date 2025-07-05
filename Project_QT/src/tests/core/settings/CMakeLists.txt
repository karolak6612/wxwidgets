# CMakeLists.txt for core settings unit tests

find_package(Qt6 REQUIRED COMPONENTS Test Core) # Core for QSettings, QCoreApplication, etc.

# Test executable
add_executable(TestCoreSettings_AppSettings TestAppSettings.cpp)
target_link_libraries(TestCoreSettings_AppSettings PRIVATE Qt6::Test Qt6::Core rme_core_lib)
add_test(NAME TestCoreSettings_AppSettings COMMAND TestCoreSettings_AppSettings)

# Note on test data: AppSettings tests currently manage their own settings files
# programmatically via QSettings, so no explicit data file copying needed here.
