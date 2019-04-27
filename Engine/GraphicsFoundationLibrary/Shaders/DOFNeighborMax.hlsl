/*
* Motion DOF Neighbor Max shader
* Input: RG16F max coc, closest depth texture
* Output: max coc, closest depth in 3x3 neighborhood
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

float2 CompareWithNeighbor(int2 tileCorner, int s, int t, float2 result)
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	int2 texSize = int3(inputTexSize).xy - int2(1, 1);


	int2 offset = int2(s, t);
	float2 data = inputTex.Load(int3(clamp(tileCorner + offset, int2(0, 0), texSize), 0)).xy;

	if (result.x < data.x)
	{
		result.x = data.x;
	}

	if (result.y > data.y)
	{
		result.y = data.y;
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

	float2 result = float2(0.0, 1.0);

	int searchRadius = 1;

	for (int x = -searchRadius; x <= searchRadius; ++x)
	{
		for (int y = -searchRadius; y <= searchRadius; ++y)
		{
			result = CompareWithNeighbor(tileCorner, x, y, result);
		}
	}

	return result.xy;
}
