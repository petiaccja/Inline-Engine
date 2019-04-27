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
	float2 texCoord : TEX_COORD0;
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

	float centerDepth = depthTex.Sample(samp0, input.texCoord);

	float4 result = float4(0, 0, 0, 0);

	for (int c = 0; c < 5; c++)
	{
		float currentDepth = depthTex.Sample(samp0, input.texCoord + offsets[c] * direction);
		float diff = abs(centerDepth - currentDepth);
		float bilateralWeight = exp(-12.0*diff);
		result += ssaoTex.Sample(samp1, input.texCoord + offsets[c] * direction) * weights[c] * bilateralWeight;
	}

	return result;
}
