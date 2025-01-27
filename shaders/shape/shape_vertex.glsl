#version 330 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 col;
out vec3 color;
void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
    color = col;
};
