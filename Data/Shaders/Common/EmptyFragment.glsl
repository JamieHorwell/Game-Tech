#version 150 core

in Vertex {
	vec3 position;
} IN;
out vec4 OutFrag;

void main(void)	{
	OutFrag = vec4(IN.position, 1.0f);
}