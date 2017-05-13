Texture2D<uint> lightCullData : register(t600);

struct light_data
{
	float4 diffuse_color;
	float4 vs_position;
	float4 attenuation_end;
};

struct Uniforms
{
	light_data ld[10];
	float4 screen_dimensions;
	int group_size_x, group_size_y;
	float2 dummy;
};

ConstantBuffer<Uniforms> uniforms : register(b600);

float3 get_tiled_lighting(float4 sv_position, //gl_FragCoord
						  float4 albedo, 
						  float3 vs_normal,
						  float4 vs_pos
						)
{
	uint2 global_id = uint2(sv_position.xy);
	float2 global_size = uniforms.screen_dimensions;
	uint2 local_size = uint2(16, 16);
	uint2 group_id = global_id / local_size;
	float2 texel = global_id / global_size;

	uint local_num_of_lights = lightCullData.Load(int3(group_id.x * uniforms.group_size_y + group_id.y, 0, 0));

	float3 color = float3(0, 0, 0);
	for (uint c = 0; c < local_num_of_lights; ++c)
	{
		uint index = lightCullData.Load(int3(group_id.x * uniforms.group_size_y + group_id.y, c + 1, 0));
		float3 vs_light_pos = uniforms.ld[index].vs_position.xyz;
		float attenuation_end = uniforms.ld[index].attenuation_end;
		float4 diffuse_color = uniforms.ld[index].diffuse_color;

		float3 light_dir = vs_light_pos - vs_pos.xyz;
		float distance = length(light_dir);
		light_dir = normalize(light_dir);

		float attenuation = (attenuation_end - distance) / attenuation_end;

		if (attenuation > 0.0)
		{
			float n_dot_l = max(dot(vs_normal, light_dir), 0.0);

			color += (n_dot_l * attenuation) * (diffuse_color.xyz * 10.0 * albedo.xyz); //TODO: shadow
		}
	}

	return color;
}