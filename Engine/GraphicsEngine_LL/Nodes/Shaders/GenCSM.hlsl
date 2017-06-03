
struct Transform
{
	float4x4 MVP;
};


ConstantBuffer<Transform> cb : register(b0);
SamplerState theSampler : register(s0);
Texture2DArray<float4> albedoTex : register(t0);


struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD;
};


PS_Input VSMain(float4 position : POSITION, float4 texCoord : TEX_COORD)
{
	PS_Input result;

    result.position = mul(position, cb.MVP);
	result.texCoord = texCoord.xy;

	return result;
}


void PSMain(PS_Input input)
{
}
