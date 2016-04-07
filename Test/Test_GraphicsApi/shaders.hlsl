
// Transform constants
struct Transform {
	float4x4 world;
	float4x4 viewProj;
};
ConstantBuffer<Transform> transform : register(b0);
// Misc constants
struct MiscInfo {
	float4 color;
};
ConstantBuffer<MiscInfo> misc : register(b8);

// Textures
Texture2D<float4> tex : register(t0);

// Samplers
sampler samp : register(s1);


struct VSInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEX;
};

struct PSInput {
	float4 position : SV_POSITION;
	float3 normal : COLOR;
};

// Vertex shader
PSInput VSmain(VSInput input) {
	PSInput result;

	//result.position = float4(input.position, 1);
	float4x4 wvp = mul(transform.world, transform.viewProj);
	result.position = mul(float4(input.position, 1), wvp);
	float3x3 worldRot = (float3x3)transform.world;
	result.normal = mul(input.normal, worldRot);

	return result;
}

// Pixel shader
float4 PSmain(PSInput input) : SV_TARGET {
	float ndotl = dot(input.normal, float3(-0.5, 0.5, 0.5));
	return float4(ndotl, ndotl, ndotl,1);
}
