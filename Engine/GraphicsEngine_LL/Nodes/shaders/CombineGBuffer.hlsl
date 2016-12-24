
struct Sun
{
	float4 dir; // in view space
	float4 color;
};

ConstantBuffer<Sun> sun : register(b0);
SamplerState theSampler : register(s0);
Texture2D<float4> tex_albedoRoughness : register(t0);
Texture2D<float2> tex_normal : register(t1);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD;
};

PS_Input VSMain(float3 position : POSITION)
{
	PS_Input result;

	result.position = float4(position, 1);
	result.texCoord = position.xy*0.5 + 0.5;

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float3 coords = {input.texCoord.x, 1.0-input.texCoord.y, 0.0};
	
	float2 nSmpl = tex_normal.Sample(theSampler, coords).xy;
	float3 normal = float3(nSmpl.xy, sqrt(1.0 - nSmpl.x*nSmpl.x - nSmpl.y*nSmpl.y));
	
	float shade = max(0.0, dot(normal, sun.dir.xyz));
	
	return tex_albedoRoughness.Sample(theSampler, coords) * shade * sun.color;
}
