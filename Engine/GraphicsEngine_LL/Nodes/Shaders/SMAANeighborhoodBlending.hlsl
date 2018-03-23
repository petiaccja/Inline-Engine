/*
* SMAA neighborhood blending shader
* Input0: LDR color texture
* Input1: Blending weights texture
* Output: Antialiased texture
*/

struct Uniforms
{
	float4 pixelSize;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //LDR texture
Texture2D blendingWeightsTex : register(t3); //Blending weights texture
SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD0;
	float4 offset : TEX_COORD1;
};

#define SMAA_HLSL_4_1
#define SMAA_PRESET_ULTRA 1
#define SMAA_RT_METRICS uniforms.pixelSize

#include "SMAA.hlsl"


PS_Input VSMain(uint vertexId : SV_VertexID)
{
	// Triangle strip based on vertex id
	// 3-----2
	// |   / |
	// | /   |
	// 1-----0
	// 0: (1, 0)
	// 1: (0, 0)
	// 2: (1, 1)
	// 3: (0, 1)
    PS_Input output;

    output.texCoord.x = (vertexId & 1) ^ 1; // 1 if bit0 is 0.
    output.texCoord.y = vertexId >> 1; // 1 if bit1 is 1.

    float2 posL = output.texCoord.xy * 2.0f - float2(1, 1);
    output.position = float4(posL, 0.5f, 1.0f);
    output.texCoord.y = 1.f - output.texCoord.y;

    SMAANeighborhoodBlendingVS(output.texCoord, output.offset);

    return output;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	return SMAANeighborhoodBlendingPS(input.texCoord, input.offset, inputTex, blendingWeightsTex);
}
