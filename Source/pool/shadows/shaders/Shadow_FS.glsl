#version 330

// Get color value from vertex shader
layout(location = 0) out float fragment_depth;

in vec2 out_texture_coord;

uniform sampler2D u_texture_1;

void main()
{
	float depth = texture(u_texture_1, out_texture_coord).x;
	depth = 1.0 - (1.0 - depth) * 25.0;
	fragment_depth = depth;
	// out_color = vec4(depth);
	// out_color = vec4(frag_color, 1);
}