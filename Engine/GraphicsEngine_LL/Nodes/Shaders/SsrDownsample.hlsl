/*
* Ssr downsample shader
* Input: HDR color texture
* Output: Half res texture texture
*/

Texture2D inputTex : register(t0); //HDR texture
SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);

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
	//bilinear sample
	return inputTex.Sample(samp1, input.texcoord);
}
