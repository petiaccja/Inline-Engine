/*
* DOF Upsample shader
* Input: HDR color texture
* Output: upsampled HDR color texture
*/

struct Uniforms
{
	float maxBlurDiameter;
	float tileSize;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
Texture2D depthTex : register(t1); //
Texture2D originalTex : register(t2); //full res tex
SamplerState samp0 : register(s0); //point
SamplerState samp1 : register(s1); //linear

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

float4 filterFuncTier3(float2 uv, float2 resolution, float4 center_tap)
{
	const float pi = 3.14159265;
	int taps = 8;
	const float threshold = 0.1;

	float center_coc = center_tap.w;
	float dist = max(center_coc * 0.5 * 0.333, 1.0); //9

	if (center_coc <= 1.0)
	{
		return center_tap;
	}

	float4 result = center_tap;
	float ftaps = 1.0 / float(taps);
	float2 pixelSize = 1.0 / resolution;

	for (int c = 0; c < taps; ++c)
	{
		float xx = cos(2.0 * pi * float(c) * ftaps) * dist;
		float yy = sin(2.0 * pi * float(c) * ftaps) * dist;

		float2 sampleUV = uv + float2(xx, yy) * pixelSize;
 
		result += inputTex.Sample(samp1, sampleUV); //bilinear tap
	}

	return result * ftaps;
}

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

	float4 center_tap = originalTex.Sample(samp0, input.texcoord);
	float4 depthGather = depthTex.Gather(samp0, input.texcoord);
	float maxDepth = max(depthGather.x, max(depthGather.y, max(depthGather.z, depthGather.w)));
	float inputDepth = linearize_depth(maxDepth, 0.1, 100);

	float sensor_width = 0.035; //35mm full frame sensor

	float focal_length_multiplier = 1.5; //1.0 for full frame
	float focal_length = 50 * focal_length_multiplier; //millimeters
	float f_stops = 5.6; //millimeters
	float subject_distance = 0.3; //meters

	float coc = calculate_coc(focal_length * 0.001, subject_distance, f_stops * 0.001, inputDepth); //in meters
	float pixel_coc = inputTexSize.x * coc / sensor_width;
	float final_coc = min(pixel_coc, uniforms.maxBlurDiameter);


	float4 fullres = filterFuncTier3(input.texcoord, inputTexSize.xy, float4(center_tap.xyz, final_coc));
	float4 halfres = inputTex.Sample(samp1, input.texcoord);

	return halfres;
	//return inputTex.Sample(samp1, input.texcoord);
	//return originalTex.Sample(samp0, input.texcoord);
	//return lerp(fullres, halfres, clamp(final_coc-1.0, 0.0, 2.0)*0.5);
}
