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

float4 circle_filter(float2 uv, float2 dist, float maxCocScaled, float currentCocScaled, const int taps)
{
	const float pi = 3.14159265;
	float ftaps = 1.0 / float(taps);
	float4 result = float4(0,0,0,0);
	for (int c = 0; c < taps; ++c)
	{
		float xx = cos(2.0 * pi * float(c) * ftaps);
		float yy = sin(2.0 * pi * float(c) * ftaps);
		float4 data = inputTex.Sample(samp0, uv + float2(xx, yy)* dist);
		float compareCoc = ((c & 1) == 1) ? currentCocScaled : maxCocScaled; 
		//TODO is this correct?
		if (data.w > compareCoc)
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
		+ circle_filter(uv, 0.333 * dist, 0.333 * coc, 0.333 * center_tap.w, 8)
		+ circle_filter(uv, 0.666 * dist, 0.666 * coc, 0.666 * center_tap.w, 16)
		+ circle_filter(uv, 0.999 * dist, 0.999 * coc, 0.999 * center_tap.w, 24);

	return result / result.w;
}

float4 filterFuncTier2(float2 uv, float2 resolution, float4 center_tap, float coc)
{
	float2 dist = coc / resolution; //19

	float4 result = float4(center_tap.xyz, 1.0)
		+ circle_filter(uv, 0.5 * dist, 0.5 * coc, 0.5 * center_tap.w, 8)
		+ circle_filter(uv, 0.999 * dist, 0.999 * coc, 0.999 * center_tap.w, 16);

	return result / result.w;
}

float4 filterFuncTier3(float2 uv, float2 resolution, float4 center_tap, float coc)
{
	float2 dist = coc / resolution; //9

	float4 result = float4(center_tap.xyz, 1.0)
		+ circle_filter(uv, 0.999 * dist, 0.999 * coc, 0.999 * center_tap.w, 8);

	return result / result.w;
}

float rand(float2 co) {
	return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float4 groundTruth(float2 uv, float2 resolution)
{
	float2 pixStep = 1.0 / resolution;

	float4 result = float4(0, 0, 0, 0);

	for (float y = -uniforms.maxBlurRadius; y <= uniforms.maxBlurRadius; ++y)
	{
		for (float x = -uniforms.maxBlurRadius; x <= uniforms.maxBlurRadius; ++x)
		{
			float tapDist = length(float2(x, y));
			
			if (tapDist > uniforms.maxBlurRadius)
			{
				continue;
			}

			float4 data = inputTex.Sample(samp0, uv + float2(x, y) * pixStep);
			float tapCoc = data.w;

			if (tapCoc > tapDist)
			{
				result += float4(data.xyz, 1);
			}
		}
	}

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

	float tileDistX = float((currPixelPos.x % (int)uniforms.maxBlurRadius - (int)uniforms.maxBlurRadius / 2)) / (uniforms.maxBlurRadius*0.5);
	float tileDistY = float((currPixelPos.y % (int)uniforms.maxBlurRadius - (int)uniforms.maxBlurRadius / 2)) / (uniforms.maxBlurRadius*0.5);

	//return float4(abs(tileDistX), abs(tileDistY), 0, 1);
	//return float4(rand(input.texcoord) < abs(tileDistX), rand(input.texcoord+1) < abs(tileDistY), 0, 1);
	//return float4(sign(tileDistX) * (rand(input.texcoord) < abs(tileDistX)) * 0.5 + 0.5, sign(tileDistY) * (rand(input.texcoord+1) < abs(tileDistY)) * 0.5 + 0.5, 0, 1);
	//return float4(sign(tileDistX) * 0.5 + 0.5, sign(tileDistY) * 0.5 + 0.5, 0, 1);

	//float2 tileData = neighborhoodMaxTex.Load(int3(currPixelPos / (int)uniforms.maxBlurRadius, 0)).xy;
	int2 offset = int2(sign(tileDistX) * float(rand(input.texcoord) < (abs(tileDistX) * 0.5)), sign(tileDistY) * float(rand(input.texcoord + 1) < (abs(tileDistY) * 0.5)));
	//int2 offset = int2(rand(input.texcoord) < 0.5, rand(input.texcoord + 1) < 0.5);
	float2 tileData = neighborhoodMaxTex.Load(clamp(int3(currPixelPos/(int)uniforms.maxBlurRadius + offset, 0), int3(0,0,0), int3(inputTexSize.xy / (int)uniforms.maxBlurRadius - 1, 0))).xy;

	/*float2 currentTileData = neighborhoodMaxTex.Load(int3(currPixelPos / (int)uniforms.maxBlurRadius, 0)).xy;
	float currentTileMaxCoC = currentTileData.x;

	float maxNeighborCoC = currentTileMaxCoC;
	float minNeighborCoC = currentTileMaxCoC;
	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			//float randOffset = rand(input.texcoord) * uniforms.maxBlurRadius * 0.1;
			//float2 tileData = neighborhoodMaxTex.Load(clamp(int3(currPixelPos / (int)uniforms.maxBlurRadius + int2(x, y) * randOffset, 0), int3(0, 0, 0), int3(inputTexSize.xy / (int)uniforms.maxBlurRadius - 1, 0))).xy;
			float2 tileData = neighborhoodMaxTex.Load(clamp(int3(currPixelPos / (int)uniforms.maxBlurRadius + int2(x, y), 0), int3(0, 0, 0), int3(inputTexSize.xy / (int)uniforms.maxBlurRadius - 1, 0))).xy;
			maxNeighborCoC = max(maxNeighborCoC, tileData.x);
			minNeighborCoC = min(minNeighborCoC, tileData.x);
		}
	}

	float tileMaxCoc = currentTileMaxCoC;
	if (rand(input.texcoord) < (max(maxNeighborCoC - currentTileMaxCoC, currentTileMaxCoC - minNeighborCoC)) / uniforms.maxBlurRadius)
	//if (rand(input.texcoord) < 0.5)
	{
		tileMaxCoc = maxNeighborCoC;
	}*/

	//float2 tileData = neighborhoodMaxTex.Sample(samp0, input.texcoord).xy;
	float tileMaxCoc = tileData.x;
	//float tileClosestDepth = linearize_depth(tileData.y, 0.1, 100.0);

	float4 result = float4(0, 0, 0, 0);

	result = groundTruth(input.texcoord, float2(inputTexSize.xy));

	//result = filterFuncTier1(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc);

	//return float4(rand(input.texcoord), 0, 0, 1);

	/*if (tileMaxCoc > 19.0)
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
	}*/

	return result;

	//return float4(tileMaxCoc, tileMaxCoc, tileMaxCoc, tileMaxCoc)/uniforms.maxBlurRadius;
	//return float4(tileClosestDepth, tileClosestDepth, tileClosestDepth, tileClosestDepth)*0.01;
	//return currentColor;
	//return float4(currentDepth, currentDepth, currentDepth, currentDepth)*0.01;
}
