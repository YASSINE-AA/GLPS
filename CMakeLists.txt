cmake_minimum_required(VERSION 3.10)

project(GLPS VERSION 1.0 LANGUAGES C)

# Set output directories for libraries and executables
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# -------------------------------------------------------------------
# Source Files
# -------------------------------------------------------------------
set(SOURCES
    src/glps_opengl.c
    src/glps_wayland.c
    src/glps_window_manager.c
    src/glad/glad.c
    src/utils/logger/pico_logger.c
    src/utils/wayland_utils.c
    src/xdg/wlr-data-control-unstable-v1.c
    src/xdg/xdg-decorations.c
    src/xdg/xdg-dialog.c
    src/xdg/xdg-shell.c
)

# -------------------------------------------------------------------
# Header Files (for IDE integration and installation)
# -------------------------------------------------------------------
set(HEADERS
    include/glps_opengl.h
    include/glps_wayland.h
    include/glps_window_manager.h
    include/glps_common.h
    include/glps_config.h
    internal/linmath.h
    internal/glad/glad.h
    internal/KHR/khrplatform.h
    internal/utils/logger/pico_logger.h
    internal/utils/wayland_utils.h
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
    /usr/include/freetype2
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
    GL
    EGL
    wayland-client
    wayland-server
    wayland-cursor
    wayland-egl
    freetype
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
# Build Examples (Optional)
# -------------------------------------------------------------------
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_executable(3d_basic examples/3d_cube.c)
    add_executable(sine_wave examples/sine_wave.c)

    foreach(EXAMPLE 3d_basic sine_wave)
        target_link_libraries(${EXAMPLE} PRIVATE ${PROJECT_NAME} m)
        target_include_directories(${EXAMPLE}
            PUBLIC ${PROJECT_SOURCE_DIR}/include
            PRIVATE ${PROJECT_SOURCE_DIR}/internal
        )
    endforeach()
endif()

# -------------------------------------------------------------------
# Enable Testing (if applicable)
# -------------------------------------------------------------------
enable_testing()
