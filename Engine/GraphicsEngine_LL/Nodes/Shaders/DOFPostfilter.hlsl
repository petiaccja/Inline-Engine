/*
* DOF Postfilter shader
* Input: HDR color texture
* Output: median filtered HDR color texture
*/

struct Uniforms
{
	float maxBlurDiameter;
	float tileSize;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
Texture2D depthTex : register(t1); //
Texture2D originalTex : register(t2); //full res tex
SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD0;
};

#define s2(a, b)				temp = a; a = min(a, b); b = max(temp, b);
#define mn3(a, b, c)			s2(a, b); s2(a, c);
#define mx3(a, b, c)			s2(b, c); s2(a, c);

#define mnmx3(a, b, c)			mx3(a, b, c); s2(a, b);                                   // 3 exchanges
#define mnmx4(a, b, c, d)		s2(a, b); s2(c, d); s2(a, c); s2(b, d);                   // 4 exchanges
#define mnmx5(a, b, c, d, e)	s2(a, b); s2(c, d); mn3(a, c, e); mx3(b, d, e);           // 6 exchanges
#define mnmx6(a, b, c, d, e, f) s2(a, d); s2(b, e); s2(c, f); mn3(a, b, c); mx3(d, e, f); // 7 exchanges

//https://graphicscodex.com/research/McGuire2008Median/index.html
float4 MedianFilter(int2 uv, int2 resolution)
{
	float4 v[9];

	// Add the pixels which make up our window to the pixel array.
	for (int dX = -1; dX <= 1; ++dX) 
	{
		for (int dY = -1; dY <= 1; ++dY) 
		{
			// If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
			// pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
			// bottom right pixel of the window at pixel[N-1].
			v[(dX + 1) * 3 + (dY + 1)] = inputTex.Load(clamp(int3(uv + int2(dX, dY), 0), int3(0,0,0), int3(resolution, 0)));
		}
	}

	float4 temp;

	// Starting with a subset of size 6, remove the min and max each time
	mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
	mnmx5(v[1], v[2], v[3], v[4], v[6]);
	mnmx4(v[2], v[3], v[4], v[7]);
	mnmx3(v[3], v[4], v[8]);
	
	return v[4];
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

	//return medianFilter(input.position.xy, inputTexSize.xy);
	return inputTex.Load(int3(int2(input.position.xy), 0));
}
