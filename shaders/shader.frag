#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 1) in vec2 fuv;

void main() {
    outColor = vec4(texture(texSampler, fuv).rgb, 1.0);
}