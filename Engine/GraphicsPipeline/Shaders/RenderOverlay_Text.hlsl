// Shader constants
struct TransformConstants {
    float3x3 worldViewProj;
	uint __padding;
    float z;
};

struct RenderConstants {
	int2 atlasAccessTopleft;
	int2 atlasAccessSize;
	float4 color;
	float3x3 discardTransform;
	uint __padding;
	bool enableDiscard;
};
ConstantBuffer<TransformConstants> transformConstants : register(b0);
ConstantBuffer<RenderConstants> renderConstants : register(b7);


// Texture inputs
Texture2D alphaTexture : register(t0);
SamplerState linearSampler : register(s0);


// Shaders
void VSMain(uint vertexId : SV_VertexID,
			out float4 posHOut : SV_Position,
			out float2 texCoordOut : TEXCOORD0,
			out float2 posNdcOut : TEXCOORD1)
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
	
	texCoordOut.x = (vertexId & 1) ^ 1; // 1 if bit0 is 0.
	texCoordOut.y = vertexId >> 1; // 1 if bit1 is 1.

	float2 posL = texCoordOut.xy * 2.0f - float2(1,1);

    float3 posH = mul(float3(posL, 1), transformConstants.worldViewProj);
    posHOut = float4(posH.xy, transformConstants.z * posH.z, posH.z);
	posNdcOut = posH / posH.z;
}


float4 PSMain(float4 posS : SV_Position, float2 texCoord : TEXCOORD0, float2 posNdc : TEXCOORD1) : SV_Target0
{
	if (renderConstants.enableDiscard) {
		float3 discardPos = mul(float3(posNdc.xy, 1.0f), renderConstants.discardTransform);
		discardPos /= discardPos.z;
		discardPos.z = 0.0f;
		if (abs(discardPos.x) > 0.5f || abs(discardPos.y) > 0.5f) {
			discard;
		}
	}

	uint2 atlasSize;
	alphaTexture.GetDimensions(atlasSize.x, atlasSize.y);

	float2 topleft = (float2)renderConstants.atlasAccessTopleft / (float2)atlasSize;
	float2 bottomRight = (float2)(renderConstants.atlasAccessTopleft + renderConstants.atlasAccessSize) / (float2)atlasSize;

    float2 sampleCoord = float2(topleft.x * (1 - texCoord.x) + bottomRight.x * texCoord.x, topleft.y * texCoord.y + bottomRight.y * (1 - texCoord.y));

	float4 alpha = alphaTexture.Sample(linearSampler, sampleCoord);

    return float4(renderConstants.color.xyz, alpha.x);
}