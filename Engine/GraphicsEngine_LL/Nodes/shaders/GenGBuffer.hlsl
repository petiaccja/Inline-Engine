
struct Transform
{
	float4x4 MVP;
	float4x4 worldViewInvTr;
};

ConstantBuffer<Transform> cb : register(b0);
SamplerState theSampler : register(s0);
Texture2DArray<float4> albedoTex : register(t0);
Texture2DArray<float3> normalTex : register(t1);


struct PS_Input
{
	float4 position : SV_POSITION;
	float2 normal : NO;
	float2 texCoord : TEX_COORD;
};


struct PS_Output
{
	float4 albedo_roughness : SV_TARGET0;
	float2 normal : SV_TARGET1;
};


PSInput VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEX_COORD)
{
	PS_Input result;

	float3 worldNormal = normalize(mul(cb.worldViewInvTr, float4(normal.xyz, 0.0)).xyz);

	result.position = mul(cb.MVP, position);
	result.normal = worldNormal.xy;
	result.texCoord = texCoord;

	return result;
}


PS_OUTPUT PSMain(PS_Input input)
{
	PS_Output output;

	float3 coords = {input.texCoord.x, 1-input.texCoord.y, 0.0};

	output.albedo_roughness = float4(albedoTex.Sample(theSampler, coords).rgb, 0.25);
	//TODO implement normal mapping
	output.normal = input.normal;
}
