/*
* DOF Prepare shader
* Input: HDR color texture
* Input: depth texture
* Output: premultiplied color texture, coc
*/

struct Uniforms
{
};

//ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
Texture2D depthTex : register(t1); //
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
};

//warning: result [0...far]
float linearize_depth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vs_zrecon = B / (zndc - A);

	//range: [0...far]
	return vs_zrecon;
};

//all in metres (scene unit)
float calculate_coc(float focal_length, float subject_distance, float opening_diameter, float scene_depth)
{
	//return focal_length * (focal_length / subject_distance) * abs(scene_depth - subject_distance) /
	//	((focal_length / opening_diameter) * (subject_distance + (scene_depth < subject_distance ? -1 : 1) * abs(scene_depth - subject_distance)));

	return opening_diameter * (focal_length / subject_distance) * abs(scene_depth - subject_distance) / scene_depth;
}


PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	float4 inputData = inputTex.Sample(samp0, input.texcoord);

	float focal_length = 55; //millimeters
	float f_stops = 2.8; //millimeters
	float subject_distance = 1;
	float max_blur_radius = 28; //pixels
	//calculate coc at far plane
	//calculate multiplier for max blur (which should be at far plane)
	float coc_multiplier = max_blur_radius / calculate_coc(55 * 0.001, subject_distance, 2.8 * 0.001, 100);

	//return inputTex.Sample(samp0, input.texcoord);
	return float4(inputData.xyz, 1) * min(calculate_coc(focal_length * 0.001, subject_distance, f_stops * 0.001, linearize_depth(depthTex.Sample(samp0, input.texcoord), 0.1, 100)) * coc_multiplier, max_blur_radius);
	//return linearize_depth(depthTex.Sample(samp0, input.texcoord), 0.1, 100);
}
