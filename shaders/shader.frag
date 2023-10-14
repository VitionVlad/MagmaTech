#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2DArray texSampler;

layout(set = 0, binding = 2) uniform sampler2D shadowMap;

layout(set = 0, binding = 3) uniform samplerCube cubemap;

layout(location = 0) in vec2 fuv;

layout(location = 1) in vec3 pos;

layout(location = 2) in vec3 normals;

layout(location = 3) in mat3 tbn;

void main() {
    outColor = vec4(texture(texSampler, vec3(fuv, 1)).rgb, 1.0);
}