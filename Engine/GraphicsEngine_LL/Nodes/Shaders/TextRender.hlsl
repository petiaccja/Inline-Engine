/*
* Text render shader
* Input: font texture
* Output: text rendered onto rtv
*/

struct Uniforms
{
	float4x4 trans;
	uint x, y, w, h;
	float4 color;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D fontTex : register(t0); //half size
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
};


PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = mul(position, uniforms.trans);
	result.texcoord = texcoord.xy;

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float textAlpha = fontTex.Load(int3(int2(uniforms.x, uniforms.y) + int2(input.texcoord * float2(uniforms.w, uniforms.h)), 0)).w;
	return float4(uniforms.color.xyz, textAlpha);
}
