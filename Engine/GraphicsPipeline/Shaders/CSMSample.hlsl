Texture2DArray<float> shadowMapTex : register(t500);
Texture2D<float4> shadowMXTex : register(t501);
Texture2D<float2> csmSplitsTex : register(t502);
Texture2D<float4> lightMVPTex : register(t503);
SamplerState theSampler : register(s500);

// TODO replace all the hardwired constants that
// depend on number of cascades (shadowMapTex array element count)

static const float4 radarColors[14] =
{
	{ 0,0.9255,0.9255,1 },   // cyan
	{ 0,0.62745,0.9647,1 },  // light blue
	{ 0,0,0.9647,1 },        // blue
	{ 0,1,0,1 },             // bright green
	{ 0,0.7843,0,1 },        // green
	{ 0,0.5647,0,1 },        // dark green
	{ 1,1,0,1 },             // yellow
	{ 0.90588,0.75294,0,1 }, // yellow-orange
	{ 1,0.5647,0,1 },        // orange
	{ 1,0,0,1 },             // bright red
	{ 0.8392,0,0,1 },        // red
	{ 0.75294,0,0,1 },       // dark red
	{ 1,0,1,1 },             // magenta
	{ 0.6,0.3333,0.7882,1 }, // purple
};

float4 NumToRadarColor(int num, int numMax)
{
	// black for 0
	if (num == 0) return float4(0, 0, 0, 1);
	// light purple for reaching the max
	else if (num == numMax) return float4(0.847, 0.745, 0.921, 1);
	// white for going over the max
	else if (num > numMax) return float4(1, 1, 1, 1);
	// else use weather radar colors
	else
	{
		// use a log scale to provide more detail when the number of lights is smaller

		// want to find the base b such that the logb of uMaxNumLightsPerTile is 14
		// (because we have 14 radar colors)
		float fLogBase = exp2(0.07142857f*log2((float)numMax));

		// change of base
		// logb(x) = log2(x) / log2(b)
		int idx = floor(log2((float)num) / log2(fLogBase));
		return radarColors[idx];
	}
}

float3 GetShadowUv(float2 uv, float cascadeIdx)
{
	// float2 res = float2(uv.x*0.25, uv.y) + cascade_idx * float2(0.25, 0);
	// res.x = clamp(res.x, cascade_idx * 0.25, (cascade_idx + 1) * 0.25);
	// return res;
	
	return float3(uv, cascadeIdx);
}

float OffsetLookup(float4 loc, float2 offset, float2 scale, float cascade)
{
	//return shadowMapTex.SampleCmp(shadowSampler, get_shadow_uv(loc.xy, cascade), loc.z, int2(offset)).x;
	//return shadowMapTex.Sample(theSampler, get_shadow_uv(loc.xy, cascade), int2(offset)).x;
	return shadowMapTex.SampleLevel(theSampler, GetShadowUv(loc.xy, cascade), 0.0).x;
}

float ShadowPcf3x3(float4 shadowCoord, float2 scale, float2 offset, float cascade)
{
	/*const int size = 1;
	float accum = 0;
	int count = 0;
	for (int c = -size; c <= size; ++c)
	for (int d = -size; d <= size; ++d)
	{
	accum += offset_lookup(shadow_coord, offset + float2(c, d) * 1.5, scale, cascade);
	++count;
	}

	return accum / count;*/
	return OffsetLookup(shadowCoord, offset, scale, cascade); //TODO only one sample for now
}

float SampleCsm(int cascade, float4 vsPos
#ifdef CSM_EXTENDED_INFO
, out float4 _shadowCoord
#endif
)
{
	float4x4 shadowMx;
	for (int d = 0; d < 4; ++d)
	{
		shadowMx[d] = shadowMXTex.Load(int3(cascade * 4 + d, 0, 0));
	}
	float4 shadowCoord = mul(vsPos, shadowMx);
	shadowCoord /= shadowCoord.w;

	float bias = 0.0075;
	shadowCoord.z -= bias;

	float2 offset = fmod(shadowCoord.xy * 0.5, 0.25);
	offset.y += offset.x;  // y ^= x in floating point

	if (offset.y > 1.1)
	{
		offset.y = 0;
	}

	uint3 inputTexSize;
	uint levels;
	shadowMapTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z, levels);
	float2 scale = 1.0 / float2(inputTexSize.xy);

	//shadow_coord.xy = shadow_coord.xy * float2(0.5, -0.5) + 0.5;
	
	#ifdef CSM_EXTENDED_INFO
	_shadowCoord = float4(shadowCoord.xyz, cascade);
	#endif

	float shadowDepth = OffsetLookup(shadowCoord, offset, scale, cascade);
	//return float3(shadow_coord.x < 0.0, shadow_coord.y < 0.0, shadow_coord.z < 0.0);
	return shadowCoord.z < shadowDepth;
	//return shadowMapTex.Sample(theSampler, float3(shadow_coord.xy * 0.5 + 0.5, 1.0)).x;
	//return float(inputTexSize.z > 3);
	//return shadow_depth;
	//return shadow_pcf_3x3(shadow_coord, scale, offset, cascade);
}

//vec3 get_shadow(sampler2D tex, vec4 shadow_coord)
float3 GetCsmShadow(float4 vsPos
#ifdef CSM_EXTENDED_INFO
, out float4 shadowCoord
#endif
)
{
	//return sample_csm(0, vs_pos);

	int cascade;
	for (int c = 0; c < 4; ++c)
	{
		float2 split = csmSplitsTex.Load(int3(c, 0, 0)).xy;

		if (vsPos.z >= split.x && vsPos.z < split.y)
		{
			cascade = c;
			break;
		}
	}

	float shadowTerm = SampleCsm(cascade, vsPos
	#ifdef CSM_EXTENDED_INFO
	, shadowCoord
	#endif
	);

	{ //filter across cascades
		const float blendThreshold = 0.05f;
		float2 split = csmSplitsTex.Load(int3(cascade, 0, 0)).xy;
		float prevSplit = split.x;
		float nextSplit = split.y;
		float splitSize = nextSplit - prevSplit;
		float splitDistPrev = (vsPos.z - prevSplit) / splitSize;
		float splitDistNext = (nextSplit + vsPos.z) / splitSize;

		//method 1
		//sharper transition
		/*if(splitDistNext <= blendThreshold && cascade != 3)
		{
		float next_shadow_term = sample_csm( cascade + 1, tex, vs_pos );
		float mix_amount = smoothstep(0.0f, blendThreshold, splitDistNext);

		//if( shadow_term > 0 )
		shadow_term = mix(next_shadow_term, shadow_term, mix_amount);
		}*/

		//method 2
		//smoother transition
		/*if(splitDistPrev <= BlendThreshold && cascade != 0)
		{
		float prev_shadow_term = sample_csm( cascade - 1, tex, vs_pos );
		float mix_amount = smoothstep(0.0f, BlendThreshold, splitDistPrev);

		if( shadow_term > 0 )
		if( shadow_term > 0 )
		shadow_term = mix(prev_shadow_term, shadow_term, mix_amount);
		}*/
	}

	//return float3(1,1,1);
	//return cascade * 0.25;
	//return shadow_term;
	return float3(shadowTerm, shadowTerm, shadowTerm);
	//return num_to_radar_color(cascade, 4).xyz;
	//return float3(shadow_term, shadow_term, shadow_term);// *0.5 + num_to_radar_color(cascade, 4).xyz * 0.1;
	//return float(shadow_coord.z - 0.005 < texture(tex, shadow_coord.xy).x) * shadow_selector / 4.0;
	//return ls_ndc_pos.z/0.25;
	//return shadow_coord.xyz;
	//return vec3(texture(tex, gl_FragCoord.xy / vec2(1280, 720)).x*0.1);
}
