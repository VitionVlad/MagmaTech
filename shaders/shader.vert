#version 450

layout(location = 0) in vec3 positions;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

void main() {
    gl_Position = vec4(positions, 1.0);
}