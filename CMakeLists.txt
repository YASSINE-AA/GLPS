cmake_minimum_required(VERSION 3.10)

project(GLPS VERSION 1.0 LANGUAGES C)
set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND wayland-client wayland-egl egl)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Building for Win32")
    add_definitions(-DGLPS_USE_WIN32)
elseif(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND wayland-client wayland-egl egl)

    if(WAYLAND_FOUND)
        message(STATUS "Building for Linux Wayland")
        add_definitions(-DGLPS_USE_WAYLAND)
    else()
        message(STATUS "Building for Linux X11")
        add_definitions(-DGLPS_USE_X11)
    endif()
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# -------------------------------------------------------------------
# Source Files
# -------------------------------------------------------------------
set(SOURCES
    src/glps_wayland.c
    src/glps_window_manager.c
    src/utils/logger/pico_logger.c
    src/xdg/wlr-data-control-unstable-v1.c
    src/xdg/xdg-decorations.c
    src/xdg/xdg-dialog.c
    src/xdg/xdg-shell.c
)

# -------------------------------------------------------------------
# Header Files (for IDE integration and installation)
# -------------------------------------------------------------------
set(HEADERS
    include/glps_wayland.h
    include/glps_window_manager.h
    include/glps_common.h
    internal/utils/logger/pico_logger.h
    internal/xdg/wlr-data-control-unstable-v1.h
    internal/xdg/xdg-decorations.h
    internal/xdg/xdg-dialog.h
    internal/xdg/xdg-shell.h
)

# -------------------------------------------------------------------
# Include Directories
# -------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)

# Ensure linmath.h is included
include_directories(${PROJECT_SOURCE_DIR}/internal)

# Add system include directories
include_directories(SYSTEM
    /usr/include/glib-2.0
    /usr/lib/glib-2.0/include
    /usr/include/pixman-1
    /usr/include/libdrm
    /usr/include/libpng12
    /usr/include
)

# -------------------------------------------------------------------
# Compiler Flags
# -------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -g3 -fsanitize=address,undefined")

# -------------------------------------------------------------------
# Create the Shared Library Target
# -------------------------------------------------------------------
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Specify include directories for the GLPS library.
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/GLPS>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/internal
)

# Link against required libraries for GLPS.
target_link_libraries(${PROJECT_NAME} PRIVATE
    m
    EGL
    wayland-client
    wayland-server
    wayland-cursor
    wayland-egl
    xkbcommon
)

# -------------------------------------------------------------------
# Installation Rules
# -------------------------------------------------------------------
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include/GLPS/)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/internal/ DESTINATION include/GLPS/)


install(TARGETS ${PROJECT_NAME}
    EXPORT GLPSConfig
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(EXPORT GLPSConfig
    FILE GLPSConfig.cmake
    NAMESPACE GLPS::
    DESTINATION lib/cmake/GLPS
)

# -------------------------------------------------------------------
# Enable Testing (if applicable)
# -------------------------------------------------------------------
enable_testing()
