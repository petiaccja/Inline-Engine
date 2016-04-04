
struct PSInput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSmain(float3 position : POSITION, float3 color : COLOR) {
	PSInput result;

	result.position = position;
	result.color = color;

	return result;
}

float4 PSmain(PSInput input) : SV_TARGET {
	return input.color;
}
