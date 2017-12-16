struct light_data
{
	float4 diffuse_color;
	float4 vs_position;
	float4 attenuation_end;
};

struct Uniforms
{
	float4x4 invV;
	light_data ld[10];
	float4 screen_dimensions;
	float4 vs_cam_pos;
	int group_size_x, group_size_y;
	float halfExposureFramerate, //0.5 * exposure time (% of time exposure is open -> 0.75?) * frame rate (s? or fps?)
		maxMotionBlurRadius; //pixels
};

ConstantBuffer<Uniforms> uniforms : register(b600);