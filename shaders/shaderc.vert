#version 450

layout(location = 0) in vec3 positions;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

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


layout(location = 0) out vec2 fuv;

layout(location = 1) out vec3 pos;

void main() {
    vec4 vert = ubo.mscale * vec4(positions, 1.0f);
    vert = ubo.mtranslate * ubo.mrotx * ubo.mroty * ubo.mrotz * vert;
    vert = ubo.projection * ubo.rotx * ubo.roty * ubo.translate * vert;
    vert = vert.xyww;
    gl_Position = vert;
    fuv = vec2(uv.x, -uv.y);
    vec4 tof = ubo.mtranslate * ubo.mrotx * ubo.mroty * ubo.mrotz * (ubo.mscale * vec4(positions, 1.0f));
    tof.y = -tof.y;
    pos = tof.xyz;
}