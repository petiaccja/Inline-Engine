
struct Sun
{
	float4x4 invViewProj;
	float4 dirView;
	float4 dirWorld;
	float4 color;
	float4 viewPos;
};


ConstantBuffer<Sun> sun : register(b0);
SamplerState theSampler : register(s0);


struct PS_Input
{
	float4 position : SV_POSITION;
	float4 worldPos : COLOR0;
};


PS_Input VSMain(float3 position : POSITION)
{
	PS_Input result;

	result.position = float4(position, 1);
	result.worldPos = mul(sun.invViewProj, float4(position, 1));
	result.worldPos /= result.worldPos.w;

	return result;
}


float4 PSMain(PS_Input input) : SV_TARGET
{
	//return float4(0.4, .75, 1.0, 1.0)*max(0.05, smoothstep(-0.15, 0.1, sun.dirWorld.y));
	float3 lookDir = normalize(input.worldPos - sun.viewPos);
	float horizon = pow(1.f - pow(dot(lookDir, float3(0, 0, 1)), 2), 10);
	float corona = pow(0.5f + 0.5f*dot(sun.dirWorld, lookDir), 4);
	return 0.8*(float4(0.6, 0.86, 1, 1.0)*horizon + sun.color*corona*0.3 + float4(0.4, 0.78, 1, 1)*(1-horizon));
}
