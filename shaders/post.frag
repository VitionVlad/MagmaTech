#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    vec4 useLookAt;
    vec4 resolution;
    vec4 cameraPosition;

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

    vec4 lightPos[10];
    vec4 lightColor[10];
} ubo;

layout(set = 0, binding = 1) uniform sampler2D ColorMap;

layout(set = 0, binding = 2) uniform sampler2D DepthMap;

layout(location = 1) in vec2 uv;

void main() {
    outColor = vec4(texture(ColorMap, uv).rgb, 1.0);
}