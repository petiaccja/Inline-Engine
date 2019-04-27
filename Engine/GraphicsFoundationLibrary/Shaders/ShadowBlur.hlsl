/*
* Shadow blur shader
* Input: layered penumbra texture, layered shadow texture
* Output: blurred shadows
*/

#define LOW 1
#define MEDIUM 2
#define HIGH 3
#define ULTRA 4

#define QUALITY ULTRA

struct Uniforms
{
	float4x4 invV;
	float4 farPlaneData0, farPlaneData1;
	float lightSize, nearPlane, farPlane, dummy;
	float4 vsLightPos;
	float2 direction;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex0 : register(t0); //depth texture
//TODO normal tex
Texture2DArray<float> inputTex1 : register(t1); //csm minfilter map
TextureCube<float> inputTex2 : register(t2); //cube minfilter map
Texture2D inputTex3 : register(t3); //layered penumbra texture
Texture2D inputTex4 : register(t4); //layered shadow texture
SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);

#include "ShadowBlurSample.hlsl"

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

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

    return output;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	uint3 inputTexSize;
	inputTex0.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	
	float depth = inputTex0.Sample(samp0, input.texCoord).x;
	
	if(depth > 0.999)
	{
		discard;
	}
	
	float linearDepth = LinearizeDepth(depth, uniforms.nearPlane, uniforms.farPlane);
	
	float3 farPlaneLL = uniforms.farPlaneData0.xyz;
	float3 farPlaneUR = float3(uniforms.farPlaneData0.w, uniforms.farPlaneData1.xy);

	float2 uv = float2(input.texCoord.x, 1 - input.texCoord.y);
	float3 vsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, uv) / uniforms.farPlane, 1.0) * linearDepth;
	
	//TODO replace with proper normals
	float3 vsDepthNormal = -normalize(cross(ddy(vsPos.xyz), ddx(vsPos.xyz)));
	
	float4 penumbra = float4(0.0, 0.0, 0.0, 0.0);
	//TODO
	//hacked here to hide cascade transitions
	for(int y = -1; y <= 1; ++y)
	{
		for(int x = -1; x <= 1; ++x)
		{
			penumbra += inputTex3.Sample(samp0, input.texCoord, int2(x,y) * 3) / 9.0;
		}
	}
	float4 hardShadow = inputTex4.Sample(samp0, input.texCoord);
	
	const float anisoThreshold = 0.25; //TODO make it uniform
	float2 stepSize = GetStepSize( uniforms.direction, vsDepthNormal, linearDepth, anisoThreshold ) / uniforms.farPlane * 3.0;
	
	float4 blurredResultLayers = float4(0.0, 0.0, 0.0, 0.0);
	
	blurredResultLayers.x = Blur( stepSize, input.texCoord, hardShadow.x, depth, penumbra.x, 0 );
	blurredResultLayers.y = Blur( stepSize, input.texCoord, hardShadow.y, depth, penumbra.y, 1 );
	blurredResultLayers.z = Blur( stepSize, input.texCoord, hardShadow.z, depth, penumbra.z, 2 );
	blurredResultLayers.w = Blur( stepSize, input.texCoord, hardShadow.w, depth, penumbra.w, 3 );
	
	return blurredResultLayers;
}
