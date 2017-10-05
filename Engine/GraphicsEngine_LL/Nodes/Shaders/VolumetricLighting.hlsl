/*
 * Volumetric lighting shader
 * Input: depth texture, lit opaque scene texture, culled sdfs per tile
 * Output: scattered light blended on top of scene
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

[numthreads(LOCAL_SIZE_X, LOCAL_SIZE_Y, 1)]
void CSMain(
	uint3 groupId : SV_GroupID, //WorkGroupId
	uint3 groupThreadId : SV_GroupThreadID, //LocalInvocationId
	uint3 dispatchThreadId : SV_DispatchThreadID, //GlobalInvocationId
	uint groupIndex : SV_GroupIndex //LocalInvocationIndex
)
{
	//[0...far]
	float linear_depth = linearize_depth(depthTex.Load(int3(dispatchThreadId.xy, 0)).x, uniforms.cam_near, uniforms.cam_far);

	uint local_num_of_sdfs = cullTex.Load(int3(groupId.x * uniforms.num_workgroups_y + groupId.y, 0, 0));

	float4 outColor = float4(0, 0, 0, 0);
	//outColor = float4(local_num_of_sdfs, 0, 0, 1);
	//outColor = float4(linear_depth, linear_depth, linear_depth, linear_depth);

	for (uint c = 0; c < local_num_of_sdfs; ++c)
	{
		uint index = cullTex.Load(int3(groupId.x * uniforms.num_workgroups_y + groupId.y, c + 1, 0));
		//get sdf data...
	}

	dstTex[dispatchThreadId.xy] = outColor;
}
