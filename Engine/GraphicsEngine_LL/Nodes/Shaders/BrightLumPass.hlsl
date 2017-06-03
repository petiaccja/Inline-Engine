/*
* Bloom bright pass and luminance calculation shader
* Input: lightmvp texture
* Output: shadow map for the specific cascade
*/

struct Uniforms
{
	float bright_pass_threshold;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
};


PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	return result;
}

struct PS_OUTPUT
{
	float4 brightPassResult: SV_Target0;
	float luminance: SV_Target1;
};

PS_OUTPUT PSMain(PS_Input input) : SV_TARGET
{
	PS_OUTPUT outputData;

	float4 inputData = inputTex.Sample(samp0, input.texcoord);
	float4 threshold = float4(uniforms.bright_pass_threshold, uniforms.bright_pass_threshold, uniforms.bright_pass_threshold, uniforms.bright_pass_threshold);

	outputData.brightPassResult = max(inputData - threshold, float4(0, 0, 0, 0));

	const float3 lum = float3(0.2126, 0.7152, 0.0722);
	const float delta = 0.0001;
	outputData.luminance = log(dot(inputData.xyz, lum) + delta);

	return outputData;
}
