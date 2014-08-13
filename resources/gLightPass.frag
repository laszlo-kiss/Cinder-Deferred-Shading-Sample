#version 330

uniform sampler2D wcPositions;
uniform sampler2D wcNormals;
uniform sampler2D DiffuseSpecular;
uniform sampler2D DepthTex;

in vec2 texCoord;

layout (location = 0) out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
};

uniform Light lights[64];
uniform Light Sun;
uniform float numLights;
uniform vec3 GlobalAmbient;


vec3 sunContribution( in vec3 Position, in vec3 Normal, in vec3 Diffuse, in float spec_power, in float spec_map, in Light light ) {
    vec3 n = normalize( Normal );
    vec3 d = light.position - Position;
    vec3 s = normalize( d );
    vec3 v = normalize( -Position );
    vec3 h = normalize( v + s );
    return light.intensity * ( light.diffuse * max( dot(s, n), 0.0 ) * Diffuse  +
                               light.specular * pow( max(dot(h,n),0.0), spec_power ) 
                              );
}

vec3 ads( in vec3 Position, in vec3 Normal, in vec3 Diffuse, in float spec_power, in float spec_map, in Light light ) {
    vec3 n = normalize( Normal );
    vec3 d = light.position - Position;
    vec3 s = normalize( d );
    vec3 v = normalize( -Position );
    vec3 h = normalize( v + s );
    float dist = length( d );
    return light.intensity * ( light.diffuse * max( dot(s, n), 0.0 ) * Diffuse  +
                               light.specular * pow( max(dot(h,n),0.0), spec_power )
                              ) * ( 1. / ( .0002 * ( dist + pow(dist, 2.) ) ) );
}

void main(){
    
    if( texture(DepthTex, texCoord).r < 1. ){
    
        vec4 wcPosition = texture( wcPositions, texCoord );
        vec4 wcNormal = texture( wcNormals, texCoord );
        vec4 diffSpec = texture( DiffuseSpecular, texCoord );
        FragColor = vec4(0.);
        
        for(int i=0;i<numLights;i++){
            
            FragColor.rgb += ads( wcPosition.xyz, wcNormal.xyz, diffSpec.rgb, diffSpec.a, wcNormal.a, lights[i] );
            
        }
        
        FragColor.rgb += sunContribution( wcPosition.xyz, wcNormal.xyz, diffSpec.rgb, diffSpec.a, wcNormal.a, Sun );
        FragColor.rgb += GlobalAmbient;
        FragColor.a = 1.;
        
    }else{
        discard;
    }
}