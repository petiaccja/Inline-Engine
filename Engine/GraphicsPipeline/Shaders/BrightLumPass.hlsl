/*
* Bloom bright pass and luminance calculation shader
* Input: lightmvp texture
* Output: shadow map for the specific cascade
*/

struct Uniforms
{
	float brightPassThreshold;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
SamplerState samp0 : register(s0);

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

struct PS_OUTPUT
{
	float4 brightPassResult: SV_Target0;
	float luminance: SV_Target1;
};

PS_OUTPUT PSMain(PS_Input input) : SV_TARGET
{
	PS_OUTPUT outputData;

	float4 inputData = max(inputTex.Sample(samp0, input.texCoord), float4(0,0,0,0));
	float4 threshold = float4(uniforms.brightPassThreshold, uniforms.brightPassThreshold, uniforms.brightPassThreshold, uniforms.brightPassThreshold);

	outputData.brightPassResult = max(inputData - threshold, float4(0, 0, 0, 0));

	const float3 lum = float3(0.2126, 0.7152, 0.0722);
	const float delta = 0.0001;
	outputData.luminance = log(dot(inputData.xyz, lum) + delta);

	return outputData;
}
