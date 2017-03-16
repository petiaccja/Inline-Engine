
SamplerState theSampler : register(s0);
Texture2D<float4> tex0 : register(t0);
Texture2D<float4> tex1 : register(t1);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD;
};

PS_Input VSMain(float4 position : POSITION)
{
	PS_Input result;

	result.position = position;
	result.texCoord = position.xy*0.5 + 0.5;

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float3 coords = {input.texCoord.x, 1.0-input.texCoord.y, 0.0};
	
	float4 a = tex0.Sample(theSampler, coords);
	float4 b = tex1.Sample(theSampler, coords);
	
	return lerp(a, b, b.a);
}
