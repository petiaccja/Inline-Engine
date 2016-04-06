
struct PSInput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSmain(float3 position : POSITION, float3 color : COLOR) {
	PSInput result;

	result.position = float4(position, 1);
	result.color = float4(color, 1);

	return result;
}

float4 PSmain(PSInput input) : SV_TARGET {
	return input.color;
}
