/*
* Bloom add shader
* Input0: half size HDR color texture
* Input1: normal size HDR color texture
* Output: the avg of the textures
*/

struct Uniforms
{
};

//ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D input0Tex : register(t0); //half size
Texture2D input1Tex : register(t1); //normal size
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

float4 PSMain(PS_Input input) : SV_TARGET
{
	return 0.5 * (input0Tex.Sample(samp0, input.texcoord) + input1Tex.Sample(samp0, input.texcoord));
}
