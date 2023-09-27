#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2DArray texSampler;

layout(set = 0, binding = 2) uniform sampler2D shadowMap;

layout(location = 1) in vec2 fuv;

void main() {
    outColor = vec4(texture(texSampler, vec3(fuv, 1)).rgb, 1.0);
}