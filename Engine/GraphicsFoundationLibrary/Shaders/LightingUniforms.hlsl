struct LightData
{
	float4 diffuseColor;
	float4 vsPosition;
	float4 attenuationEnd;
};

struct Uniforms
{
	float4x4 invV;
	LightData ld[10];
	float4 screenDimensions;
	float4 vsCamPos;
	int groupSizeX, groupSizeY;
	float halfExposureFramerate, //0.5 * exposure time (% of time exposure is open -> 0.75?) * frame rate (s? or fps?)
		maxMotionBlurRadius; //pixels
};

ConstantBuffer<Uniforms> uniforms : register(b600);