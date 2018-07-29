/*
* Motion Blur Tile Max shader
* Input: RG8 motion vector texture
* Output: max motion vector in max blur radius
*/

struct Uniforms
{
	float maxMotionBlurRadius;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD0;
};

float2 UndoVelocityBiasScale(float2 v)
{
	return v * 2.0 - 1.0;
}

float2 DoVelocityBiasScale(float2 v)
{
	return (v + 1.0) * 0.5;
}

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

float2 PSMain(PS_Input input) : SV_TARGET
{
	int2 tileCorner = int2(input.position.xy) * uniforms.maxMotionBlurRadius;

	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	int2 texSize = int3(inputTexSize).xy - int2(1,1);

	float maxVelSqr = 0.0;
	float2 result = float2(0, 0);

	for (int s = 0; s < uniforms.maxMotionBlurRadius; ++s)
	{
		for (int t = 0; t < uniforms.maxMotionBlurRadius; ++t)
		{
			float2 velocity = UndoVelocityBiasScale(inputTex.Load(int3(clamp(tileCorner + int2(s,t), int2(0,0), texSize),0)).xy);

			float lenSqr = dot(velocity, velocity);
			if (maxVelSqr < lenSqr)
			{
				maxVelSqr = lenSqr;
				result = velocity;
			}
		}
	}

	return DoVelocityBiasScale(result);
}
