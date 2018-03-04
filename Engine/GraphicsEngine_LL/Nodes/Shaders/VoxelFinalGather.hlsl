/*
* Voxel Final Gather shader
* Input0: voxel light tex
* Input1: depth tex
* Output: scene with added GI
*/

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

SamplerState samp0 : register(s0); //point
SamplerState samp1 : register(s1); //bilinear
SamplerState samp2 : register(s2); //trilinear

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
float4 coneTrace(float3 wsPos, float3 wsNormal, float3 traceDir, float coneAperture)
{
	float4 result = float4(0, 0, 0, 0);

	float traceDist = uniforms.voxelSize;
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

		float4 data = inputTex1.SampleLevel(samp2, voxelTexCoord, mipLevel);
		
		result += (1.0 - result.w) * data;

		//TODO 0.5?????????
		//voxel size???????
		//depends on miplevel???
		traceDist += diameter * 0.5;
	}

	return float4(result.xyz * result.w, 1.0);
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

	return result;
	//return float4(linearDepth, linearDepth, linearDepth, linearDepth);
	//return float4(wsPos, 1.0);
	//return float4(wsViewDir, 1.0);
	//return float4(wsDepthNormal, 1.0);
	//return float4(perfectReflectionDir, 1.0);
}
