/*
* Motion Blur Neighbor Max shader
* Input: RG8 max tile motion vector texture
* Output: max motion vector in tile neighborhood
*/

struct Uniforms
{
};

//ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

float2 UndoVelocityBiasScale(float2 v)
{
	return v * 2.0 - 1.0;
}

float2 DoVelocityBiasScale(float2 v)
{
	return (v + 1.0) * 0.5;
}

float3 CompareWithNeighbor(int2 tileCorner, int s, int t, float3 result)
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	int2 texSize = int3(inputTexSize).xy - int2(1, 1);


	int2 offset = int2(s, t);
	float2 velocity = UndoVelocityBiasScale(inputTex.Load(int3(clamp(tileCorner + offset, int2(0, 0), texSize), 0)).xy);

	float lenSqr = dot(velocity, velocity);

	if (result.z < lenSqr)
	{
		float displacement = abs(float(offset.x)) + abs(float(offset.y));
		float2 orientation = sign(float2(offset) * velocity);
		float distance = float(orientation.x + orientation.y);

		if (abs(distance) == displacement)
		{
			result.xy = velocity;
			result.z = lenSqr;
		}
	}
	return result;
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
	int2 tileCorner = int2(input.position.xy);

	float3 result = float3(0.5, 0.5, 0.0);

	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			result = CompareWithNeighbor(tileCorner, x, y, result);
		}
	}

	return DoVelocityBiasScale(result.xy);
}
