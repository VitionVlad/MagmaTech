#version 450

layout(location = 0) in vec3 positions;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(binding = 0) uniform UniformBufferObject {
    vec2 resolution;
    vec3 cameraPosition;
    mat4 mvp;
    mat4 mesh;
    mat4 smvp;
} ubo;

void main() {
    gl_Position = ubo.mvp * vec4(positions, 1.0);
}