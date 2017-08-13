/*
* DOF Prepare shader
* Input: HDR color texture
* Input: depth texture
* Output: premultiplied color texture, coc
*/

struct Uniforms
{
	float maxBlurDiameter;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

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
	
	//based on wikipedia
	return opening_diameter * (focal_length / subject_distance) * abs(scene_depth - subject_distance) / scene_depth;

	//based on gpu gems
	//return opening_diameter * (focal_length / scene_depth) * abs(scene_depth - subject_distance) / abs(subject_distance - focal_length);

	//gpu gems, optimized math
	//gpu
	//CoC = abs(z * CoCScale + CoCBias) 
	//cpu
	//CoCScale = (aperture * focallength * planeinfocus * (zfar - znear)) / ((planeinfocus - focallength) * znear * zfar)
	//CoCBias = (aperture * focallength * (znear - planeinfocus)) /	((planeinfocus * focallength) * znear)
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

	float sensor_width = 0.035; //35mm full frame sensor

	float focal_length_multiplier = 1.5; //1.0 for full frame
	float focal_length = 50 * focal_length_multiplier; //millimeters
	float f_stops = 5.6; //millimeters
	float subject_distance = 0.3; //meters
	//calculate coc at far plane
	//calculate multiplier for max blur (which should be at far plane)
	//float coc_multiplier = uniforms.maxBlurDiameter / calculate_coc(focal_length * 0.001, subject_distance, f_stops * 0.001, 100);

	float coc = calculate_coc(focal_length * 0.001, subject_distance, f_stops * 0.001, linearize_depth(depthTex.Sample(samp0, input.texcoord), 0.1, 100)); //in meters
	float pixel_coc = inputTexSize.x * coc / sensor_width;

	return float4(inputData.xyz, min(pixel_coc, uniforms.maxBlurDiameter));
	//return inputTex.Sample(samp0, input.texcoord);
	//return float4(inputData.xyz, min(coc * coc_multiplier, uniforms.maxBlurDiameter) );
	//return linearize_depth(depthTex.Sample(samp0, input.texcoord), 0.1, 100);
}
