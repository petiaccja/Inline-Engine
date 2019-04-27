/*
* Shadow minfilter shader
* Input: Shadow map / minfilter texture
* Output: minfiltered texture
*/

struct Uniforms
{
	float4x4 invV;
	float4 farPlaneData0, farPlaneData1;
	float lightSize, nearPlane, farPlane, dummy;
	float4 vsLightPos;
	float2 direction;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //shadow map / minfilter texture
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};


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

float PSMain(PS_Input input) : SV_TARGET
{
	float offsets[10] =
	{
		5.0, 4.0, 3.0, 2.0, 1.0, -1.0, -2.0, -3.0, -4.0, -5.0
	};

	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	float2 direction = uniforms.direction / float2(inputTexSize.xy);
	
	float2 magicConst = float2(inputTexSize.xy) / float2(128.0, 128.0);

	float minz = inputTex.Sample(samp0, input.texCoord).x; //center

	for (int c = 0; c < 10; c++)
	{
		minz = min(minz, inputTex.Sample(samp0, input.texCoord + direction * (offsets[c] * uniforms.lightSize * magicConst)));
	}

	return minz;
}
