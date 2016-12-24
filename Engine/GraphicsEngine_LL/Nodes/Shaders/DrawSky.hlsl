
struct Sun
{
	float4 dirView;
	float4 dirWorld;
	float4 color;
};


ConstantBuffer<Sun> sun : register(b0);
SamplerState theSampler : register(s0);


struct PS_Input
{
	float4 position : SV_POSITION;
};


PS_Input VSMain(float3 position : POSITION)
{
	PS_Input result;

	result.position = float4(position, 1);

	return result;
}


float4 PSMain(PS_Input input) : SV_TARGET
{
	//return float4(0.4, .75, 1.0, 1.0)*max(0.05, smoothstep(-0.15, 0.1, sun.dirWorld.y));
	return float4(0.4, .6, 1.0, 1.0);
}
