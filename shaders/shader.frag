#version 450

layout(location = 0) out vec4 outColor;

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

    vec3 lightPos[10];
    vec3 lightColor[10];
} ubo;

layout(set = 0, binding = 1) uniform sampler2DArray texSampler;

layout(set = 0, binding = 2) uniform sampler2D shadowMap;

layout(set = 0, binding = 3) uniform samplerCube cubemap;

layout(location = 0) in vec2 fuv;

layout(location = 1) in vec3 pos;

layout(location = 2) in vec3 normals;

layout(location = 3) in mat3 tbn;

void main() {
    vec3 color = vec3(0, 0, 0);
    float ambient = 0.2;
    vec3  albedo = texture(texSampler, vec3(fuv, 0)).rgb;
    float roughness = texture(texSampler, vec3(fuv, 1)).r;

    for(int i = 0; i < 10; i++){
        vec3 norm = normalize(normals);
        vec3 lightDir = normalize(ubo.lightPos[i] - pos);  

        float diff = max(dot(norm, lightDir), 0.0);

        color += (diff + ambient)*albedo*ubo.lightColor[i];
    }

    outColor = vec4(color, 1.0);
}