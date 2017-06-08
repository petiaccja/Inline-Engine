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
	float2 texcoord : TEX_COORD0;
};

float2 undoVelocityBiasScale(float2 v)
{
	return v * 2.0 - 1.0;
}

float2 doVelocityBiasScale(float2 v)
{
	return (v + 1.0) * 0.5;
}

float3 compareWithNeighbor(int2 tileCorner, int s, int t, float3 result)
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	inputTexSize -= uint3(1, 1, 0);

	int2 offset = int2(s, t);
	float2 velocity = undoVelocityBiasScale(inputTex.Load(int3(clamp(tileCorner + offset, int2(0, 0), inputTexSize.xy), 0)).xy);

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

PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	return result;
}

float2 PSMain(PS_Input input) : SV_TARGET
{
	uint2 tileCorner = uint2(input.position.xy);

	float3 result = float3(0.5, 0.5, 0.0);

	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			result = compareWithNeighbor(tileCorner, x, y, result);
		}
	}

	return doVelocityBiasScale(result.xy);
}
