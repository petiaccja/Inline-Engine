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
	float2 texCoord : TEXCOORD0;
};

struct PS_Output
{
	float4 colorCoc : SV_TARGET0;
	float depth : SV_TARGET1;
};

//warning: result [0...far]
float LinearizeDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vsZrecon = B / (zndc - A);

	//range: [0...far]
	return vsZrecon;
};

//warning: result [0...1]
float ToDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);

	float zndc = B / depth + A;
	return zndc;
}

float Rand(float2 co) {
	return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float4 FilterFuncTier3(float2 uv, float2 resolution, float4 centerTap, float centerDepth)
{
	const float pi = 3.14159265;
	int taps = 8;
	const float threshold = 0.1;

	float centerCoc = centerTap.w;
	float dist = max(centerCoc * 0.5 * 0.333, 1.0); //9

	if (centerCoc <= 1.0)
	{
		return centerTap;
	}

	float4 result = centerTap;
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
		float depthMin = LinearizeDepth(min(depthGather.x, min(depthGather.y, min(depthGather.z, depthGather.w))), 0.1, 100);


		if (abs(depthMin - centerDepth) < threshold)
		{
			result += data;
			samples++;
		}
	}

	return float4(result.xyz / samples, centerTap.w);
}

//all in metres (scene unit)
float CalculateCoc(float focalLength, float subjectDistance, float openingDiameter, float sceneDepth)
{
	//return focal_length * (focal_length / subject_distance) * abs(scene_depth - subject_distance) /
	//	((focal_length / opening_diameter) * (subject_distance + (scene_depth < subject_distance ? -1 : 1) * abs(scene_depth - subject_distance)));
	
	//based on wikipedia
	return openingDiameter * (focalLength / subjectDistance) * abs(sceneDepth - subjectDistance) / sceneDepth;

	//based on gpu gems
	//return opening_diameter * (focal_length / scene_depth) * abs(scene_depth - subject_distance) / abs(subject_distance - focal_length);

	//gpu gems, optimized math
	//gpu
	//CoC = abs(z * CoCScale + CoCBias) 
	//cpu
	//CoCScale = (aperture * focallength * planeinfocus * (zfar - znear)) / ((planeinfocus - focallength) * znear * zfar)
	//CoCBias = (aperture * focallength * (znear - planeinfocus)) /	((planeinfocus * focallength) * znear)
}


PS_Input VSMain(uint vertexId : SV_VertexID)
{
	// Triangle strip based on vertex id
	// 3-----2
	// |   / |
	// | /   |
	// 1-----0
	// 0: (1, 0)
	// 1: (0, 0)
	// 2: (1, 1)
	// 3: (0, 1)
    PS_Input output;

    output.texCoord.x = (vertexId & 1) ^ 1; // 1 if bit0 is 0.
    output.texCoord.y = vertexId >> 1; // 1 if bit1 is 1.

    float2 posL = output.texCoord.xy * 2.0f - float2(1, 1);
    output.position = float4(posL, 0.5f, 1.0f);
    output.texCoord.y = 1.f - output.texCoord.y;

    return output;
}

PS_Output PSMain(PS_Input input)
{
	PS_Output output;

	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	float4 inputData = inputTex.Sample(samp0, input.texCoord);
	float4 depthGather = depthTex.Gather(samp0, input.texCoord);
	float maxDepth = max(depthGather.x, max(depthGather.y, max(depthGather.z, depthGather.w)));
	float inputDepth = LinearizeDepth(maxDepth, 0.1, 100);
	
	output.colorCoc = inputData;
	output.depth = maxDepth;
	return output;

	float sensorWidth = 0.035; //35mm full frame sensor

	float focalLengthMultiplier = 1.5; //1.0 for full frame
	float focalLength = 50 * focalLengthMultiplier; //millimeters
	float fStops = 5.6; //millimeters
	float subjectDistance = 0.3; //meters

	float coc = CalculateCoc(focalLength * 0.001, subjectDistance, fStops * 0.001, inputDepth); //in meters
	//float pixel_coc = inputTexSize.x * 0.5 * coc / sensor_width; //0.5 for half res rendering!
	float pixelCoc = inputTexSize.x * coc / sensorWidth; //0.5 for half res rendering!
	float finalCoc = min(pixelCoc, uniforms.maxBlurDiameter);

	//float4 prefilteredColor = filterFuncTier3(input.texCoord, inputTexSize.xy, inputData, inputDepth);
	float4 prefilteredColor = FilterFuncTier3(input.texCoord, inputTexSize.xy, float4(inputData.xyz, finalCoc), inputDepth);

	output.colorCoc = float4(prefilteredColor.xyz, finalCoc);
	output.depth = maxDepth;
	return output;
	//return inputTex.Sample(samp0, input.texCoord);
	//return float4(inputData.xyz, min(coc * coc_multiplier, uniforms.maxBlurDiameter) );
	//return linearize_depth(depthTex.Sample(samp0, input.texCoord), 0.1, 100);
}
