# RME-Qt6 Visual Studio Profiling Setup
## REFACTOR-02 Implementation

This directory contains the Visual Studio profiling infrastructure for the RME-Qt6 project, specifically designed for REFACTOR-02 performance analysis.

## Overview

The profiling setup focuses exclusively on Microsoft Visual Studio Performance Profiler integration with CMake, providing:

- **Profiling Target**: Executable designed for Visual Studio profiling
- **Profiling Scenarios**: Predefined scenarios for comprehensive analysis
- **Visual Studio Integration**: Optimized build settings for profiling
- **Memory Leak Detection**: CRT debug heap integration
- **Profiling Markers**: Visual Studio-compatible profiling events

## Files Structure

```
src/profiling/
├── CMakeLists.txt                    # CMake profiling configuration
├── ProfilingMain.cpp                 # Main profiling executable
├── ProfilingUtils.h/.cpp             # Profiling utilities
├── profiling_scenarios.txt           # Scenario documentation
├── scripts/
│   └── run_vs_profiling.bat.in      # Visual Studio profiling script
└── README_VISUAL_STUDIO_PROFILING.md # This file
```

## Build Configuration

### CMake Settings for Visual Studio Profiling

The CMakeLists.txt configures the profiling target with:

```cmake
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
endif()
```

### Building the Profiling Target

```bash
# Configure with Visual Studio
cmake -B build -S Project_QT -G "Visual Studio 17 2022"

# Build the profiling target
cmake --build build --target rme_profiling_target --config Debug

# Or build all
cmake --build build --config Debug
```

## Profiling Scenarios

### Available Scenarios

1. **Application Startup** (`--startup`)
   - Service container initialization
   - Qt application startup overhead
   - Initial memory allocation patterns

2. **UI Creation** (`--ui`)
   - MainWindow instantiation
   - Widget hierarchy creation
   - Qt styling system (Qlementine) overhead

3. **Memory Stress** (`--memory`)
   - Memory allocation/deallocation patterns
   - Heap fragmentation analysis
   - Memory leak detection

4. **CPU Intensive** (`--cpu`)
   - Mathematical computation performance
   - Event processing overhead
   - Algorithm efficiency

### Running Scenarios

```bash
# Run all scenarios
rme_profiling_target.exe --profile-scenarios --duration 60

# Run specific scenario
rme_profiling_target.exe --startup
rme_profiling_target.exe --ui
rme_profiling_target.exe --memory
rme_profiling_target.exe --cpu

# Interactive mode (keeps running for manual profiling)
rme_profiling_target.exe --interactive
```

## Visual Studio Performance Profiler Setup

### Step-by-Step Instructions

1. **Open Visual Studio 2022**

2. **Access Performance Profiler**
   - Go to `Debug > Performance Profiler`
   - Or use `Alt+F2`

3. **Configure Target**
   - Select "Executable" as target type
   - Browse to: `build/bin/profiling/rme_profiling_target.exe`
   - Set Working Directory: `build/`

4. **Choose Profiling Tools**
   - **CPU Usage**: For performance bottlenecks and hot paths
   - **Memory Usage**: For memory leaks and allocation patterns
   - **.NET/C++ Heap**: For detailed heap analysis
   - **GPU Usage**: For OpenGL/rendering analysis (if applicable)

5. **Set Command Line Arguments** (optional)
   ```
   --profile-scenarios --duration 60
   ```

6. **Start Profiling**
   - Click "Start" to begin profiling session
   - The application will run with the specified scenarios
   - Visual Studio will collect profiling data

### Profiling Tools Recommendations

#### CPU Usage Profiler
- **Best for**: Identifying performance bottlenecks
- **Focus areas**: Function call trees, hot paths, CPU utilization
- **Scenarios**: All scenarios, especially `--cpu`

#### Memory Usage Profiler
- **Best for**: Memory leak detection and allocation analysis
- **Focus areas**: Heap snapshots, allocation patterns, memory growth
- **Scenarios**: `--memory`, `--startup`, `--ui`

#### .NET/C++ Heap Profiler
- **Best for**: Detailed memory analysis
- **Focus areas**: Object lifetimes, heap fragmentation, allocation stacks
- **Scenarios**: `--memory` with heap profiling enabled

## Profiling Output and Markers

### Visual Studio Profiling Markers

The profiling target outputs markers that appear in Visual Studio profiler:

```cpp
// Profiling scope markers
[VS_PROFILE_START] Application Startup
[VS_PROFILE_END] Application Startup

// Profiling events
[VS_PROFILE_EVENT] Creating Service Container
[VS_PROFILE_EVENT] Service Container Created

// Memory snapshots
[VS_MEMORY_SNAPSHOT] Before Startup
[VS_MEMORY_SNAPSHOT] After Service Container
```

### Console Output

The application provides detailed console output for correlation:

```
========================================
  RME-Qt6 Visual Studio Profiling Target
  REFACTOR-02 Implementation
  Microsoft Visual Studio + CMake
========================================

[SCENARIO] Application Startup - Creating services...
[VS_PROFILE_START] Application Startup
[VS_PROFILE_EVENT] Creating Service Container
[MEMORY] Snapshot: Before Startup:
  Working Set: 15234 KB
  Peak Working Set: 15234 KB
  Private Usage: 12456 KB
  Virtual Size: 45678 KB
```

## Memory Leak Detection

### CRT Debug Heap Integration

The profiling target enables CRT debug heap for memory leak detection:

```cpp
#ifdef _WIN32
// Enable CRT debug heap for memory leak detection
_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
```

### Memory Leak Reporting

Memory leaks are reported in:
1. **Visual Studio Output Window** (Debug tab)
2. **Console output** when the application exits
3. **Visual Studio Memory Usage profiler**

## Performance Analysis Workflow

### 1. Baseline Profiling
```bash
# Run all scenarios to establish baseline
rme_profiling_target.exe --profile-scenarios --duration 120
```

### 2. Scenario-Specific Analysis
```bash
# Focus on specific performance areas
rme_profiling_target.exe --startup    # Service initialization
rme_profiling_target.exe --ui         # UI creation overhead
rme_profiling_target.exe --memory     # Memory patterns
rme_profiling_target.exe --cpu        # CPU bottlenecks
```

### 3. Interactive Profiling
```bash
# Keep application running for manual interaction
rme_profiling_target.exe --interactive
```

### 4. Analysis in Visual Studio
1. **CPU Usage**: Identify hot functions and call trees
2. **Memory Usage**: Track allocation patterns and leaks
3. **Timeline**: Correlate events with performance spikes
4. **Call Tree**: Analyze function execution hierarchy

## Expected Profiling Results

### Performance Bottlenecks to Investigate

1. **Service Container Initialization**
   - Dependency resolution overhead
   - Service registration performance
   - Circular dependency detection

2. **Qt/UI Initialization**
   - Widget creation overhead
   - Qlementine styling system impact
   - OpenGL context initialization

3. **Memory Allocation Patterns**
   - Large object allocations
   - Frequent small allocations
   - Memory fragmentation

### Optimization Opportunities

1. **Lazy Initialization**: Defer expensive operations
2. **Object Pooling**: Reuse frequently allocated objects
3. **Caching**: Cache expensive computations
4. **Algorithm Optimization**: Improve algorithmic complexity

## Integration with REFACTOR-03

The profiling results will be used in REFACTOR-03 to:

1. **Identify Bottlenecks**: Specific functions/operations to optimize
2. **Prioritize Efforts**: Focus on high-impact optimizations
3. **Validate Improvements**: Measure optimization effectiveness
4. **Establish Baselines**: Performance benchmarks for regression testing

## Troubleshooting

### Common Issues

1. **Executable Not Found**
   ```
   ERROR: Profiling executable not found!
   ```
   **Solution**: Build the project first with `cmake --build build --target rme_profiling_target`

2. **Visual Studio Can't Find Symbols**
   **Solution**: Ensure Debug build configuration and `/Zi` compiler flag

3. **Memory Leak Detection Not Working**
   **Solution**: Verify `_CRTDBG_MAP_ALLOC` is defined and CRT debug heap is enabled

4. **Profiling Markers Not Visible**
   **Solution**: Check Visual Studio Output window (Debug tab) for marker output

### Debug Information

To verify profiling setup:

```bash
# Check if executable has debug information
dumpbin /headers build/bin/profiling/rme_profiling_target.exe | findstr "debug"

# Verify CRT debug heap
rme_profiling_target.exe --memory 2>&1 | findstr "CRTDBG"
```

## Best Practices

1. **Use Debug Builds**: For accurate profiling with symbols
2. **Disable Optimizations**: Use `/Od` for accurate line-level profiling
3. **Enable All Warnings**: Use `/Wall` to catch potential issues
4. **Profile Realistic Scenarios**: Use representative workloads
5. **Multiple Runs**: Average results across multiple profiling sessions
6. **Correlate Data**: Use profiling markers to correlate with Visual Studio data

## Next Steps

After completing profiling analysis:

1. **Document Findings**: Create detailed profiling report
2. **Prioritize Issues**: Rank performance issues by impact
3. **Plan Optimizations**: Design optimization strategies for REFACTOR-03
4. **Establish Baselines**: Record current performance metrics
5. **Validate Improvements**: Use this setup to verify optimizations