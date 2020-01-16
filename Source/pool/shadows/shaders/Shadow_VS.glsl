#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output value to fragment shader
out vec2 out_texture_coord;

void main()
{
	gl_Position = Projection * View * Model * vec4(v_position, 1.0);
	out_texture_coord = v_texture_coord;
}
