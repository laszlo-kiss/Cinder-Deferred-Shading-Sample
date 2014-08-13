#version 330

in vec4 wcPosition;
in vec3 wcNormal;
in vec3 diffuse;
in float specular;

layout (location = 0 ) out vec4 dPosition;
layout (location = 1 ) out vec4 dNormal;
layout (location = 2 ) out vec4 dColor;

uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform mat4 ciViewMatrix;
uniform sampler2D diffuseMap;

in vec2 texCoord;

vec3 perturbNormal2Arb( in vec3 eye_pos, in vec3 surf_norm, in vec2 tex_coord ) {
    vec3 q0 = dFdx( eye_pos.xyz );
    vec3 q1 = dFdy( eye_pos.xyz );
    vec2 st0 = dFdx( tex_coord );
    vec2 st1 = dFdy( tex_coord );
    vec3 S = normalize(  q0 * st1.t - q1 * st0.t );
    vec3 T = normalize( -q0 * st1.s + q1 * st0.s );
    vec3 N = normalize( surf_norm );
    vec3 mapN = -texture( normalMap, tex_coord ).xyz * 2.0 - 1.0;
    mapN.xy = vec2(.5) * mapN.xy;
    mat3 tsn = mat3( S, T, N );
    return normalize( tsn * mapN );
}


void main(){
    
    //save the world space position
    dPosition = wcPosition;
    //save the world space normal
    //to use normal mapping preturb the normal with above function before sending it out
    dNormal.rgb = wcNormal;
    //This is the specularMap value
    dNormal.a = texture( specularMap, texCoord ).r;
    //texture look up
    dColor.rgb = texture( diffuseMap, texCoord ).rgb;
    //this is the shininess value
    dColor.a = specular;
    
    ////////////////
    ///NOTES:
    ////////////////
    
    //normals can be comressed to 2 values in spherical coordinates
    //see http://aras-p.info/texts/CompactNormalStorage.html
    
    //depth buffer is automatically created in this case, but a waste of texture space, but you could pack the depth into 2 half floats
    
    //encode in gbuffer
    //float depth = gl_FragCoord.z / gl_FragCoord.w;
    //float low_d = floor( depth*1024 );
    //float high_d = fract( depth*1024 );
    
    //decode in screenspace
    //float depth = (d_high+d_low)/1024;
    
}