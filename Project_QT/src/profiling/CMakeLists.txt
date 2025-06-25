# Profiling Infrastructure for REFACTOR-02
# Visual Studio Profiling with CMake - No Benchmarks

# Profiling target for Visual Studio Performance Profiler
add_executable(rme_profiling_target
    ProfilingMain.cpp
    ProfilingUtils.cpp
    ProfilingUtils.h
)

# Link against core libraries and Qt6
target_link_libraries(rme_profiling_target
    PRIVATE
    rme_core_lib
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::Concurrent
    oclero::qlementine
)

# Include directories
target_include_directories(rme_profiling_target
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Visual Studio specific profiling settings
if(MSVC)
    # Enable debug information for profiling
    target_compile_options(rme_profiling_target PRIVATE 
        /Zi          # Generate debug information
        /Od          # Disable optimizations for accurate profiling
        /RTC1        # Runtime checks
    )
    
    # Linker settings for profiling
    set_target_properties(rme_profiling_target PROPERTIES
        LINK_FLAGS "/DEBUG /PROFILE /SUBSYSTEM:CONSOLE"
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    
    # Add profiling-specific preprocessor definitions
    target_compile_definitions(rme_profiling_target PRIVATE
        RME_PROFILING_BUILD=1
        RME_ENABLE_PERFORMANCE_COUNTERS=1
        _CRTDBG_MAP_ALLOC  # Enable CRT debug heap
    )
    
    # Set Visual Studio startup project properties
    set_target_properties(rme_profiling_target PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "--profile-scenarios"
        VS_DEBUGGER_ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin;$ENV{PATH}"
    )
endif()

# Install profiling target
install(TARGETS rme_profiling_target
    RUNTIME DESTINATION bin/profiling
)

# Create profiling scripts directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

# Generate Visual Studio profiling batch script
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_vs_profiling.bat.in
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_vs_profiling.bat
    @ONLY
)

# Copy profiling scripts and configuration
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_vs_profiling.bat
    ${CMAKE_CURRENT_SOURCE_DIR}/profiling_scenarios.txt
    DESTINATION bin/profiling/scripts
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)