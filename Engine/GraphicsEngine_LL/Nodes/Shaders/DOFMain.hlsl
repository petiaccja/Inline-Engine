/*
* DOF Main shader
* Input: HDR color texture
* Input: depth texture
* Input: neighborhood max coc, closest depth
* Output: blurred HDR color texture
*/

struct Uniforms
{
	float maxBlurRadius;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
Texture2D depthTex : register(t1); //
Texture2D neighborhoodMaxTex : register(t2); //
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
};

//warning: result [0...far]
float linearize_depth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vs_zrecon = B / (zndc - A);

	//range: [0...far]
	return vs_zrecon;
};

float4 circle_filter(float2 uv, float2 dist, float cocScaled, const int taps)
{
	const float pi = 3.14159265;
	float ftaps = 1.0 / float(taps);
	float4 result = float4(0,0,0,0);
	for (int c = 0; c < taps; ++c)
	{
		float xx = cos(2.0 * pi * float(c) * ftaps);
		float yy = sin(2.0 * pi * float(c) * ftaps);
		float4 data = inputTex.Sample(samp0, uv + float2(xx, yy)* dist);
		if (data.w > cocScaled)
		{
			result.xyz += data.xyz;
			result.w++;
		}
	}
	return result;
}

float4 filterFuncTier1(float2 uv, float2 resolution, float4 center_tap, float coc)
{
	float2 dist = coc / resolution; //28

	float4 result = float4(center_tap.xyz, 1.0)
		+ circle_filter(uv, 0.333 * dist, 0.333 * coc, 8)
		+ circle_filter(uv, 0.666 * dist, 0.666 * coc, 16)
		+ circle_filter(uv, 0.999 * dist, 0.999 * coc, 24);

	return result / result.w;
}

float4 filterFuncTier2(float2 uv, float2 resolution, float4 center_tap, float coc)
{
	float2 dist = coc / resolution; //19

	float4 result = float4(center_tap.xyz, 1.0)
		+ circle_filter(uv, 0.5 * dist, 0.5 * coc, 8)
		+ circle_filter(uv, 0.999 * dist, 0.999 * coc, 16);

	return result / result.w;
}

float4 filterFuncTier3(float2 uv, float2 resolution, float4 center_tap, float coc)
{
	float2 dist = coc / resolution; //9

	float4 result = float4(center_tap.xyz, 1.0)
		+ circle_filter(uv, 0.999 * dist, 0.999 * coc, 8);

	return result / result.w;
}


PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	int2 currPixelPos = int2(input.position.xy);
	float4 currentColor = inputTex.Sample(samp0, input.texcoord);
	float currentDepth = linearize_depth(depthTex.Sample(samp0, input.texcoord), 0.1, 100.0);
	float2 tileData = neighborhoodMaxTex.Load(int3(currPixelPos/(int)uniforms.maxBlurRadius, 0)).xy;
	float tileMaxCoc = tileData.x;
	float tileClosestDepth = linearize_depth(tileData.y, 0.1, 100.0);

	float4 result = float4(0, 0, 0, 0);

	if (tileMaxCoc > 19.0)
	{
		result = filterFuncTier1(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc);
	}
	else if (tileMaxCoc > 9.0)
	{
		result = filterFuncTier2(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc);
	}
	else
	{
		result = filterFuncTier3(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc);
	}

	return result;

	//return float4(tileMaxCoc, tileMaxCoc, tileMaxCoc, tileMaxCoc)/uniforms.maxBlurRadius;
	//return float4(tileClosestDepth, tileClosestDepth, tileClosestDepth, tileClosestDepth)*0.01;
	//return currentColor;
	//return float4(currentDepth, currentDepth, currentDepth, currentDepth)*0.01;
}
