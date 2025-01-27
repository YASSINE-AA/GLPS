#ifndef WAYLAND_UTILS_H
#define WAYLAND_UTILS_H

#include "../glad/glad.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "../linmath.h"
#include <freetype2/freetype/freetype.h>
#include <unistd.h>

typedef struct Vertex {
    vec2 pos;
    vec3 col;
} Vertex;
void set_shader_src_file(const char *file_path, GLuint shader);

#endif