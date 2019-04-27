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
SamplerState samp1 : register(s1);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

//warning: result [0...far]
float LinearizeDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vsZrecon = B / (zndc - A);

	//range: [0...far]
	return vsZrecon;
};

//warning: result [0...1]
float ToDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	
	float zndc = B / depth + A;
	return zndc;
}

float Rand(float2 co) {
	return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float BokehShape(float2 center, float2 uv, float radius)
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
			return lerp(0.35, 1.0, cos4 / 0.8);
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

float Weight(float2 uv, float alpha)
{
	float subjectDistance = ToDepth(0.3, 0.1, 100); //0.667
	float depth = depthTex.Sample(samp0, uv).x;
	float toSubjectDist = abs(depthTex.Sample(samp0, uv).x - subjectDistance);

	if (depth > subjectDistance)
	{
		float dist = toSubjectDist;
		float oneMinusDepth = 1 - dist;
		float oneMinusDepth3 = oneMinusDepth * oneMinusDepth * oneMinusDepth;
		return alpha * max(0.01, 200 * oneMinusDepth3);
	}
	else
	{
		float dist = depth;
		float oneMinusDepth = 1 - dist;
		float oneMinusDepth3 = oneMinusDepth * oneMinusDepth * oneMinusDepth;
		return alpha * max(0.01, 200 * oneMinusDepth3);
	}
}

float4 CircleFilter(float2 uv, float dist, float2 resolution, const int taps, inout float4 result, inout float revealage)
{
	const float pi = 3.14159265;
	float ftaps = 1.0 / float(taps);
	float2 pixelSize = 1.0 / resolution;
	
	//result = float4(0,0,0,0);
	//revealage = 1;
	
	for (int c = 0; c < taps; ++c)
	{
		float xx = (cos(2.0 * pi * float(c) * ftaps + 0.464)) * dist;
		float yy = (sin(2.0 * pi * float(c) * ftaps + 0.464)) * dist;

		float2 sampleUV = uv + float2(xx, yy) * pixelSize;
		float tapDistSqr = dot(float2(xx, yy), float2(xx, yy));

		float4 data = inputTex.Sample(samp0, sampleUV);
		float tapCoc = max(data.w, 1.0);

		//float bokeh = bokehShape(uv*resolution + float2(xx, yy), uv*resolution, tapCoc * 0.5);

		if (tapCoc > 15)
		{
			//data.xyz *= bokeh;
		}
		
		if (tapCoc * tapCoc * 0.25 > tapDistSqr)
		{
			float alpha = (4 * pi) / (tapCoc*tapCoc*pi*0.25);
			//float alpha = (pi) / (tapCoc*tapCoc*pi*0.25);

			result += float4(data.xyz * alpha, alpha) * min(Weight(sampleUV, alpha), 1.0);
			
			if (revealage > 0.001) //float underflow fix
			{
				revealage *= (1.0 - alpha);
			}
		}
	}
	return result;
}

float4 FilterFunc(float2 uv, float2 resolution, float4 centerTap, float coc, int rings)
{
	const float pi = 3.14159265;

	float dist = coc * 0.5;

	float centerCoc = max(centerTap.w, 1.0);
	float centerAlpha = (4 * pi) / (centerCoc*centerCoc*pi*0.25);
	//float center_alpha = (pi) / (center_coc*center_coc*pi*0.25);

	float4 result = float4(centerTap.xyz * centerAlpha, centerAlpha) * min(Weight(uv, centerAlpha), 1.0);
	float revealage = (1 - centerAlpha);

	for (int d = 1; d <= rings; ++d)
	{
		CircleFilter(uv, float(d) / float(rings) * dist, resolution, 8 * d, result, revealage);
	}

	return float4((result.rgb / clamp(result.a, 1e-4, 5e4)) * saturate(1.0 - revealage), 1);
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

float4 PSMain(PS_Input input) : SV_TARGET
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	int2 currPixelPos = int2(input.position.xy);
	float4 currentColor = inputTex.Sample(samp0, input.texCoord);
	float currentDepth = LinearizeDepth(depthTex.Sample(samp0, input.texCoord), 0.1, 100.0);
	return currentColor;

	float tileDistX = float((currPixelPos.x % (int)uniforms.tileSize - (int)uniforms.tileSize / 2)) / (uniforms.tileSize*0.5);
	float tileDistY = float((currPixelPos.y % (int)uniforms.tileSize - (int)uniforms.tileSize / 2)) / (uniforms.tileSize*0.5);

	int2 offset = int2(sign(tileDistX) * float(Rand(input.texCoord) < (abs(tileDistX) * 0.5)), sign(tileDistY) * float(Rand(input.texCoord + 1) < (abs(tileDistY) * 0.5)));
	float2 tileData = neighborhoodMaxTex.Load(clamp(int3(currPixelPos/(int)uniforms.tileSize + offset, 0), int3(0,0,0), int3(inputTexSize.xy / (int)uniforms.tileSize - 1, 0))).xy;
	//float2 tileData = neighborhoodMaxTex.Sample(samp1, input.texCoord + float2(rand(input.texCoord)*2-1, rand(input.texCoord*2)*2-1)*0.05);
	//float2 tileData = neighborhoodMaxTex.Sample(samp1, input.texCoord);

	float tileMaxCoc = tileData.x;

	float4 result = float4(0, 0, 0, 0);
	
	result = FilterFunc(input.texCoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 7);

	/*if (tileMaxCoc >= 29.0)
	{
		result = filterFunc(input.texCoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 7);
	}
	else if (tileMaxCoc >= 25.0)
	{
		result = filterFunc(input.texCoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 6);
	}
	else if (tileMaxCoc >= 21.0)
	{
		result = filterFunc(input.texCoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 5);
	}
	else if (tileMaxCoc >= 17.0)
	{
		result = filterFunc(input.texCoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 4);
	}
	else if (tileMaxCoc >= 13.0)
	{
		result = filterFunc(input.texCoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 3);
	}
	else if (tileMaxCoc >= 9.0)
	{
		result = filterFunc(input.texCoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 2);
	}
	else if (tileMaxCoc >= 5.0)
	{
		result = filterFunc(input.texCoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 1);
	}
	else
	{
		result = currentColor;
	}*/

	//return float4(result.xyz, currentColor.w);

	//return float4(tileMaxCoc, tileMaxCoc, tileMaxCoc, tileMaxCoc)/uniforms.maxBlurDiameter;
	//return float4(tileClosestDepth, tileClosestDepth, tileClosestDepth, tileClosestDepth)*0.01;
	//return currentColor;
	//return float4(currentColor.w, currentColor.w, currentColor.w, currentColor.w) / uniforms.maxBlurDiameter;
	//return float4(currentDepth, currentDepth, currentDepth, currentDepth)*0.01;
}
