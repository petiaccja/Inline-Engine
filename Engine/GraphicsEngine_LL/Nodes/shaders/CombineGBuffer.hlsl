
static const int CASCADE_COUNT = 5;

static const float3 cascadeTint[CASCADE_COUNT] = {
	float3(0.3, .9, 0.3),
	float3(0.3, 0.4, .9),
	float3(.9, 0.3, 0.3),
	float3(.9, .9, 0.3),
	float3(.9, 0.3, 0.3)
};


struct Sun
{
	float4 dir; // in view space
	float4 color;
};

struct CascadeBoudaries
{
	float farDepth[CASCADE_COUNT-1];
};

struct Transform
{
    float4x4 ndcToWorld;
    float4x4 worldToShadow[CASCADE_COUNT];
};

ConstantBuffer<Sun> sun : register(b0);
ConstantBuffer<CascadeBoudaries> cascadeBoundaries : register(b1);
ConstantBuffer<Transform> transform : register(b2);

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
	float2 coords = {input.texCoord.x, 1.0-input.texCoord.y};
	
	float2 nSmpl = tex_normal.Sample(theSampler, coords).xy;
	float3 normal = float3(nSmpl.xy, sqrt(1.0 - nSmpl.x*nSmpl.x - nSmpl.y*nSmpl.y));
	
	float shadowFactor = 0.0;
	float depth = tex_depth.Sample(theSampler, coords).x;
	
	float4 worldPos = mul(transform.ndcToWorld, float4(float3(input.texCoord.xy*2.0-1.0, depth), 1.0));
	worldPos.xyz /= worldPos.w;
	worldPos.w = 1.0;
	
	int selectedCascade = CASCADE_COUNT-1;
	[unroll] for (int i = CASCADE_COUNT-2; i >= 0; i--) {
		selectedCascade =
			(depth < cascadeBoundaries.farDepth[i]) ? i : selectedCascade;
	}
	
	float4 shadowPos = mul(transform.worldToShadow[selectedCascade], worldPos);
	shadowPos.xy = shadowPos.xy*0.5 + 0.5;
	shadowPos.y = 1.0-shadowPos.y;
	
	shadowFactor +=
		tex_shadowMaps.Sample(theSampler, float3(shadowPos.xy, selectedCascade)).x < shadowPos.z ? 1.0 : 0.0;
	
	float diffFactor = max(0.0, dot(normal, sun.dir.xyz))*(1-shadowFactor);
	
	float4 ambientColor = float4(0.12, 0.16, 0.23, 1);//(1.0-sun.color)*0.5;
	
	float4 outColor = tex_albedoRoughness.Sample(theSampler, coords) * lerp(ambientColor, sun.color, diffFactor);


	// crappy tone-mapper
	float a = 1.3;
	float b = 2;
	return a*(b*outColor) / (1 + b*outColor);
}
