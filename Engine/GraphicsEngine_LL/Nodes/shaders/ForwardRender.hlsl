
struct Transform
{
	float4x4 MVP;
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
	float2 texCoord : TEX_COORD;
};


PS_Input VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEX_COORD)
{
	PS_Input result;

	float3 worldNormal = normalize(mul(transform.worldInvTr, float4(normal.xyz, 0.0)).xyz);

	result.position = mul(transform.MVP, position);
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
