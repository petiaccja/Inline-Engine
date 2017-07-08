
struct Sun
{
	float4 dir; // world space
	float4 color;
};

struct Cam
{
    float4x4 invViewProj;
    float4 position; // world space
};

ConstantBuffer<Sun> sun : register(b0);
ConstantBuffer<Cam> cam : register(b1);
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
    result.worldPos = mul(float4(position, 1), cam.invViewProj);
	result.worldPos /= result.worldPos.w;

	return result;
}


float4 PSMain(PS_Input input) : SV_TARGET
{
	//return float4(0.4, .75, 1.0, 1.0)*max(0.05, smoothstep(-0.15, 0.1, sun.dir.y));
	float3 lookDir = normalize(input.worldPos - cam.position);
	float horizon = pow(1.f - pow(dot(lookDir, float3(0, 0, 1)), 2), 10);
	float corona = pow(0.5f + 0.5f*dot(-sun.dir, lookDir), 4);
	float sundisk = saturate(600 * (dot(-sun.dir, lookDir) - 0.9993908270191));
	return 0.8*((float4(0.6, 0.86, 1, 1.0)+sun.color)*0.5f*horizon + sun.color*(corona*0.6+sundisk) + float4(0.4, 0.78, 1, 1)*(1-horizon));
}
