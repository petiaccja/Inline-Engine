// Shader constants
struct Constants {
    float3x3 worldViewProj;
    float4 color;
    bool hasTexture;
	bool hasMesh;
    float z;
};
ConstantBuffer<Constants> constants : register(b0);


// Texture inputs
Texture2D colorTexture : register(t0);
SamplerState linearSampler : register(s0);


// Shaders
void VSMain(float2 posL : POSITION,
			float2 texCoord : TEXCOORD0,
			out float4 posHOut : SV_Position,
			out float2 texCoordOut : TEXCOORD0)
{
    float3 posH = mul(float3(posL, 1), constants.worldViewProj);
    posHOut = float4(posH.xy, constants.z * posH.z, posH.z);
    texCoordOut = texCoord;
}


float4 PSMain(float4 posS : SV_Position, float2 texCoord : TEXCOORD0) : SV_Target0
{
    float4 texColor;
    if (constants.hasTexture) {
        texColor = colorTexture.Sample(linearSampler, texCoord);
        return texColor * constants.color;
    }
    else {
		return constants.color;
    }
}