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
	float2 texcoord : TEX_COORD0;
};

float2 compareWithNeighbor(int2 tileCorner, int s, int t, float2 result)
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

PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	return result;
}

float2 PSMain(PS_Input input) : SV_TARGET
{
	int2 tileCorner = int2(input.position.xy);

	float2 result = float2(0.0, 1.0);

	int searchRadius = 2;

	for (int x = -searchRadius; x <= searchRadius; ++x)
	{
		for (int y = -searchRadius; y <= searchRadius; ++y)
		{
			result = compareWithNeighbor(tileCorner, x, y, result);
		}
	}

	return result.xy;
}
