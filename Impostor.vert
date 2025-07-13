#version 460 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;

layout(binding = 0) uniform Camera 
{
	mat4 view;
	mat4 projection;
} cam;

layout(push_constant) uniform PushConstants {
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;

layout(location = 0) out vec2 outTexCoord;

void main(void)
{
	outTexCoord = vTexCoord;
	
	mat4 m4ModelView= cam.view * pc.model;
	
	// zero out camera rotation
    m4ModelView[0].xyz = vec3(1,0,0);
    m4ModelView[1].xyz = vec3(0,1,0);
    m4ModelView[2].xyz = vec3(0,0,1);

	gl_Position = cam.projection * m4ModelView * vec4(vPosition,1.0);
}
