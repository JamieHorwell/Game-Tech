#version 330 core
#define SHADOWMAP_NUM  3

#define SHADOWMAP_NUM_VERTS  9

layout(triangles) in;
layout(triangle_strip, max_vertices = SHADOWMAP_NUM_VERTS) out; 

uniform mat4 uShadowTransform[4];

out Vertex	{
	vec3 position;
} OUT;

void main()  
{  	
	for (int layer = 0; layer < SHADOWMAP_NUM; ++layer)
	{
		for (int i = 0; i < gl_in.length(); ++i) {
			gl_Position = uShadowTransform[layer] * gl_in[i].gl_Position;
			OUT.position = (uShadowTransform[layer] * gl_in[i].gl_Position).xyz;
			gl_Layer = layer;
			EmitVertex();
		}
		EndPrimitive();
	}
}  