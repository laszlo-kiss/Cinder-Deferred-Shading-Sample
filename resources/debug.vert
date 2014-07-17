#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;

uniform mat4 ciModelViewProjectionMatrix;

out vec2 tc;

void main(){
    tc = texcoord;
    gl_Position = ciModelViewProjectionMatrix * vec4( position, 1.0 );
}