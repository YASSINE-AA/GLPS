cmake_minimum_required(VERSION 3.10)

project(GLPS VERSION 1.0 LANGUAGES C)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CROSSCOMPILING 1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(PkgConfig REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
set(CMAKE_C_COMPILER "/usr/bin/x86_64-w64-mingw32-gcc")

# Set the output format for libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

    message(STATUS "Building for Win32")
    set(CMAKE_C_COMPILER "/usr/bin/x86_64-w64-mingw32-gcc-win32")
    add_definitions(-DGLPS_USE_WIN32)

    set(SOURCES
        src/glps_wgl_context.c
        src/glps_win32.c
        src/glps_window_manager.c
        src/utils/logger/pico_logger.c
    )

    set(HEADERS
        internal/glps_wgl_context.h
        include/glps_window_manager.h
        internal/glps_win32.h
        internal/glps_common.h
        internal/utils/logger/pico_logger.h
    )

    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

   set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        OUTPUT_NAME ${PROJECT_NAME}
         PREFIX ""  # Removes "lib" prefix, resulting in GLPS.dll instead of libGLPS.dll
    SUFFIX ".dll"  # Ensures the output ends with .dll
    )

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32 )
message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")

    # Ensure DLL and import library (.dll.a) are placed in the correct directory
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    )

elseif(UNIX AND NOT APPLE)
    pkg_check_modules(WAYLAND wayland-client wayland-egl egl)

    if(WAYLAND_FOUND)
        message(STATUS "Building for Linux Wayland")
        add_definitions(-DGLPS_USE_WAYLAND)

        set(SOURCES
            src/glps_wayland.c
            src/glps_window_manager.c
            src/utils/logger/pico_logger.c
            src/glps_egl_context.c
            src/xdg/wlr-data-control-unstable-v1.c
            src/xdg/xdg-decorations.c
            src/xdg/xdg-dialog.c
            src/xdg/xdg-shell.c
        )

        set(HEADERS
            internal/glps_wayland.h
            include/glps_window_manager.h
            internal/glps_egl_context.h
            internal/glps_common.h
            internal/utils/logger/pico_logger.h
            internal/xdg/wlr-data-control-unstable-v1.h
            internal/xdg/xdg-decorations.h
            internal/xdg/xdg-dialog.h
            internal/xdg/xdg-shell.h
        )

        add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

        set_target_properties(${PROJECT_NAME} PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION 1
        )

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -g3 -fsanitize=address,undefined")

        target_link_libraries(${PROJECT_NAME} PRIVATE
            m
            EGL
            wayland-client
            wayland-server
            wayland-cursor
            wayland-egl
            xkbcommon
        )
    else()
        message(STATUS "Building for Linux X11")
        add_definitions(-DGLPS_USE_X11)
    endif()
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Include Directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/internal)

if(UNIX AND NOT APPLE)
    include_directories(SYSTEM
        /usr/include/glib-2.0
        /usr/lib/glib-2.0/include
        /usr/include/pixman-1
        /usr/include/libdrm
        /usr/include/libpng12
        /usr/include
    )
endif()

# Create the Shared Library Target
# (already handled above)

# Installation Rules
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

# Enable Testing (if applicable)
enable_testing()
