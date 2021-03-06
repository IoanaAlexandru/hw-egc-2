#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Uniforms for light properties
uniform vec3 light_position;
uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;
uniform vec3 object_color;
uniform float z_offset;

// Output value to fragment shader
out vec3 frag_position;
out vec3 frag_color;

// Output values for PBR
out vec3 world_pos;
out vec3 N;
out vec3 V;

void main()
{
	// Compute world space vectors
	world_pos = (Model * vec4(v_position,1)).xyz;
	N = normalize(mat3(Model) * v_normal);

	vec3 L = normalize(light_position - world_pos);
	V = normalize(eye_position - world_pos);
	vec3 H = normalize(L + V);

	// Define ambient light component
	float ambient_light = 0.5;

	// Compute diffuse light component
	float diffuse_light = material_kd * max (dot(N, L), 0);

	// Compute specular light component
	int has_light = 0;
	if (dot(N, L) > 0)
		has_light = 1;

	float specular_light = material_ks * has_light * pow(max(dot(N, H), 0), material_shininess);

	// Compute light
	float d = distance(light_position, world_pos);
	float attenuation = 1/pow(d, 2);

	float light = ambient_light + attenuation * (diffuse_light + specular_light);

	// Send color light output to fragment shader
	frag_color = object_color * light;

	// Add offset on z axis for animation
	frag_position = v_position + vec3(0, 0, z_offset);

	gl_Position = Projection * View * Model * vec4(frag_position, 1.0);
}
