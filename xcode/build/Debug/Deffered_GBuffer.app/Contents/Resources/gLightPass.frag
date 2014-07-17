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


vec3 sunContribution( in vec3 Position, in vec3 Normal, in vec3 Diffuse, in float spec, in Light light ) {
    vec3 n = normalize( Normal );
    vec3 d = light.position - Position;
    vec3 s = normalize( d );
    vec3 v = normalize( -Position );
    vec3 h = normalize( v + s );
    return light.intensity * ( light.diffuse * max( dot(s, n), 0.0 ) * Diffuse  +
                               light.specular * pow( max(dot(h,n),0.0), spec )
                              );
}

vec3 ads( in vec3 Position, in vec3 Normal, in vec3 Diffuse, in float spec, in Light light ) {
    vec3 n = normalize( Normal );
    vec3 d = light.position - Position;
    vec3 s = normalize( d );
    vec3 v = normalize( -Position );
    vec3 h = normalize( v + s );
    float dist = length( d );
    return light.intensity * ( light.diffuse * max( dot(s, n), 0.0 ) * Diffuse  +
                               light.specular * pow( max(dot(h,n),0.0), spec )
                              ) * ( 1. / ( .02 * ( dist + pow(dist, 2.) ) ) );
}

void main(){
    
    if( texture(DepthTex, texCoord).r < 1. ){
    
        vec4 wcPosition = texture( wcPositions, texCoord );
        vec3 wcNormal = texture( wcNormals, texCoord ).rgb;
        vec4 diffSpec = texture( DiffuseSpecular, texCoord );
        FragColor = vec4(0.);
        
        for(int i=0;i<numLights;i++){
            
            //        float d = length(lights[i].position - wcPosition.xyz);
            //
            //        if( d > maxes[0] && d > maxes[1] && d > maxes[2] ){
            //            maxes[2] = d;
            //            maxind[2] = i;
            //        }else if( d > maxes[0] && d > maxes[1] && d < maxes[2] ){
            //            maxes[1] = d;
            //            maxind[1] = i;
            //        }else if( d > maxes[0] && d < maxes[1] && d < maxes[2] ){
            //            maxes[0] = d;
            //            maxind[0] = i;
            //        }
            FragColor.rgb += ads( wcPosition.xyz, wcNormal, diffSpec.rgb, diffSpec.a, lights[i] );
            
        }
        
        FragColor.rgb += sunContribution( wcPosition.xyz, wcNormal, diffSpec.rgb, diffSpec.a, Sun );
        FragColor.rgb += GlobalAmbient;
        //    FragColor.rgb += ads( wcPosition.xyz, wcNormal, diffSpec.rgb, diffSpec.a, lights[maxind[1]], maxes[1] );
        //    FragColor.rgb += ads( wcPosition.xyz, wcNormal, diffSpec.rgb, diffSpec.a, lights[maxind[2]], maxes[2] );
        FragColor.a = 1.;
        
    }else{
        discard;
    }
}