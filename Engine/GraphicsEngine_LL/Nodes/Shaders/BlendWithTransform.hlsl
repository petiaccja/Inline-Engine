
struct Transform
{
	float4x4 t;
};

ConstantBuffer<Transform> transform : register(b0);
SamplerState theSampler : register(s0);
Texture2D<float4> tex0 : register(t0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD;
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

    output.position = mul(float4(posL, 0, 1), transform.t);
    output.texCoord.y = 1.f - output.texCoord.y;
	
    return output;
}


float4 PSMain(PS_Input input) : SV_TARGET
{
	float3 coords = {input.texCoord.x, input.texCoord.y, 0.0};
	
	return tex0.Sample(theSampler, coords);
}
