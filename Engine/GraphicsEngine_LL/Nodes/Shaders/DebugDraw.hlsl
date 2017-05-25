/*
 * Debug Draw shader
 */

struct Uniforms
{
	float4x4 vp;
	float4 color;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

struct PS_Input
{
	float4 position : SV_POSITION;
};


PS_Input VSMain(float4 position : POSITION)
{
	PS_Input result;

    result.position = mul(position, uniforms.vp);

	return result;
}


float4 PSMain(PS_Input input) : SV_TARGET
{
	return uniforms.color;
}
