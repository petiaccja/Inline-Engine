/*
* Voxel Final Gather shader
* Input0: voxel light tex
* Input1: depth tex
* Output: scene with added GI
*/

#include "EncodeDecode.hlsl"

#define LOW 1
#define MEDIUM 2
#define HIGH 3
#define ULTRA 4

#define QUALITY LOW

#if QUALITY == ULTRA
//ULTRA HIGH SETTING CONES
#define NUM_CONES 46
#define DIFFUSE_APERTURE 0.174533
static const float3 coneDirs[46] = {
	float3(-0.4713, 0.6617, 0.5831),
	float3(-0.7002, 0.6617, -0.2680),
	float3(0.0385, 0.6617, -0.7488),
	float3(0.7240, 0.6617, -0.1947),
	float3(0.2680, 0.9435, -0.1947),
	float3(0.4911, 0.7947, -0.3568),
	float3(0.4089, 0.6617, -0.6284),
	float3(-0.1024, 0.9435, -0.3151),
	float3(-0.1876, 0.7947, -0.5773),
	float3(-0.4713, 0.6617, -0.5831),
	float3(-0.3313, 0.9435, 0.0000),
	float3(-0.6071, 0.7947, 0.0000),
	float3(-0.7002, 0.6617, 0.2680),
	float3(-0.1024, 0.9435, 0.3151),
	float3(-0.1876, 0.7947, 0.5773),
	float3(0.0385, 0.6617, 0.7488),
	float3(0.2680, 0.9435, 0.1947),
	float3(0.4911, 0.7947, 0.3568),
	float3(0.7240, 0.6617, 0.1947),
	float3(0.8897, 0.3304, -0.3151),
	//float3(0.7947, 0.1876, -0.5773),
	float3(0.5746, 0.3304, -0.7488),
	float3(-0.0247, 0.3304, -0.9435),
	//float3(-0.3035, 0.1876, -0.9342),
	float3(-0.5346, 0.3304, -0.7779),
	float3(-0.9050, 0.3304, -0.2680),
	//float3(-0.9822, 0.1876, 0.0000),
	float3(-0.9050, 0.3304, 0.2680),
	float3(-0.5346, 0.3304, 0.7779),
	//float3(-0.3035, 0.1876, 0.9342),
	float3(-0.0247, 0.3304, 0.9435),
	float3(0.5746, 0.3304, 0.7488),
	//float3(0.7947, 0.1876, 0.5773),
	float3(0.8897, 0.3304, 0.3151),
	/*float3(0.3066, 0.1256, -0.9435),
	float3(-0.8026, 0.1256, -0.5831),
	float3(-0.8026, 0.1256, 0.5831),
	float3(0.3066, 0.1256, 0.9435),
	float3(0.9921, 0.1256, 0.0000),*/
	float3(0.4089, 0.6617, 0.6284),
	float3(0.276388, 0.447220, 0.850649),
	float3(-0.723607, 0.447220, 0.525725),
	float3(-0.723607, 0.447220, -0.525725),
	float3(0.276388, 0.447220, -0.850649),
	float3(0.894426, 0.447216, 0.000000),
	float3(0.000000, 1.000000, 0.000000),
	float3(0.688189, 0.525736, 0.499997),
	float3(-0.262869, 0.525738, 0.809012),
	float3(-0.850648, 0.525736, 0.000000),
	float3(-0.262869, 0.525738, -0.809012),
	float3(0.688189, 0.525736, -0.499997),
	float3(0.162456, 0.850654, 0.499995),
	float3(0.525730, 0.850652, 0.000000),
	float3(-0.425323, 0.850654, 0.309011),
	float3(-0.425323, 0.850654, -0.309011),
	float3(0.162456, 0.850654, -0.499995)
};
#elif QUALITY == HIGH
#elif QUALITY == MEDIUM
#elif QUALITY == LOW
#define NUM_CONES 6
#define DIFFUSE_APERTURE 0.453786
	static const float3 coneDirs[6] = {
		float3(0.0, 1.0,  0.000000),
		float3(-0.794654, 0.607062,  0.000000),
		float3(0.642889, 0.607062,  0.467086),
		float3(0.642889, 0.607062, -0.467086),
		float3(-0.245562, 0.607062,  0.755761),
		float3(-0.245562, 0.607062, -0.755761)
	};
#else
#error "define quality"
#endif

struct Uniforms
{
	float4x4 model, viewProj, invView;
	float3 voxelCenter; float voxelSize;
	float4 farPlaneData0, farPlaneData1;
	float4 wsCamPos;
	int voxelDimension; int inputMipLevel; int outputMipLevel; int dummy;
	float nearPlane, farPlane;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture3D<float4> inputTex0 : register(t0); //voxel light tex
Texture3D<float4> inputTex1 : register(t1); //voxel tex
Texture2D<float> inputTex2 : register(t2); //depth tex
Texture3D<float4> inputTex3 : register(t3); //secondary voxel tex
Texture2D<float4> inputTex4 : register(t4); //velocityNormal tex
Texture2D<float4> inputTex5 : register(t5); //albedoRoughnessMetalness tex
Texture2D<float4> inputTex6 : register(t6); //screenSpaceAmbientOcclusion tex

SamplerState samp0 : register(s0); //point
SamplerState samp1 : register(s1); //bilinear
SamplerState samp2 : register(s2); //trilinear

//transforms a direction into the coordinate system
//defined by the normal vector
float3 TransNormal(float3 n, float3 d)
{
	float3 a, b;

	//had to modify it to 0.9 (should be 1 for normalization check, div-by-0)
	if (abs(n.z) < 0.9)
	{
		a = normalize(cross(float3(0, 0, 1), n));
	}
	else
	{
		a = normalize(cross(float3(1, 0, 0), n));
	}

	b = normalize(cross(n, a));

	return a * d.x + b * d.y + n * d.z;
}

float3 MultiBounce(float ao, float3 albedo)
{
	float3 a = 2.0404 * albedo - 0.3324;
	float3 b = -4.7951 * albedo + 0.6417;
	float3 c = 2.7552 * albedo + 0.6903;

	return max(ao, ((ao * a + b) * ao + c) * ao);
}

float LinearizeDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vsZrecon = B / (zndc - A);

	//range: [0...far]
	return vsZrecon;// / far;
};

float3 WsPosToVoxelTC(float3 wsPos)
{
	const float voxelTotalSize = uniforms.voxelDimension * uniforms.voxelSize;
	const float voxelTotalSizeInv = 1.0 / voxelTotalSize;
	const float3 voxelOrigin = uniforms.voxelCenter - float3(voxelTotalSize, voxelTotalSize, voxelTotalSize) * 0.5;
	return (wsPos - voxelOrigin) * voxelTotalSizeInv;
}

//aperture: tan(coneHalfAngle) ???
float4 ConeTrace(float3 wsPos, float3 wsNormal, float3 traceDir, float coneAperture, float ssao, const bool opacityOnly = false)
{
	//float4 result = float4(0, 0, 0, 0.0);
	float4 result = float4(0, 0, 0, ssao);

	//TODO start offset based on cone aperture?
	float traceDist = uniforms.voxelSize * 2.0;
	float3 wsStartPos = wsPos + wsNormal * traceDist;

	const float maxDist = uniforms.voxelDimension * uniforms.voxelSize;
	const float invVoxelSize = 1.0 / uniforms.voxelSize;
	const float maxMipLevel = log2(uniforms.voxelDimension);

	//max # of steps?
	while(traceDist < maxDist && result.w < 1.0)
	{
		//at least one voxel
		float diameter = max(uniforms.voxelSize, 2.0 * coneAperture * traceDist);
		float mipLevel = log2(diameter * invVoxelSize); //is this correct?

		//get texture coordinate to sample
		float3 voxelTexCoord = WsPosToVoxelTC(wsStartPos + traceDir * traceDist);

		if (any(voxelTexCoord > float3(1,1,1)) || any(voxelTexCoord < float3(0,0,0)) || mipLevel >= maxMipLevel)
		{
			//we are outsize the voxel texture
			//TODO: just sample from neighbouring voxel tex
			break; 
		}

		//TODO: alpha won't be correct here, as we used the alpha channel for atomic avg counter...
		float4 data = float4(0, 0, 0, 0);
		if (!opacityOnly)
		{
			data.xyz = inputTex0.SampleLevel(samp2, voxelTexCoord, mipLevel).xyz;
		}
		data.w = inputTex3.SampleLevel(samp2, voxelTexCoord, mipLevel).x;
		

		result += (1.0 - result.w) * data;
		//result += min(1.0-ssao, 1.0 - result.w) * data;


		//TODO 0.5?????????
		//voxel size???????
		//depends on miplevel???
		traceDist += diameter * 0.5;
	}

	if (!opacityOnly)
	{
		return float4(result.xyz, result.w);
	}
	else
	{
		return result.wwww;
	}
}

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
};


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

	output.texcoord.x = (vertexId & 1) ^ 1; // 1 if bit0 is 0.
	output.texcoord.y = vertexId >> 1; // 1 if bit1 is 1.

    float2 posL = output.texcoord.xy * 2.0f - float2(1, 1);
    output.position = float4(posL, 0.5f, 1.0f);
    output.texcoord.y = 1.f - output.texcoord.y;

    return output;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float depth = inputTex2.Sample(samp0, input.texcoord);

	if (depth > 0.9999)
	{
		return 0.0;
	}

	float linearDepth = LinearizeDepth(depth, uniforms.nearPlane, uniforms.farPlane);

	float3 farPlaneLL = uniforms.farPlaneData0.xyz;
	float3 farPlaneUR = float3(uniforms.farPlaneData0.w, uniforms.farPlaneData1.xy);

	float2 uv = float2(input.texcoord.x, 1 - input.texcoord.y);
	float3 vsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, uv) / uniforms.farPlane, 1.0) * linearDepth;
	float3 wsPos = mul(float4(vsPos, 1.0), uniforms.invView).xyz;

	float3 wsViewDir = normalize(wsPos - uniforms.wsCamPos.xyz);

	//TODO replace with proper normals
	//float3 wsDepthNormal = normalize(cross(ddy(wsPos.xyz), ddx(wsPos.xyz)));
	float3 vsNormal = DecodeNormal(inputTex4.Sample(samp1, input.texcoord.xy).zw);
	float3 wsNormal = mul(float4(vsNormal, 0.0), uniforms.invView).xyz;

	float3 albedo;
	float roughness;
	float metalness;
	{ //load up albedo, roughness, metalness
		float4 centerAlbedoRoughnessMetalness = inputTex5.Load(float3(input.position.xy, 0));
		float2 a0 = inputTex5.Load(float3(input.position.xy + float2(1, 0), 0)).xy;
		float2 a1 = inputTex5.Load(float3(input.position.xy - float2(1, 0), 0)).xy;
		float2 a2 = inputTex5.Load(float3(input.position.xy + float2(0, 1), 0)).xy;
		float2 a3 = inputTex5.Load(float3(input.position.xy - float2(0, 1), 0)).xy;
		roughness = centerAlbedoRoughnessMetalness.z;
		metalness = centerAlbedoRoughnessMetalness.w;
		albedo = YcocgToRgb(int2(input.position.xy), centerAlbedoRoughnessMetalness.xy, a0, a1, a2, a3);
	}

	float4 ssao = inputTex6.Sample(samp1, input.texcoord.xy).x;

	float3 perfectReflectionDir = normalize(reflect(wsViewDir, wsNormal));

	//TODO derive aperture from something...
	//roughness * pi * 0.125???
	//TODO at grazing angles self reflection happens... we need to overcome this somehow. I added a dot for now but I doubt it's correct...
	float4 specularResult = ConeTrace(wsPos, wsNormal, perfectReflectionDir, 0.0, tan(0.174533 * 0.5)) * pow(max(dot(perfectReflectionDir, wsNormal), 0.0), 0.75);

	//cone trace diffuse GI + AO in alpha
	float4 diffuseResult = float4(0,0,0,0);
	for (int c = 0; c < NUM_CONES; ++c)
	{
		float3 dir = coneDirs[c];
		float3 dirOriented = TransNormal(wsNormal, dir);

		//half angle = 10deg
		float4 coneResult = ConeTrace(wsPos, wsNormal, dirOriented, ssao, tan(DIFFUSE_APERTURE));
		diffuseResult.xyz += coneResult.xyz * pow(max(dot(dirOriented, wsNormal), 0.0), 0.1);
		diffuseResult.w += coneResult.w;
	}
	diffuseResult /= NUM_CONES;
	diffuseResult.w = 1.0 - diffuseResult.w;

	//return ssao;
	//return diffuseResult.w;
	//return min(diffuseResult.w, ssao);
	//return diffuseResult.w *ssao;
	//return float4(diffuseResult.xyz * ssao, 1.0);
	//return float4(diffuseResult.xyz, 1.0);
	//return float4(specularResult.xyz, 1.0);
	return float4(/*albedo * */(diffuseResult.xyz + specularResult.xyz), 1.0);
	//return float4(multiBounce(aoResult, float3(1,1,1)), 1.0);
	//return result;
	//return float4(linearDepth, linearDepth, linearDepth, linearDepth);
	//return float4(wsPos, 1.0);
	//return float4(wsViewDir, 1.0);
	//return float4(wsDepthNormal, 1.0);
	//return float4(perfectReflectionDir, 1.0);
}
