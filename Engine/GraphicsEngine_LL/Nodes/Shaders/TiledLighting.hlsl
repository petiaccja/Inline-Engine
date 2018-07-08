Texture2D<uint> lightCullData : register(t600);
Texture2D<float> screenSpaceShadowTex : register(t601);
Texture2D<float4> layeredShadowTex : register(t602);
SamplerState theSampler : register(s500);

//NOTE: actually, just use SRGB, it's got better quality!
float3 linear_to_gamma(float3 col)
{
	return pow(col, float3(1 / 2.2, 1 / 2.2, 1 / 2.2));
}

float3 gamma_to_linear(float3 col)
{
	return pow(col, float3(2.2, 2.2, 2.2));
}

float3 get_sky_color()
{
	return float3(110, 165, 255) / 255.0;
}

float3 hemisphere_ambient_lighting(float3 ws_n)
{
	//hemisphere ambient term for visualization
	float4 sky_col = float4(get_sky_color(), 1);
	float3 sky_dir = float3(0, 0, 1);
	float4 ground_col = float4(float3(0.25, 0.25, 0.25), 1);
	float hemi_intensity = 0.7;

	float vec_hemi = dot(ws_n, sky_dir) * 0.5 + 0.5;
	return hemi_intensity * lerp(ground_col.xyz, sky_col.xyz, vec_hemi);
}

float getLayeredShadow( float2 texCoord, const int layer )
{
	if(layer == 0)
	{
		return layeredShadowTex.SampleLevel(theSampler, texCoord, 0).x;
	}
	else if(layer == 1)
	{
		return layeredShadowTex.SampleLevel(theSampler, texCoord, 0).y;
	}
	else if(layer == 2)
	{
		return layeredShadowTex.SampleLevel(theSampler, texCoord, 0).z;
	}
	else
	{
		return layeredShadowTex.SampleLevel(theSampler, texCoord, 0).w;
	}
}

float3 get_lighting(float4 sv_position, //gl_FragCoord
					float4 albedo, 
					float3 vs_normal,
					float4 vs_pos,
					float roughness,
					float metalness
						)
{
	uint2 global_id = uint2(sv_position.xy);
	float2 global_size = uniforms.screen_dimensions;
	uint2 local_size = uint2(16, 16);
	uint2 group_id = global_id / local_size;
	float2 texel = global_id / global_size;

	uint local_num_of_lights = lightCullData.Load(int3(group_id.x * uniforms.group_size_y + group_id.y, 0, 0));
	//uint local_num_of_lights = lightCullData.Load(int3(global_id, 0));
	
	//return float3(asfloat(local_num_of_lights), 0, 0);
	//return float3(float(local_num_of_lights), 0, 0);

	float3 vs_view_dir = normalize(uniforms.vs_cam_pos.xyz - vs_pos.xyz);

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

			//color += (n_dot_l * attenuation) * (diffuse_color.xyz * 10.0 * albedo.xyz); //TODO: shadow

			color += getCookTorranceBRDF(albedo.xyz,
										 vs_normal,
										 vs_view_dir,
									     light_dir,
										 diffuse_color.xyz * attenuation * 10.0 * getLayeredShadow(texel, 1),
										 roughness, 
										 metalness 
										);

			//const float roughness = 1.0;
			//const float metallic = 0.0;
			//const float3 dielectricF0 = 0.04;
			//
			//float3 L = light_dir;
			//float3 V = vs_view_dir;
			//float3 H = normalize(L + V);
			//float3 N = vs_normal;
			//
			//float NoV = max(dot(N, V), 0.0);
			//float NoL = max(dot(N, L), 0.0);
			//float NoH = max(dot(N, H), 0.0);
			//float VoH = max(dot(V, H), 0.0);
			//
			//float3 brdfDielectric = StandardBRDF(dielectricF0, NoV, NoL, NoH, VoH, roughness, albedo);
			//float3 brdfMetal = StandardBRDF(albedo, NoV, NoL, NoH, VoH, roughness, float3(0.0, 0.0, 0.0));
			//float3 brdf = metallic * brdfMetal + (1 - metallic) * brdfDielectric;
			//
			//color += n_dot_l * brdf * attenuation * diffuse_color;
		}
	}

	float screenSpaceShadow = screenSpaceShadowTex.Sample(samp0, texel);

	color += getCookTorranceBRDF(albedo.xyz,
								 vs_normal,
								 vs_view_dir,
								 -g_lightDir,
								 g_lightColor.xyz * 10.0 * screenSpaceShadow * getLayeredShadow(texel, 0),
								 roughness,
								 metalness);

    //color.xyz += hemisphere_ambient_lighting(g_wsNormal.xyz) * 0.8f * albedo.xyz;

	return color;
	//return g_normal.xyz;
	//return g_ndcPos.xyz;
	//return g_lightColor.xyz;
}