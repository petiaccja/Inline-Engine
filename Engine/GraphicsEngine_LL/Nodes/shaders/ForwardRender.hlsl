
struct Transform
{
	float4x4 MVP;
	float4x4 MV;
	float4x4 worldInvTr;
};

struct Sun
{
	float4 dir; // in world space
	float4 color;
};

ConstantBuffer<Transform> transform : register(b0);
ConstantBuffer<Sun> sun : register(b1);
SamplerState theSampler : register(s0);
//SamplerState shadowSampler : register(s1);
Texture2DArray<float4> albedoTex : register(t3);
Texture2D<float> shadowMapTex : register(t0);
Texture2D<float4> shadowMXTex : register(t1);
Texture2D<float4> csmSplitsTex : register(t2);

struct PS_Input
{
	float4 position : SV_POSITION;
	float3 normal : NO;
	float2 texCoord : TEX_COORD0;
	float4 vsPosition : TEX_COORD1;
};

float2 get_shadow_uv(float2 uv, float cascade_idx)
{
	float2 res = float2(uv.x*0.25, uv.y) + cascade_idx * float2(0.25, 0);
	res.x = clamp(res.x, cascade_idx * 0.25, (cascade_idx + 1) * 0.25);
	return res;
}

float offset_lookup(float4 loc, float2 offset, float2 scale, float cascade)
{
	//return shadowMapTex.SampleCmp(shadowSampler, get_shadow_uv(loc.xy, cascade), loc.z, int2(offset)).x;
	//return shadowMapTex.Sample(theSampler, get_shadow_uv(loc.xy, cascade), int2(offset)).x;
	return shadowMapTex.Sample(theSampler, get_shadow_uv(loc.xy, cascade)).x;
}

float shadow_pcf_3x3(float4 shadow_coord, float2 scale, float2 offset, float cascade)
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
	return offset_lookup(shadow_coord, offset, scale, cascade); //TODO only one sample for now
}

float sample_csm(int cascade, float4 vs_pos)
{
	float4x4 shadow_mx;
	for (int d = 0; d < 4; ++d)
	{
		shadow_mx[d] = shadowMXTex.Load(int3(cascade * 4 + d, 0, 0));
	}
	float4 shadow_coord = mul(shadow_mx, vs_pos);
	shadow_coord /= shadow_coord.w;

	float bias = 0.0075;
	shadow_coord.z -= bias;

	float2 offset = fmod(shadow_coord.xy * 0.5, 0.25);
	offset.y += offset.x;  // y ^= x in floating point

	if (offset.y > 1.1)
	{
		offset.y = 0;
	}

	uint3 inputTexSize;
	shadowMapTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	float2 scale = 1.0 / float2(inputTexSize.xy);

	return shadow_pcf_3x3(shadow_coord, scale, offset, cascade);
}

//vec3 get_shadow(sampler2D tex, vec4 shadow_coord)
float get_shadow(float4 vs_pos)
{
	int cascade;
	for (int c = 0; c < 4; ++c)
	{
		float2 split = csmSplitsTex.Load(int3(c, 0, 0)).xy;

		if (-vs_pos.z >= split.x && -vs_pos.z < split.y)
		{
			cascade = c;
			break;
		}
	}

	float shadow_term = sample_csm(cascade, vs_pos);

	{ //filter across cascades
		const float blendThreshold = 0.05f;
		float2 split = csmSplitsTex.Load(int3(cascade, 0, 0)).xy;
		float prevSplit = split.x;
		float nextSplit = split.y;
		float splitSize = nextSplit - prevSplit;
		float splitDistPrev = (-vs_pos.z - prevSplit) / splitSize;
		float splitDistNext = (nextSplit + vs_pos.z) / splitSize;

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

	//return cascade;
	return shadow_term;
	//return float(shadow_coord.z - 0.005 < texture(tex, shadow_coord.xy).x) * shadow_selector / 4.0;
	//return ls_ndc_pos.z/0.25;
	//return shadow_coord.xyz;
	//return vec3(texture(tex, gl_FragCoord.xy / vec2(1280, 720)).x*0.1);
}

PS_Input VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEX_COORD)
{
	PS_Input result;

	float3 worldNormal = normalize(mul(transform.worldInvTr, float4(normal.xyz, 0.0)).xyz);

	result.position = mul(transform.MVP, position);
	result.vsPosition = mul(transform.MV, position);
	result.normal = worldNormal;
	result.texCoord = texCoord.xy;

	return result;
}

float4 PSMain(PS_Input input): SV_TARGET
{
	float3 coords = {input.texCoord.x, 1-input.texCoord.y, 0.0};
	float3 albedo = albedoTex.Sample(theSampler, coords).rgb;
	float shadow = get_shadow(input.vsPosition);
	return float4((max(0.0, dot(normalize(input.normal), -sun.dir)) * shadow) * albedo, 1.0);
}
