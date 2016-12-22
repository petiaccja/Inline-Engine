


SamplerState theSampler : register(s0);
Texture2D<float4> tex : register(t0);

struct PSInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD;
};

PSInput VSMain(float4 position : POSITION)
{
	PSInput result;

	result.position = position;
	result.texCoord = position.xy*0.5 + 0.5;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float3 coords = {input.texCoord.x, input.texCoord.y, 0.0};
	
	return tex.Sample(theSampler, coords);
}
