/*
 * SDF culling shader
 * Input: sdfs
 * Output 0: sdf indices + num of sdfs
 */

Texture2D depthTex : register(t0);
Texture2D<float4> inputColorTex : register(t1);
RWTexture2D<float4> volDstTex0 : register(u0);
RWTexture2D<float4> volDstTex1 : register(u1);
RWTexture2D<float4> dstTex : register(u2);
RWTexture2D<uint> cullTex : register(u3);

struct SdfData
{
	float4 vsPosition;
	float radius;
	float3 dummy;
};

struct LightData
{
	float4 diffuseLightColor;
	float4 vsPosition;
	float attenuationEnd;
	float3 dummy;
};

struct Uniforms
{
	SdfData sd[10];
	LightData ld[10];
	float4x4 v, p;
	float4x4 invVP, oldVP;
	float camNear, camFar, dummy1, dummy2;
	uint numSdfs, numWorkgroupsX, numWorkgroupsY; float haltonFactor;
	float4 sunDirection;
	float4 sunColor;
	float4 camPos;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

#define FLT_MAX 3.402823466e+38

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

groupshared int localNumSDFsInput, localNumSDFsOutput;
groupshared int localSDFs[1024];

float LinearizeDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vsZrecon = B / (zndc - A);

	//range: [0...1]
	return vsZrecon / far;
};

[numthreads(LOCAL_SIZE_X, LOCAL_SIZE_Y, 1)]
void CSMain(
	uint3 groupId : SV_GroupID, //WorkGroupId
	uint3 groupThreadId : SV_GroupThreadID, //LocalInvocationId
	uint3 dispatchThreadId : SV_DispatchThreadID, //GlobalInvocationId
	uint groupIndex : SV_GroupIndex //LocalInvocationIndex
)
{
	uint3 inputTexSize;
	inputColorTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	if (groupIndex == 0)
	{
		localNumSDFsInput = uniforms.numSdfs;
		localNumSDFsOutput = 0;
	}

	GroupMemoryBarrierWithGroupSync(); //local memory barrier

	float2 tileScale = float2(inputTexSize.x, inputTexSize.y) * (1.0 / (LOCAL_SIZE_X + LOCAL_SIZE_Y));
	float2 tileBias = tileScale - float2(groupId.x, groupId.y);

	float proj11 = uniforms.p[0].x;
	float proj22 = uniforms.p[1].y;

	float4 c1 = float4(proj11 * tileScale.x, 0.0, tileBias.x, 0.0);
	float4 c2 = float4(0.0, -proj22 * tileScale.y, tileBias.y, 0.0);
	float4 c4 = float4(0.0, 0.0, 1.0, 0.0);

	float4 frustumPlanes[6];

	frustumPlanes[0] = c4 - c1;
	frustumPlanes[1] = c4 + c1;
	frustumPlanes[2] = c4 - c2;
	frustumPlanes[3] = c4 + c2;
	frustumPlanes[4] = float4(0.0, 0.0, 1.0, 0.0); 
	frustumPlanes[5] = float4(0.0, 0.0, 1.0, uniforms.camFar); 

	frustumPlanes[0].xyz = normalize(frustumPlanes[0].xyz);
	frustumPlanes[1].xyz = normalize(frustumPlanes[1].xyz);
	frustumPlanes[2].xyz = normalize(frustumPlanes[2].xyz);
	frustumPlanes[3].xyz = normalize(frustumPlanes[3].xyz);

	for (uint c = groupIndex; c < localNumSDFsInput; c += LOCAL_SIZE_X * LOCAL_SIZE_Y)
	{
		bool inFrustum = true;
		int index = int(c);

		float radius = uniforms.sd[index].radius;
		float4 sdfPos = float4(uniforms.sd[index].vsPosition.xyz, 1.0);

		//manual unroll
		{
			float e = dot(frustumPlanes[0], sdfPos);
			inFrustum = inFrustum && (e >= -radius);
		}
		{
			float e = dot(frustumPlanes[1], sdfPos);
			inFrustum = inFrustum && (e >= -radius);
		}
		{
			float e = dot(frustumPlanes[2], sdfPos);
			inFrustum = inFrustum && (e >= -radius);
		}
		{
			float e = dot(frustumPlanes[3], sdfPos);
			inFrustum = inFrustum && (e >= -radius);
		}
		/*{
			float e = dot(frustum_planes[4], sdf_pos);
			in_frustum = in_frustum && (e >= -radius);
		}
		{
			float e = dot(frustum_planes[5], sdf_pos);
			in_frustum = in_frustum && (e >= -radius);
		}*/

		if (inFrustum)
		{
			localSDFs[localNumSDFsOutput] = int(index);
			InterlockedAdd(localNumSDFsOutput, 1);
		}
	}

	GroupMemoryBarrierWithGroupSync(); //local memory barrier

	if (groupIndex == 0)
	{
		cullTex[int2(groupId.x * uniforms.numWorkgroupsY + groupId.y, 0)] = uint(localNumSDFsOutput);
	}

	for (uint c = groupIndex; c < localNumSDFsOutput; c += LOCAL_SIZE_X * LOCAL_SIZE_Y)
	{
		cullTex[int2(groupId.x * uniforms.numWorkgroupsY + groupId.y, c + 1)] = uint(localSDFs[c]);
	}
}
