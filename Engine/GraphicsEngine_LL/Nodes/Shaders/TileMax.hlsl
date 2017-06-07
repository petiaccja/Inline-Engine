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

PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	return result;
}

float2 PSMain(PS_Input input) : SV_TARGET
{
	uint2 tileCorner = uint2(input.position.xy) * uniforms.maxMotionBlurRadius;

	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	inputTexSize -= uint3(1,1,0);

	float maxVelSqr = 0.0;
	float2 result = float2(0, 0);

	for (int s = 0; s < uniforms.maxMotionBlurRadius; ++s)
	{
		for (int t = 0; t < uniforms.maxMotionBlurRadius; ++t)
		{
			float2 velocity = undoVelocityBiasScale(inputTex.Load(int3(clamp(tileCorner + int2(s,t), int2(0,0), inputTexSize.xy),0)).xy);

			float lenSqr = dot(velocity, velocity);
			if (maxVelSqr < lenSqr)
			{
				maxVelSqr = lenSqr;
				result = velocity;
			}
		}
	}

	return doVelocityBiasScale(result);
}
