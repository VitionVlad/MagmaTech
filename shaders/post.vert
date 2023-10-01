#version 450

vec2 screenplane[6] = vec2[](
    vec2(-1, -1),
    vec2(-1, 1),
    vec2(1, 1),
    vec2(-1, -1),
    vec2(1, -1),
    vec2(1, 1)
);

layout(location = 1) out vec2 uv;

void main() {
    gl_Position = vec4(screenplane[gl_VertexIndex], 0, 1);
    uv = (screenplane[gl_VertexIndex]+1)/2;
}