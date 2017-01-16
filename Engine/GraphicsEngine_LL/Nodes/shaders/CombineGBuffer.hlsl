
struct Sun
{
	float4 dir; // in view space
	float4 color;
};


struct Transform
{
    float4x4 ndcToWorld;
    float4x4 worldToShadow;
};

ConstantBuffer<Sun> sun : register(b0);
ConstantBuffer<Transform> transform : register(b1);
SamplerState theSampler : register(s0);
Texture2D<float4> tex_albedoRoughness : register(t0);
Texture2D<float2> tex_normal : register(t1);
Texture2D<float> tex_depth : register(t2);
Texture2DArray<float> tex_shadowMaps : register(t3);

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
	
	float shadowFactor = 0.0;
	float depth = tex_depth.Sample(theSampler, coords).x;
	
	float4 worldPos = mul(transform.ndcToWorld, float4(float3(input.texCoord.xy*2.0-1.0, depth), 1.0));
	worldPos.xyz /= worldPos.w;
	worldPos.w = 1.0;
	float4 shadowPos = mul(transform.worldToShadow, worldPos);
	//shadowPos.xyz /= shadowPos.w;
	shadowPos.xy = shadowPos.xy*0.5 + 0.5;
	shadowPos.y = 1.0-shadowPos.y;
	
	shadowFactor += tex_shadowMaps.Sample(theSampler, float3(shadowPos.xy, 0.0)).x < shadowPos.z ? 1.0 : 0.0;
	
	if (shadowPos.x > 1 || shadowPos.y > 1 || shadowPos.x < 0 || shadowPos.y < 0) {
        shadowFactor = 0;
	}
	
	float diffFactor = max(0.0, dot(normal, sun.dir.xyz))*(1-shadowFactor);
	
	float4 ambientColor = float4(0.12, 0.16, 0.23, 1);//(1.0-sun.color)*0.5;
	
	float4 outColor = tex_albedoRoughness.Sample(theSampler, coords) * lerp(ambientColor, sun.color, diffFactor);


	// crappy tone-mapper
	float a = 1.3;
	float b = 2;
	return a*(b*outColor) / (1 + b*outColor);
}
