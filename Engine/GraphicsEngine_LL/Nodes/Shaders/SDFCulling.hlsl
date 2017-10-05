/*
 * SDF culling shader
 * Input: sdfs
 * Output 0: sdf indices + num of sdfs
 */

Texture2D depthTex : register(t0);
Texture2D<float4> inputColorTex : register(t1);
RWTexture2D<float4> dstTex : register(u0);
RWTexture2D<uint> cullTex : register(u1);

struct sdf_data
{
	float4 vs_position;
	float radius;
	float3 dummy;
};

struct Uniforms
{
	sdf_data sd[10];
	float4x4 p;
	float4 far_plane0, far_plane1;
	float cam_near, cam_far; 
	uint num_sdfs, num_workgroups_x, num_workgroups_y;
	float3 dummy;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

#define FLT_MAX 3.402823466e+38

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

groupshared int localNumSDFsInput, localNumSDFsOutput;
groupshared int localSDFs[1024];

float linearize_depth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vs_zrecon = B / (zndc - A);

	//range: [0...1]
	return vs_zrecon / far;
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
		localNumSDFsInput = uniforms.num_sdfs;
		localNumSDFsOutput = 0;
	}

	GroupMemoryBarrierWithGroupSync(); //local memory barrier

	float2 tile_scale = float2(inputTexSize.x, inputTexSize.y) * (1.0 / (LOCAL_SIZE_X + LOCAL_SIZE_Y));
	float2 tile_bias = tile_scale - float2(groupId.x, groupId.y);

	float proj_11 = uniforms.p[0].x;
	float proj_22 = uniforms.p[1].y;

	float4 c1 = float4(proj_11 * tile_scale.x, 0.0, tile_bias.x, 0.0);
	float4 c2 = float4(0.0, -proj_22 * tile_scale.y, tile_bias.y, 0.0);
	float4 c4 = float4(0.0, 0.0, 1.0, 0.0);

	float4 frustum_planes[6];

	frustum_planes[0] = c4 - c1;
	frustum_planes[1] = c4 + c1;
	frustum_planes[2] = c4 - c2;
	frustum_planes[3] = c4 + c2;
	frustum_planes[4] = float4(0.0, 0.0, 1.0, 0.0); 
	frustum_planes[5] = float4(0.0, 0.0, 1.0, uniforms.cam_far); 

	frustum_planes[0].xyz = normalize(frustum_planes[0].xyz);
	frustum_planes[1].xyz = normalize(frustum_planes[1].xyz);
	frustum_planes[2].xyz = normalize(frustum_planes[2].xyz);
	frustum_planes[3].xyz = normalize(frustum_planes[3].xyz);

	for (uint c = groupIndex; c < localNumSDFsInput; c += LOCAL_SIZE_X * LOCAL_SIZE_Y)
	{
		bool in_frustum = true;
		int index = int(c);

		float radius = uniforms.sd[index].radius;
		float4 sdf_pos = float4(uniforms.sd[index].vs_position.xyz, 1.0);

		//manual unroll
		{
			float e = dot(frustum_planes[0], sdf_pos);
			in_frustum = in_frustum && (e >= -radius);
		}
		{
			float e = dot(frustum_planes[1], sdf_pos);
			in_frustum = in_frustum && (e >= -radius);
		}
		{
			float e = dot(frustum_planes[2], sdf_pos);
			in_frustum = in_frustum && (e >= -radius);
		}
		{
			float e = dot(frustum_planes[3], sdf_pos);
			in_frustum = in_frustum && (e >= -radius);
		}
		/*{
			float e = dot(frustum_planes[4], sdf_pos);
			in_frustum = in_frustum && (e >= -radius);
		}
		{
			float e = dot(frustum_planes[5], sdf_pos);
			in_frustum = in_frustum && (e >= -radius);
		}*/

		if (in_frustum)
		{
			localSDFs[localNumSDFsOutput] = int(index);
			InterlockedAdd(localNumSDFsOutput, 1);
		}
	}

	GroupMemoryBarrierWithGroupSync(); //local memory barrier

	if (groupIndex == 0)
	{
		cullTex[int2(groupId.x * uniforms.num_workgroups_y + groupId.y, 0)] = uint(localNumSDFsOutput);
	}

	for (uint c = groupIndex; c < localNumSDFsOutput; c += LOCAL_SIZE_X * LOCAL_SIZE_Y)
	{
		cullTex[int2(groupId.x * uniforms.num_workgroups_y + groupId.y, c + 1)] = uint(localSDFs[c]);
	}
}
