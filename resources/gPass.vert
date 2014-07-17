#version 330

layout (location = 0) in vec3 ciPosition;
layout (location = 1) in vec3 ciNormal;
layout (location = 2) in vec2 ciTexCoord;

uniform mat4 ciModelViewProjectionMatrix;
uniform mat4 ciModelViewMatrix;
uniform mat3 ciNormalMatrix;
uniform mat4 ciModelMatrix;
uniform mat4 ciInverseViewMatrix;

uniform float uSpecular;

uniform sampler2D diffuseMap;

out vec4 wcPosition;
out vec3 wcNormal;
out vec3 diffuse;
out float specular;

void main(){
    
    float id = gl_InstanceID;
    float gridPosx = mod( id ,9.);
    float gridPosy = (id + gridPosx) / 8.;
    
    vec3 pos = ciPosition;
    
    pos.x += gridPosx*10.;
    pos.z -= gridPosy*10.;
    
    wcPosition = ciModelMatrix * vec4(pos, 1.0);
    
    vec3 norm = ciNormal;
    wcNormal = ciNormalMatrix * norm;
    wcNormal = mat3(ciInverseViewMatrix) * wcNormal;
    
    diffuse = texture( diffuseMap, ciTexCoord ).rgb;
    specular = uSpecular;
    
    gl_Position = ciModelViewProjectionMatrix * vec4(pos, 1.0);
}