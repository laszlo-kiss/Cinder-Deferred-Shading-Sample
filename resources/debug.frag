#version 330

in vec2 tc;
uniform sampler2D tex0;

layout (location = 0) out vec4 oColor;

void main(){
    oColor = texture(tex0, tc);
    //oColor = vec4(1.);
}