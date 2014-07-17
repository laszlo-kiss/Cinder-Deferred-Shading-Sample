#version 330

in vec4 wcPosition;
in vec3 wcNormal;
in vec3 diffuse;
in float specular;

layout (location = 0 ) out vec4 dPosition;
layout (location = 1 ) out vec4 dNormal;
layout (location = 2 ) out vec4 dColor;

void main(){
    
    dPosition = wcPosition;
    dNormal = vec4(wcNormal, 1.0);
    dColor.rgb = diffuse;
    dColor.a = specular;
    
}