# Lighting System for RENDER-04

# Add lighting sources to the core library
target_sources(rme_core_lib PRIVATE
    LightingTypes.h
    LightCalculatorService.h
    LightCalculatorService.cpp
    LightRenderer.h
    LightRenderer.cpp
)

# Include directories for lighting system
target_include_directories(rme_core_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)