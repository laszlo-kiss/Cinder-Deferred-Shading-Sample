#version 330

layout (location = 0) in vec3 ciPosition;
layout (location = 1) in vec2 ciTexCoord;

uniform mat4 ciModelViewProjectionMatrix;

out vec2 texCoord;

void main(){

    texCoord = ciTexCoord;
    gl_Position = ciModelViewProjectionMatrix * vec4( ciPosition, 1.0);
    
}
