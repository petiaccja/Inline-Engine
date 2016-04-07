
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


struct PSInput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

// Vertex shader
PSInput VSmain(float3 position : POSITION, float3 color : COLOR) {
	PSInput result;

	result.position = float4(position, 1);
	result.color = float4(color, 1);

	return result;
}

// Pixel shader
float4 PSmain(PSInput input) : SV_TARGET {
	return input.color;
}
