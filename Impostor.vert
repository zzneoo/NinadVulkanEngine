#version 460 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;


layout(std140, set = 0, binding = 0) uniform FrameData 
{
    mat4 view;        // offset 0   → 64 bytes
    mat4 projection;  // offset 64  → 64 bytes
    float fTime;      // offset 128
    uint  frameID;    // offset 132
    vec3 cameraPos;   // offset 144 → needs to be aligned to 16 bytes
    float _pad;       // offset 156 → pad to 160 (multiple of 16)
}global;

layout(push_constant) uniform PushConstants {
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1)flat  out vec3 outViewVector;

void main(void)
{
	outTexCoord = vTexCoord;
	
	mat4 m4ModelView= global.view * pc.model;
	
	vec4 worldPosition = pc.model * vec4(0.0,0.0,0.0,1.0);
	
	outViewVector = normalize(global.cameraPos - worldPosition.xyz);
	
	// zero out camera rotation
    m4ModelView[0].xyz = vec3(1,0,0);
    m4ModelView[1].xyz = vec3(0,1,0);
    m4ModelView[2].xyz = vec3(0,0,1);

	gl_Position = global.projection * m4ModelView * vec4(vPosition,1.0);
}
