#version 330 core

uniform mat4 uModelMtx;

layout (location = 0) in  vec3 position;

out Vertex	{
	vec3 position;
} OUT;

void main(void)	
{
	gl_Position	= uModelMtx * vec4(position, 1.0);
	OUT.position = (uModelMtx * vec4(position, 1.0)).xyz;
}