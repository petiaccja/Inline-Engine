
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
Texture2DArray<float4> albedoTex : register(t0);

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

PS_Input VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEX_COORD)
{
	PS_Input result;

    float3 worldNormal = normalize(mul(float4(normal.xyz, 0.0), transform.worldInvTr).xyz);

    result.position = mul(position, transform.MVP);
    result.vsPosition = mul(position, transform.MV);
	result.normal = worldNormal;
	result.texCoord = texCoord.xy;

	return result;
}

float4 PSMain(PS_Input input): SV_TARGET
{
	float3 coords = {input.texCoord.x, 1-input.texCoord.y, 0.0};
	float3 albedo = albedoTex.Sample(theSampler, coords).rgb;
	return float4(max(0.0, dot(normalize(input.normal), -sun.dir)) * albedo, 1.0);
}
