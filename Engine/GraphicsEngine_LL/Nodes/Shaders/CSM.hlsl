/*
* Cascaded shadow mapping shader
* Input: lightmvp texture
* Output: shadow map for the specific cascade
*/

Texture2D inputTex : register(t0); //lightMVP texture

struct Uniforms
{
	float4x4 model;
	uint cascadeIDX;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

struct PS_Input
{
	float4 position : SV_POSITION;
};


PS_Input VSMain(float4 position : POSITION)
{
	PS_Input result;

	float4x4 light_mvp;
	for (int d = 0; d < 4; ++d)
	{
		light_mvp[d] = inputTex.Load(int3(uniforms.cascadeIDX * 4 + d, 0, 0));
	}

    result.position = mul(mul(light_mvp, uniforms.model), position);

	return result;
}


void PSMain(PS_Input input)
{
}
