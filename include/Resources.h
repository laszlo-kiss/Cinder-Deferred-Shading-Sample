#pragma once
#include "cinder/CinderResources.h"

//#define RES_MY_RES			CINDER_RESOURCE( ../resources/, image_name.png, 128, IMAGE )




#define DEBUG_VERT			CINDER_RESOURCE( ../resources/, debug.vert, 128, GLSL )
#define DEBUG_FRAG			CINDER_RESOURCE( ../resources/, debug.frag, 129, GLSL )
#define GPASS_VERT			CINDER_RESOURCE( ../resources/, gPass.vert, 130, GLSL )
#define GPASS_FRAG			CINDER_RESOURCE( ../resources/, gPass.frag, 131, GLSL )
#define LIGHT_VERT			CINDER_RESOURCE( ../resources/, gLightPass.vert, 132, GLSL )
#define LIGHT_FRAG			CINDER_RESOURCE( ../resources/, gLightPass.frag, 133, GLSL )

#define ROCK                CINDER_RESOURCE( ../resources/, rock.png, 134, PNG )
#define ROCK_NRM            CINDER_RESOURCE( ../resources/, rock_NRM.png, 135, PNG )
#define ROCK_SPEC           CINDER_RESOURCE( ../resources/, rock_SPEC.png, 136, PNG )

#define FOIL                CINDER_RESOURCE( ../resources/, foil_DIFF.jpg, 137, JPG )
#define FOIL_NRM            CINDER_RESOURCE( ../resources/, foil_NRM.png, 138, PNG )
#define FOIL_SPEC           CINDER_RESOURCE( ../resources/, foil_SPEC.png, 139, PNG )