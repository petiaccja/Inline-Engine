/*
* Motion DOF Tile Max shader
* Input0: premultiplied hdr color, coc texture
* Input1: depth texture
* Output: max coc, closest depth in tile
*/

struct Uniforms
{
	float maxBlurRadius;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
Texture2D depthTex : register(t1); //HDR texture
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

float2 PSMain(PS_Input input) : SV_TARGET
{
	int2 tileCorner = int2(input.position.xy) * uniforms.maxBlurRadius;

	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	int2 texSize = int3(inputTexSize).xy - int2(1,1);

	float closestDepth = 1.0;
	float maxCoc = 0.0;

	for (int s = 0; s < uniforms.maxBlurRadius; ++s)
	{
		for (int t = 0; t < uniforms.maxBlurRadius; ++t)
		{
			float coc = inputTex.Load(int3(clamp(tileCorner + int2(s,t), int2(0,0), texSize),0)).w;
			float depth = depthTex.Load(int3(clamp(tileCorner + int2(s, t), int2(0, 0), texSize), 0)).x;
			
			maxCoc = max(maxCoc, coc);
			closestDepth = min(closestDepth, depth);
		}
	}

	return float2(maxCoc, closestDepth);
}
