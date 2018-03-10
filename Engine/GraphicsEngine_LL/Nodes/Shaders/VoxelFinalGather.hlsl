/*
* Voxel Final Gather shader
* Input0: voxel light tex
* Input1: depth tex
* Output: scene with added GI
*/

//ULTRA HIGH SETTING CONES
static const float3 coneDirs[56] = {
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
	float3(0.7947, 0.1876, -0.5773),
	float3(0.5746, 0.3304, -0.7488),
	float3(-0.0247, 0.3304, -0.9435),
	float3(-0.3035, 0.1876, -0.9342),
	float3(-0.5346, 0.3304, -0.7779),
	float3(-0.9050, 0.3304, -0.2680),
	float3(-0.9822, 0.1876, 0.0000),
	float3(-0.9050, 0.3304, 0.2680),
	float3(-0.5346, 0.3304, 0.7779),
	float3(-0.3035, 0.1876, 0.9342),
	float3(-0.0247, 0.3304, 0.9435),
	float3(0.5746, 0.3304, 0.7488),
	float3(0.7947, 0.1876, 0.5773),
	float3(0.8897, 0.3304, 0.3151),
	float3(0.3066, 0.1256, -0.9435),
	float3(-0.8026, 0.1256, -0.5831),
	float3(-0.8026, 0.1256, 0.5831),
	float3(0.3066, 0.1256, 0.9435),
	float3(0.9921, 0.1256, 0.0000),
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
#define NUM_CONES 56

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

SamplerState samp0 : register(s0); //point
SamplerState samp1 : register(s1); //bilinear
SamplerState samp2 : register(s2); //trilinear

//transforms a direction into the coordinate system
//defined by the normal vector
float3 trans_normal(float3 n, float3 d)
{
	float3 a, b;

	if (abs(n.z) < 1.0)
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

float linearize_depth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vs_zrecon = B / (zndc - A);

	//range: [0...far]
	return vs_zrecon;// / far;
};

float3 wsPosToVoxelTC(float3 wsPos)
{
	const float voxelTotalSize = uniforms.voxelDimension * uniforms.voxelSize;
	const float voxelTotalSizeInv = 1.0 / voxelTotalSize;
	const float3 voxelOrigin = uniforms.voxelCenter - float3(voxelTotalSize, voxelTotalSize, voxelTotalSize) * 0.5;
	return (wsPos - voxelOrigin) * voxelTotalSizeInv;
}

//aperture: tan(coneHalfAngle) ???
float4 coneTrace(float3 wsPos, float3 wsNormal, float3 traceDir, float coneAperture, const bool opacityOnly = false)
{
	float4 result = float4(0, 0, 0, 0);

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
		float3 voxelTexCoord = wsPosToVoxelTC(wsStartPos + traceDir * traceDist);

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
			data.xyz = inputTex1.SampleLevel(samp2, voxelTexCoord, mipLevel).xyz;
		}
		data.w = inputTex3.SampleLevel(samp2, voxelTexCoord, mipLevel).x;
		

		result += (1.0 - result.w) * data;


		//TODO 0.5?????????
		//voxel size???????
		//depends on miplevel???
		traceDist += diameter * 0.5;
	}

	if (!opacityOnly)
	{
		return float4(result.xyz * result.w, 1.0);
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


PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float depth = inputTex2.Sample(samp0, input.texcoord);

	if (depth > 0.9999)
	{
		return 0.0;
	}

	float linearDepth = linearize_depth(depth, uniforms.nearPlane, uniforms.farPlane);

	float3 farPlaneLL = uniforms.farPlaneData0.xyz;
	float3 farPlaneUR = float3(uniforms.farPlaneData0.w, uniforms.farPlaneData1.xy);

	float2 uv = float2(input.texcoord.x, 1 - input.texcoord.y);
	float3 vsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, uv) / uniforms.farPlane, 1.0) * linearDepth;
	float3 wsPos = mul(float4(vsPos, 1.0), uniforms.invView).xyz;

	float3 wsViewDir = normalize(wsPos - uniforms.wsCamPos.xyz);

	//TODO replace with proper normals
	float3 wsDepthNormal = normalize(cross(ddy(wsPos.xyz), ddx(wsPos.xyz)));

	float3 perfectReflectionDir = normalize(reflect(wsViewDir, wsDepthNormal));

	//TODO derive aperture from something...
	//roughness * pi * 0.125???
	float4 result = coneTrace(wsPos, wsDepthNormal, perfectReflectionDir, tan(0.5 * 0.0174533));

	//cone trace AO
	float aoResult = 0.0;
	for (int c = 0; c < NUM_CONES; ++c)
	{
		float3 dir = coneDirs[c];
		float3 dirOriented = trans_normal(wsDepthNormal, dir);
		//half angle = 10deg
		aoResult += max(dot(wsDepthNormal, dir), 0.0) * coneTrace(wsPos, wsDepthNormal, dirOriented, tan(0.174533), true);
	}
	aoResult /= NUM_CONES;
	aoResult = 1.0 - aoResult;

	return float4(aoResult, aoResult, aoResult, aoResult);
	//return result;// *aoResult;
	//return float4(linearDepth, linearDepth, linearDepth, linearDepth);
	//return float4(wsPos, 1.0);
	//return float4(wsViewDir, 1.0);
	//return float4(wsDepthNormal, 1.0);
	//return float4(perfectReflectionDir, 1.0);
}
