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

//warning: result [0...1]
float toDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	
	float zndc = B / depth + A;
	return zndc;
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

float weight(float2 uv, float alpha)
{
	float subject_distance = toDepth(0.3, 0.1, 100); //0.667
	float depth = depthTex.Sample(samp0, uv).x;
	float toSubjectDist = abs(depthTex.Sample(samp0, uv).x - subject_distance);

	if (depth > subject_distance)
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

float4 circle_filter(float2 uv, float dist, float2 resolution, const int taps, inout float4 result, inout float revealage)
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

			result += float4(data.xyz * alpha, alpha) * min(weight(sampleUV, alpha), 1.0);
			
			if (revealage > 0.001) //float underflow fix
			{
				revealage *= (1.0 - alpha);
			}
		}
	}
	return result;
}

float4 filterFunc(float2 uv, float2 resolution, float4 center_tap, float coc, int rings)
{
	const float pi = 3.14159265;

	float dist = coc * 0.5;

	float center_coc = max(center_tap.w, 1.0);
	float center_alpha = (4 * pi) / (center_coc*center_coc*pi*0.25);
	//float center_alpha = (pi) / (center_coc*center_coc*pi*0.25);

	float4 result = float4(center_tap.xyz * center_alpha, center_alpha) * min(weight(uv, center_alpha), 1.0);
	float revealage = (1 - center_alpha);

	for (int d = 1; d <= rings; ++d)
	{
		circle_filter(uv, float(d) / float(rings) * dist, resolution, 8 * d, result, revealage);
	}

	return float4((result.rgb / clamp(result.a, 1e-4, 5e4)) * saturate(1.0 - revealage), 1);
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

	int2 offset = int2(sign(tileDistX) * float(rand(input.texcoord) < (abs(tileDistX) * 0.5)), sign(tileDistY) * float(rand(input.texcoord + 1) < (abs(tileDistY) * 0.5)));
	float2 tileData = neighborhoodMaxTex.Load(clamp(int3(currPixelPos/(int)uniforms.tileSize + offset, 0), int3(0,0,0), int3(inputTexSize.xy / (int)uniforms.tileSize - 1, 0))).xy;
	//float2 tileData = neighborhoodMaxTex.Sample(samp1, input.texcoord + float2(rand(input.texcoord)*2-1, rand(input.texcoord*2)*2-1)*0.05);
	//float2 tileData = neighborhoodMaxTex.Sample(samp1, input.texcoord);

	float tileMaxCoc = tileData.x;

	float4 result = float4(0, 0, 0, 0);
	
	result = filterFunc(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 7);

	/*if (tileMaxCoc >= 29.0)
	{
		result = filterFunc(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 7);
	}
	else if (tileMaxCoc >= 25.0)
	{
		result = filterFunc(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 6);
	}
	else if (tileMaxCoc >= 21.0)
	{
		result = filterFunc(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 5);
	}
	else if (tileMaxCoc >= 17.0)
	{
		result = filterFunc(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 4);
	}
	else if (tileMaxCoc >= 13.0)
	{
		result = filterFunc(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 3);
	}
	else if (tileMaxCoc >= 9.0)
	{
		result = filterFunc(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 2);
	}
	else if (tileMaxCoc >= 5.0)
	{
		result = filterFunc(input.texcoord, float2(inputTexSize.xy), currentColor, tileMaxCoc, 1);
	}
	else
	{
		result = currentColor;
	}*/

	return float4(result.xyz, currentColor.w);

	//return float4(tileMaxCoc, tileMaxCoc, tileMaxCoc, tileMaxCoc)/uniforms.maxBlurDiameter;
	//return float4(tileClosestDepth, tileClosestDepth, tileClosestDepth, tileClosestDepth)*0.01;
	//return currentColor;
	//return float4(currentColor.w, currentColor.w, currentColor.w, currentColor.w) / uniforms.maxBlurDiameter;
	//return float4(currentDepth, currentDepth, currentDepth, currentDepth)*0.01;
}
