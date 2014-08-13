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

out vec4 wcPosition;
out vec3 wcNormal;
out float specular;
out vec2 texCoord;

void main(){
    
    vec3 pos = ciPosition;
    
    //position instanced teapots without using attribute matricies
    float id = gl_InstanceID;
    float gridPosx = mod( id ,25.);
    float gridPosy = (id + gridPosx) / 25.;
    
    pos.x += gridPosx*10.;
    pos.z -= gridPosy*10.;
    
    //teapot scale
    pos *= 3.;
    
    //world space position
    wcPosition = ciModelMatrix * vec4(pos, 1.0);
    
    vec3 norm = ciNormal;
    //worldView space
    wcNormal = ciNormalMatrix * norm;
    //remove viewspace for world space
    wcNormal = mat3(ciInverseViewMatrix) * wcNormal;
    
    specular = uSpecular;
    texCoord = ciTexCoord;
    
    gl_Position = ciModelViewProjectionMatrix * vec4(pos, 1.0);
}