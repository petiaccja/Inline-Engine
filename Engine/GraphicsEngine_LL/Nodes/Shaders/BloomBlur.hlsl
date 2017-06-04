/*
* Bloom blur vertical shader
* Input: HDR color texture
* Output: Blurred texture
*/

struct Uniforms
{
	float2 direction;
	float kernelScale;
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

float4 PSMain(PS_Input input) : SV_TARGET
{
	float weights[5] =
	{
		0.0702702703, 0.3162162162, 0.2270270270, 0.3162162162, 0.0702702703
	};

	float offsets[5] =
	{
		-3.2307692308, -1.3846153846, 0.0, 1.3846153846, 3.2307692308
	};

	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	float2 direction = uniforms.direction / inputTexSize.xy * uniforms.kernelScale;

	float4 result = float4(0, 0, 0, 0);

	for (int c = 0; c < 5; c++)
	{
		result += inputTex.Sample(samp0, input.texcoord + offsets[c] * direction) * weights[c];
	}

	return result;
}
