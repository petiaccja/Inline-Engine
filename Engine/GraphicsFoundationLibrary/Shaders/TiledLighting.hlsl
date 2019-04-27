Texture2D<uint> lightCullData : register(t600);
#ifndef NO_SSShadow
Texture2D<float> screenSpaceShadowTex : register(t601);
#endif
Texture2D<float4> layeredShadowTex : register(t602);
SamplerState theSampler : register(s500);

//NOTE: actually, just use SRGB, it's got better quality!
float3 LinearToGamma(float3 col)
{
	return pow(col, float3(1 / 2.2, 1 / 2.2, 1 / 2.2));
}

float3 GammaToLinear(float3 col)
{
	return pow(col, float3(2.2, 2.2, 2.2));
}

float3 GetSkyColor()
{
	return float3(110, 165, 255) / 255.0;
}

float3 HemisphereAmbientLighting(float3 wsN)
{
	//hemisphere ambient term for visualization
	float4 skyCol = float4(GetSkyColor(), 1);
	float3 skyDir = float3(0, 0, 1);
	float4 groundCol = float4(float3(0.25, 0.25, 0.25), 1);
	float hemiIntensity = 0.7;

	float vecHemi = dot(wsN, skyDir) * 0.5 + 0.5;
	return hemiIntensity * lerp(groundCol.xyz, skyCol.xyz, vecHemi);
}

float GetLayeredShadow( float2 texCoord, const int layer )
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

float3 GetLighting(float4 svPosition, //gl_FragCoord
					float4 albedo, 
					float3 vsNormal,
					float4 vsPos,
					float roughness,
					float metalness
						)
{
	uint2 globalId = uint2(svPosition.xy);
	float2 globalSize = uniforms.screenDimensions;
	uint2 localSize = uint2(16, 16);
	uint2 groupId = globalId / localSize;
	float2 texel = globalId / globalSize;

	uint localNumOfLights = lightCullData.Load(int3(groupId.x * uniforms.groupSizeY + groupId.y, 0, 0));
	//uint local_num_of_lights = lightCullData.Load(int3(global_id, 0));
	
	//return float3(asfloat(local_num_of_lights), 0, 0);
	//return float3(float(local_num_of_lights), 0, 0);

	float3 vsViewDir = normalize(uniforms.vsCamPos.xyz - vsPos.xyz);

	float3 color = float3(0, 0, 0);
	for (uint c = 0; c < localNumOfLights; ++c)
	{
		uint index = lightCullData.Load(int3(groupId.x * uniforms.groupSizeY + groupId.y, c + 1, 0));
		float3 vsLightPos = uniforms.ld[index].vsPosition.xyz;
		float attenuationEnd = uniforms.ld[index].attenuationEnd;
		float4 diffuseColor = uniforms.ld[index].diffuseColor;

		float3 lightDir = vsLightPos - vsPos.xyz;
		float distance = length(lightDir);
		lightDir = normalize(lightDir);

		float attenuation = (attenuationEnd - distance) / attenuationEnd;

		if (attenuation > 0.0)
		{
			float nDotL = max(dot(vsNormal, lightDir), 0.0);

			//color += (n_dot_l * attenuation) * (diffuse_color.xyz * 10.0 * albedo.xyz); //TODO: shadow

			color += getCookTorranceBRDF(albedo.xyz,
										 vsNormal,
										 vsViewDir,
									     lightDir,
										 diffuseColor.xyz * attenuation * 10.0 * GetLayeredShadow(texel, 1),
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

	float screenSpaceShadow = 1.0;
	#ifndef NO_SSShadow
	screenSpaceShadow = screenSpaceShadowTex.Sample(samp0, texel);
	#endif

	color += getCookTorranceBRDF(albedo.xyz,
								 vsNormal,
								 vsViewDir,
								 -g_lightDir,
								 g_lightColor.xyz * 10.0 * screenSpaceShadow * GetLayeredShadow(texel, 0),
								 roughness,
								 metalness);

    //color.xyz += hemisphere_ambient_lighting(g_wsNormal.xyz) * 0.8f * albedo.xyz;

	return color;
	//return g_normal.xyz;
	//return g_ndcPos.xyz;
	//return g_lightColor.xyz;
}