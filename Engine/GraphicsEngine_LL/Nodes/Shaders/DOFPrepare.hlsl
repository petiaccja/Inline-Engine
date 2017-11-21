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
SamplerState samp1 : register(s1);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
};

struct PS_Output
{
	float4 color_coc : SV_TARGET0;
	float depth : SV_TARGET1;
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

//warning: result [0...1]
float toDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);

	float zndc = B / depth + A;
	return zndc;
}

float rand(float2 co) {
	return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float4 filterFuncTier3(float2 uv, float2 resolution, float4 center_tap, float center_depth)
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
	float samples = 1;
	float ftaps = 1.0 / float(taps);
	float2 pixelSize = 1.0 / resolution;

	for (int c = 0; c < taps; ++c)
	{
		float xx = cos(2.0 * pi * float(c) * ftaps) * dist;
		float yy = sin(2.0 * pi * float(c) * ftaps) * dist;

		float2 sampleUV = uv + float2(xx, yy) * pixelSize;

		float4 data = inputTex.Sample(samp1, sampleUV); //bilinear tap
		float4 depthGather = depthTex.Gather(samp0, sampleUV);
		float depthMin = linearize_depth(min(depthGather.x, min(depthGather.y, min(depthGather.z, depthGather.w))), 0.1, 100);


		if (abs(depthMin - center_depth) < threshold)
		{
			result += data;
			samples++;
		}
	}

	return float4(result.xyz / samples, center_tap.w);
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

PS_Output PSMain(PS_Input input)
{
	PS_Output output;

	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	float4 inputData = inputTex.Sample(samp0, input.texcoord);
	float4 depthGather = depthTex.Gather(samp0, input.texcoord);
	float maxDepth = max(depthGather.x, max(depthGather.y, max(depthGather.z, depthGather.w)));
	float inputDepth = linearize_depth(maxDepth, 0.1, 100);

	float sensor_width = 0.035; //35mm full frame sensor

	float focal_length_multiplier = 1.5; //1.0 for full frame
	float focal_length = 50 * focal_length_multiplier; //millimeters
	float f_stops = 5.6; //millimeters
	float subject_distance = 0.3; //meters

	float coc = calculate_coc(focal_length * 0.001, subject_distance, f_stops * 0.001, inputDepth); //in meters
	//float pixel_coc = inputTexSize.x * 0.5 * coc / sensor_width; //0.5 for half res rendering!
	float pixel_coc = inputTexSize.x * coc / sensor_width; //0.5 for half res rendering!
	float final_coc = min(pixel_coc, uniforms.maxBlurDiameter);

	//float4 prefilteredColor = filterFuncTier3(input.texcoord, inputTexSize.xy, inputData, inputDepth);
	float4 prefilteredColor = filterFuncTier3(input.texcoord, inputTexSize.xy, float4(inputData.xyz, final_coc), inputDepth);

	output.color_coc = float4(prefilteredColor.xyz, final_coc);
	output.depth = maxDepth;
	return output;
	//return inputTex.Sample(samp0, input.texcoord);
	//return float4(inputData.xyz, min(coc * coc_multiplier, uniforms.maxBlurDiameter) );
	//return linearize_depth(depthTex.Sample(samp0, input.texcoord), 0.1, 100);
}
