# CMakeLists.txt for Project_QT/src/core/navigation

# Add sources for the core library.
# Assumes RME_CORE_LIBRARY_TARGET_NAME is defined in a parent CMakeLists.txt
# (e.g., in Project_QT/src/core/CMakeLists.txt)

if(NOT RME_CORE_LIBRARY_TARGET_NAME)
    message(FATAL_ERROR "RME_CORE_LIBRARY_TARGET_NAME is not defined. Please define it in a parent CMakeLists.txt before including core/navigation.")
endif()

target_sources(${RME_CORE_LIBRARY_TARGET_NAME}
    PRIVATE
        WaypointData.cpp
        # WaypointData.h is header-only for its content, but .cpp is for future.
)

# Include directory management is assumed to be handled by the parent CMakeLists.txt
# (e.g. Project_QT/src/core/CMakeLists.txt making "core/navigation/" headers available)
