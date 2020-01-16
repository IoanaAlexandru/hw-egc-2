#version 330
#define Diffuse	u_texture_0
#define Alpha	u_texture_1
#define DepthT	u_texture_7

layout(location = 0) in vec2 texture_coord;
layout(location = 1) in vec3 world_position;
layout(location = 2) in vec3 world_normal;
layout(location = 3) in vec4 shadow_coord;
layout(location = 4) in vec3 frag_position;
layout(location = 5) in vec3 frag_color;
layout(location = 6) in vec3 V;

uniform sampler2D u_texture_0;	// Diffuse texture
uniform sampler2D u_texture_1;	// Alpha texture
uniform sampler2D u_texture_7;	// Depth buffer

// Material parameters
uniform float material_ks;
uniform int material_shininess;
uniform vec3 object_color;
uniform float z_offset;
uniform vec3 light_position;
uniform vec3 eye_position;

layout(location = 0) out vec4 out_color;
//layout(location = 0) out vec3 frag_color;
//layout(location = 1) out vec3 frag_position;
//layout(location = 2) out vec3 frag_normal;
//layout(location = 3) out vec2 frag_texture_coord;

const float PI = 3.14159265359;

// PBR
float DistributionGGX(vec3 N, vec3 H, float z_offset)
{
    float a = z_offset * z_offset;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float z_offset)
{
    float r = (z_offset + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float z_offset)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, z_offset);
    float ggx1 = GeometrySchlickGGX(NdotL, z_offset);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// End of PBR

void main() {
    vec3 R = reflect(-V, world_normal); 

    // Calculate reflectance at normal incidence  
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, object_color, material_shininess);
   
	// Calculate light radiance
	vec3 L = normalize(light_position - world_position);
	vec3 H = normalize(V + L);
	float distance = length(light_position - world_position);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = frag_color * attenuation;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(world_normal, H, z_offset);   
	float G = GeometrySmith(world_normal, V, L, z_offset);      
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
	   
	vec3 nominator = NDF * G * F; 
	float denominator = 4.0 * max(dot(V, world_normal), 0.0) * max(dot(L, world_normal), 0.0) + 0.001f; // 0.001 to prevent divide by zero.
	vec3 brdf = nominator / denominator;
	
	// kS is equal to Fresnel
	vec3 kS = F;
	
	// For energy conservation, kD and kS can't be above 1.0 (unless the surface emits light)
	vec3 kD = vec3(1.0) - material_ks;
	
	// Multiply kD by the inverse metalness such that only non-metals have diffuse lighting
	kD *= 1.0 - material_shininess;	  

	// Scale light by NdotL
	float NdotL = max(dot(world_normal, L), 0.0);        

	// Add to outgoing radiance Lo
	vec3 Lo = (kD * object_color / PI + brdf) * radiance * NdotL;  
    
    // Compute ambient lightning
    vec3 ambient = vec3(0.03) * object_color;
    vec3 fragm_color = frag_color + ambient + Lo;

    // HDR tonemapping
    fragm_color /= (fragm_color + vec3(1.0));
	
    out_color = vec4(fragm_color, 1.0);
}