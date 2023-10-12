#version 450

layout(location = 0) in vec3 positions;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    int useLookAt;
    vec2 resolution;
    vec3 cameraPosition;

    mat4 projection;
    mat4 translate;
    mat4 rotx;
    mat4 roty;

    mat4 mtranslate;
    mat4 mroty;
    mat4 mrotx;
    mat4 mrotz;
    mat4 mscale;

    mat4 sprojection;
    mat4 stranslate;
    mat4 srotx;
    mat4 sroty;

    vec3 lightPos;
    vec3 lightColor;
} ubo;

layout(location = 1) out vec2 fuv;

layout(location = 2) out vec3 pos;

void main() {
    vec4 vert = ubo.mscale * vec4(positions, 1.0f);
    vert = ubo.mtranslate * ubo.mrotx * ubo.mroty * ubo.mrotz * vert;
    gl_Position = ubo.projection * ubo.rotx * ubo.roty * ubo.translate * vert;
    fuv = uv;
    pos = positions;
}