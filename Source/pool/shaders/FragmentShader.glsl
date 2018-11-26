#version 330

// Get color value from vertex shader
layout(location = 0) out vec4 out_color;

in vec3 frag_position;
in vec3 frag_color;

void main()
{
	out_color = vec4(frag_color, 1);
}