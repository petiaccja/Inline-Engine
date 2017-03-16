
struct Transform
{
	float4x4 MVP;
};

ConstantBuffer<Transform> transform : register(b0);

SamplerState theSampler : register(s0);
Texture2D<float4> tex : register(t0);


struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD;
};


PS_Input VSMain(float2 position : POSITION, float2 texCoord : TEX_COORD)
{
	PS_Input result;

	float4 pos = {position.x, position.y, 0, 1};
	result.position = mul(transform.MVP, pos);
	result.texCoord = texCoord;

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float3 coords = {input.texCoord.x, 1.0-input.texCoord.y, 0.0};
	
	return tex.Sample(theSampler, coords);
}
