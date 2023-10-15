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

layout(set = 0, binding = 1) uniform sampler2DArray texSampler;

layout(set = 0, binding = 2) uniform sampler2D shadowMap;

layout(set = 0, binding = 3) uniform samplerCube cubemap;

layout(location = 0) in vec2 fuv;

layout(location = 1) in vec3 pos;

layout(location = 2) in vec3 normals;

layout(location = 3) in mat3 tbn;

layout(location = 6) in vec4 shp;

float isInShadow(){
    vec3 lightcoord = shp.xyz / shp.w;
    lightcoord.xy = (lightcoord.xy + 1.0f)/2.0f;
    float currentDepth = lightcoord.z;
    float closestDepth = texture(shadowMap, lightcoord.xy).r;  
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, lightcoord.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0; 
    return shadow;
}

vec3 point(float roughness, int i){
    vec3 norm = normalize(normals);
    vec3 lightDir = normalize(ubo.lightPos[i].xyz - pos);  
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 viewDir = normalize(vec3(-ubo.cameraPosition.x, ubo.cameraPosition.y, -ubo.cameraPosition.z) - pos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(norm, halfwayDir), 0.0), 16.0);
    vec3 specular = roughness * spec * ubo.lightColor[i].rgb;  
    return diff + specular;
}

vec3 directional(float roughness, int i){
    vec3 norm = normalize(normals);
    vec3 lightDir = normalize(ubo.lightPos[i].xyz);  
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 viewDir = normalize(vec3(-ubo.cameraPosition.x, ubo.cameraPosition.y, -ubo.cameraPosition.z) - pos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(norm, halfwayDir), 0.0), 16.0);
    vec3 specular = roughness * spec * ubo.lightColor[i].rgb;  
    return diff + specular;
}

void main() {
    vec3 color = vec3(0, 0, 0);
    float ambient = 0.2;
    vec3  albedo = texture(texSampler, vec3(fuv, 0)).rgb;
    float roughness = texture(texSampler, vec3(fuv, 1)).r;

    for(int i = 0; i < 10; i++){
        if(ubo.lightPos[i].w < 1){
            color += (point(roughness, i) * (ambient + (1.0 - isInShadow()))) * albedo * ubo.lightColor[i].rgb;
        }else{
            color += (directional(roughness, i) * (ambient + (1.0 - isInShadow()))) * albedo * ubo.lightColor[i].rgb;
        }
    }

    outColor = vec4(color, 1.0);
}