/*
* DOF Main shader
* Input: HDR color texture
* Input: depth texture
* Input: neighborhood max coc, closest depth
* Output: blurred HDR color texture
*/

struct Uniforms
{
	float maxBlurDiameter;
	float tileSize;
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

float bokehShape(float2 center, float2 uv, float radius)
{
	float2 pos = center;
	float radiusSqr = radius * radius;

	float2 diff = pos - uv;
	float diffSqr = dot(diff, diff);

	if (diffSqr <= radiusSqr)
	{
		float linearValue = (radiusSqr - diffSqr) / radiusSqr;
		float cosValue = cos(linearValue);
		float cosSqr = cosValue * cosValue;
		float cos4 = cosSqr * cosSqr;
		if ((1.0 - linearValue)>0.8)
		{
			float invLinear = linearValue / 0.2;
			return invLinear * invLinear;
		}
		else
		{
			return cos4 / 0.8;
		}
	}
	else
	{
		return 0.0;
	}
}

/* McGuire2012 OIT method to blend samples
//c: color (premultiplied)
//a: coverage (alpha)
//z: depth
//w: a * max(0.01, 1000 * (1 - z)^3)
clear accumTexture to vec4(0), revealageTexture to float(1)
bindFramebuffer(accumTexture, revealageTexture);
glDepthMask(GL_FALSE);
glEnable(GL_BLEND);
glBlendFunci(0, GL_ONE, GL_ONE); //rRGBA = sRGBA + dRGBA;
glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA); //rRGBA = dRGBA*(1-sRGBA);
bindFragmentShader("...
gl_FragData[0] = vec4(Ci, ai) * w(zi, ai);
gl_FragData[1] = vec4(ai);
...}");
drawTransparentSurfaces();
unbindFramebuffer();

glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA); //rRGBA = sRGBA*(1-sA) + dRGBA*sA;
bindFragmentShader("...
vec4 accum = texelFetch(accumTexture, ivec2(gl_FragCoord.xy), 0);
float r = texelFetch(revealageTexture, ivec2(gl_FragCoord.xy), 0).r;
gl_FragColor = vec4(accum.rgb / clamp(accum.a, 1e-4, 5e4), r);
...}", accumTexture, revealageTexture);
*/

float weight(float2 uv, float alpha)
{
	float depth = depthTex.Sample(samp0, uv).x;
	float oneMinusDepth = 1 - depth;
	float oneMinusDepth3 = oneMinusDepth * oneMinusDepth * oneMinusDepth;
	return alpha * max(0.01, 3*1000 * oneMinusDepth3);
}

static const int numBins = 16;
int binFunc(float linearDepth)
{
	float d2 = linearDepth * linearDepth;
	float d4 = d2 * d2;
	float d6 = d4 * d2;
	return int(log2(d6)*0.5) - 1;
}

float binFuncContinuous(float linearDepth)
{
	float d2 = linearDepth * linearDepth;
	float d4 = d2 * d2;
	float d6 = d4 * d2;
	return log2(d6)*0.5 - 1;
}

float4 groundTruth(float2 uv, float2 resolution)
{
	float2 pixStep = 1.0 / resolution;

	float4 result = float4(0, 0, 0, 0);
	float samples = 0;
	float revealage = 1;

	float pi = 3.14159265;

	float4 colorBin[numBins];
	float revealageBin[numBins];

	for (int c = 0; c < numBins; ++c)
	{
		colorBin[c] = float4(0, 0, 0, 0);
		revealageBin[c] = 1;
	}

	for (float y = -uniforms.maxBlurDiameter * 0.5; y <= uniforms.maxBlurDiameter * 0.5; ++y)
	{
		for (float x = -uniforms.maxBlurDiameter * 0.5; x <= uniforms.maxBlurDiameter * 0.5; ++x)
		{
			float tapDist = length(float2(x, y));
			
			if (tapDist > uniforms.maxBlurDiameter * 0.5)
			{
				continue; 
			}

			float2 sampleUV = uv + float2(x, y) * pixStep;
			float4 data = inputTex.Sample(samp0, sampleUV);
			float tapCoc = max(data.w, 1.0);

			if (tapCoc * 0.5 > tapDist)
			{
				float depth = linearize_depth(depthTex.Sample(samp0, sampleUV), 0.1, 100);
				int bin = clamp(binFunc(depth), 0, numBins-1);

				float alpha = (4 * pi) / (tapCoc*tapCoc*pi*0.25);
				//result += float4(data.xyz * bokehShape(uv*resolution + float2(x, y), uv*resolution, tapCoc * 0.5) * alpha, alpha) * weight(sampleUV, alpha);
				//result += float4(data.xyz * alpha, alpha) * weight(sampleUV, alpha);
				result += float4(data.xyz * alpha, alpha);// *weight(sampleUV, alpha);

				colorBin[bin] += float4(data.xyz * alpha, alpha);

				samples++;
				if (revealage > 0.001) //float underflow fix
				{
					revealage *= (1.0 - alpha);
				}

				if (revealageBin[bin] > 0.001) //float underflow fix
				{
					revealageBin[bin] *= (1.0 - alpha);
				}
			}
		}
	}

	float4 blendResult = float4(0, 0, 0, 0);
	for (int d = numBins - 1; d >= 0; --d)
	{
		//rRGBA = sRGBA*(1-sA) + dRGBA*sA;
		float4 source = float4(colorBin[d].rgb / clamp(colorBin[d].a, 1e-4, 5e4), revealageBin[d]);
		blendResult = source * saturate(1 - source.a) + blendResult * saturate(source.a);
	}

	//saturate for float overflow fix
	//return float4((result.rgb / clamp(result.a, 1e-4, 5e4)) * saturate(1.0 - revealage), 1);

	return float4(result.rgb / clamp(result.a, 1e-4, 5e4), 1);
	//return float4(result.rgb, 1);

	//int bin = 15;
	//float4 source = float4(colorBin[bin].rgb / clamp(colorBin[bin].a, 1e-4, 5e4), revealageBin[bin]);
	//return source * saturate(1 - source.a);

	float depth = linearize_depth(depthTex.Sample(samp0, uv), 0.1, 100);
	//float bin = binFuncContinuous(depth);
	int bin = binFunc(depth);
	//return bin / 16.0;

	return blendResult;
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

	float tileDistX = float((currPixelPos.x % (int)uniforms.tileSize - (int)uniforms.tileSize / 2)) / (uniforms.tileSize*0.5);
	float tileDistY = float((currPixelPos.y % (int)uniforms.tileSize - (int)uniforms.tileSize / 2)) / (uniforms.tileSize*0.5);

	//return float4(abs(tileDistX), abs(tileDistY), 0, 1);
	//return float4(rand(input.texcoord) < abs(tileDistX), rand(input.texcoord+1) < abs(tileDistY), 0, 1);
	//return float4(sign(tileDistX) * (rand(input.texcoord) < abs(tileDistX)) * 0.5 + 0.5, sign(tileDistY) * (rand(input.texcoord+1) < abs(tileDistY)) * 0.5 + 0.5, 0, 1);
	//return float4(sign(tileDistX) * 0.5 + 0.5, sign(tileDistY) * 0.5 + 0.5, 0, 1);

	//float2 tileData = neighborhoodMaxTex.Load(int3(currPixelPos / (int)uniforms.maxBlurDiameter, 0)).xy;
	int2 offset = int2(sign(tileDistX) * float(rand(input.texcoord) < (abs(tileDistX) * 0.5)), sign(tileDistY) * float(rand(input.texcoord + 1) < (abs(tileDistY) * 0.5)));
	//int2 offset = int2(rand(input.texcoord) < 0.5, rand(input.texcoord + 1) < 0.5);
	float2 tileData = neighborhoodMaxTex.Load(clamp(int3(currPixelPos/(int)uniforms.tileSize + offset, 0), int3(0,0,0), int3(inputTexSize.xy / (int)uniforms.tileSize - 1, 0))).xy;

	/*float2 currentTileData = neighborhoodMaxTex.Load(int3(currPixelPos / (int)uniforms.maxBlurDiameter, 0)).xy;
	float currentTileMaxCoC = currentTileData.x;

	float maxNeighborCoC = currentTileMaxCoC;
	float minNeighborCoC = currentTileMaxCoC;
	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			//float randOffset = rand(input.texcoord) * uniforms.maxBlurDiameter * 0.1;
			//float2 tileData = neighborhoodMaxTex.Load(clamp(int3(currPixelPos / (int)uniforms.maxBlurDiameter + int2(x, y) * randOffset, 0), int3(0, 0, 0), int3(inputTexSize.xy / (int)uniforms.maxBlurDiameter - 1, 0))).xy;
			float2 tileData = neighborhoodMaxTex.Load(clamp(int3(currPixelPos / (int)uniforms.maxBlurDiameter + int2(x, y), 0), int3(0, 0, 0), int3(inputTexSize.xy / (int)uniforms.maxBlurDiameter - 1, 0))).xy;
			maxNeighborCoC = max(maxNeighborCoC, tileData.x);
			minNeighborCoC = min(minNeighborCoC, tileData.x);
		}
	}

	float tileMaxCoc = currentTileMaxCoC;
	if (rand(input.texcoord) < (max(maxNeighborCoC - currentTileMaxCoC, currentTileMaxCoC - minNeighborCoC)) / uniforms.maxBlurDiameter)
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

	//return float4(tileMaxCoc, tileMaxCoc, tileMaxCoc, tileMaxCoc)/uniforms.maxBlurDiameter;
	//return float4(tileClosestDepth, tileClosestDepth, tileClosestDepth, tileClosestDepth)*0.01;
	//return currentColor;
	//return float4(currentColor.w, currentColor.w, currentColor.w, currentColor.w) / uniforms.maxBlurDiameter;
	//return float4(currentDepth, currentDepth, currentDepth, currentDepth)*0.01;
}
