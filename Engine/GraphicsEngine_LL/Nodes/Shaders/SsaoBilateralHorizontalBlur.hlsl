/*
* Bloom blur vertical shader
* Input: HDR color texture
* Output: Blurred texture
*/

struct Uniforms
{
	float4x4 invVP, oldVP;
	float4 farPlaneData0, farPlaneData1;
	float nearPlane, farPlane, wsRadius, scaleFactor;
	float temporalIndex;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D depthTex : register(t0);
Texture2D ssaoTex : register(t1);
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
	float weights[5] =
	{
		0.0702702703, 0.3162162162, 0.2270270270, 0.3162162162, 0.0702702703
	};

	float offsets[5] =
	{
		-3.2307692308, -1.3846153846, 0.0, 1.3846153846, 3.2307692308
	};

	uint3 inputTexSize;
	depthTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	float2 direction = float2(1.0, 0.0) / inputTexSize.xy;

	float centerDepth = depthTex.Sample(samp0, input.texcoord);

	float4 result = float4(0, 0, 0, 0);

	for (int c = 0; c < 5; c++)
	{
		float currentDepth = depthTex.Sample(samp0, input.texcoord + offsets[c] * direction);
		float diff = abs(centerDepth - currentDepth);
		float bilateralWeight = exp(-12.0*diff);
		result += ssaoTex.Sample(samp1, input.texcoord + offsets[c] * direction) * weights[c] * bilateralWeight;
	}

	return result;
}
