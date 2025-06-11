# CMakeLists.txt for Project_QT/src/core/creatures

# Add sources for the core library (or the specific component library e.g., rme_core)
# The parent CMakeLists.txt (in src/core or src) should define the library target.
# This file just lists the source files from this directory to be added to that target.

# Assuming RME_CORE_LIBRARY_TARGET_NAME is defined in a higher-level CMakeLists.txt
# (e.g., in Project_QT/src/core/CMakeLists.txt or Project_QT/src/CMakeLists.txt)
# and represents the target like "rme_core_lib"

if(NOT RME_CORE_LIBRARY_TARGET_NAME)
    message(FATAL_ERROR "RME_CORE_LIBRARY_TARGET_NAME is not defined. Please define it in a parent CMakeLists.txt before including core/creatures.")
endif()

target_sources(${RME_CORE_LIBRARY_TARGET_NAME}
    PRIVATE
        Creature.cpp
        # Outfit.h is header-only, so it does not need to be listed here for compilation.
        # It will be picked up by include directories.
)

# Public include directories for this module, if its headers should be accessible
# by other modules linking against RME_CORE_LIBRARY_TARGET_NAME.
# The parent (src/core/CMakeLists.txt) should handle setting up include paths
# so that "core/creatures/Creature.h" can be found.

# Example of how parent might do it:
# target_include_directories(${RME_CORE_LIBRARY_TARGET_NAME}
#     PUBLIC
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}} # Adds this dir to include path for users of the lib
#         $<INSTALL_INTERFACE:include/core/creatures>  # For installation
# )
# For now, this file just adds sources. Include directory management is assumed to be handled by the parent.
