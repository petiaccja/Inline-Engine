/*
* Shadow mapping shader
* Input: light MVP matrix
* Output: shadow map
*/

struct Uniforms
{
	float4x4 mvp;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

struct PS_Input
{
	float4 position : SV_POSITION;
};


PS_Input VSMain(float4 position : POSITION)
{
	PS_Input result;

    result.position = mul(position, uniforms.mvp);

	return result;
}


void PSMain(PS_Input input)
{
}
