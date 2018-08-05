/*
 * Voxel mipmap gen shader
 * Input: 3D texture level N
 * Output: 3D texture level N+1
 */

struct Uniforms
{
	float4x4 model;
	float3 voxelCenter; float voxelSize;
	int voxelDimension; int inputMipLevel;
};

Texture3D inputTex : register(t0);
RWTexture3D<float4> outputTex : register(u0); //need to bind specific mip level
ConstantBuffer<Uniforms> uniforms : register(b0);

SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);

#define LOCAL_SIZE_X 8
#define LOCAL_SIZE_Y 8
#define LOCAL_SIZE_Z 8

[numthreads(LOCAL_SIZE_X, LOCAL_SIZE_Y, LOCAL_SIZE_Z)]
void CSMain(
	uint3 groupId : SV_GroupID, //WorkGroupId
	uint3 groupThreadId : SV_GroupThreadID, //LocalInvocationId
	uint3 dispatchThreadId : SV_DispatchThreadID, //GlobalInvocationId
	uint groupIndex : SV_GroupIndex //LocalInvocationIndex
	)
{
	uint4 inputTexSize;
	inputTex.GetDimensions(uniforms.inputMipLevel, inputTexSize.x, inputTexSize.y, inputTexSize.z, inputTexSize.w);
	uint3 outputTexSize;
	outputTex.GetDimensions(outputTexSize.x, outputTexSize.y, outputTexSize.z);

	float3 uvw = (float3(dispatchThreadId.xyz) + float3(0.5, 0.5, 0.5)) / outputTexSize.xyz;

	float4 data = inputTex.SampleLevel(samp1, uvw, uniforms.inputMipLevel);

	outputTex[dispatchThreadId.xyz] = data; 
}
