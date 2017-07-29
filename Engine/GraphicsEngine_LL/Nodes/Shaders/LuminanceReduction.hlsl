/*
 * Luminance reduction shader (first pass)
 * Input: luminance texture
 * Output: avg luminance in each tile
 */

Texture2D inputTex : register(t0);
RWTexture2D<float2> outputTex : register(u0);

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

//avg luminance
groupshared float localData[LOCAL_SIZE_X * LOCAL_SIZE_Y];

void init(uint2 dispatchThreadId, uint groupIndex)
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	if (any(dispatchThreadId.xy >= inputTexSize.xy))
		return;

	float luminance = inputTex.Load(int3(dispatchThreadId.xy, 0)).x;

	localData[groupIndex] = luminance;
}

void reduce(uint groupIndex, uint idx)
{
	localData[groupIndex] += localData[groupIndex + idx];
}

[numthreads(LOCAL_SIZE_X, LOCAL_SIZE_Y, 1)]
void CSMain(
	uint3 groupId : SV_GroupID, //WorkGroupId
	uint3 groupThreadId : SV_GroupThreadID, //LocalInvocationId
	uint3 dispatchThreadId : SV_DispatchThreadID, //GlobalInvocationId
	uint groupIndex : SV_GroupIndex //LocalInvocationIndex
	)
{
	localData[groupIndex] = 0.0f;

	//get data from every second workgroup on the x axis
	uint3 dispatchThreadId2 = uint3(groupId.x * 2, groupId.y, 0.0f);
	dispatchThreadId2.xy = dispatchThreadId2.xy * uint2(LOCAL_SIZE_X, LOCAL_SIZE_Y) + groupThreadId.xy;
	init(dispatchThreadId2.xy, groupIndex);
	//get data from every other workgroup on the x axis
	init(uint2(dispatchThreadId2.x + LOCAL_SIZE_X, dispatchThreadId2.y), groupIndex);

	GroupMemoryBarrierWithGroupSync();

	const uint reductionSize = LOCAL_SIZE_X * LOCAL_SIZE_Y;
	for (uint x = reductionSize / 2; x > 0; x >>= 1)
	{
		if (groupIndex < x)
		{
			reduce(groupIndex, x);
		}

		GroupMemoryBarrierWithGroupSync();
	}

	if (!groupIndex)
	{
		outputTex[groupId.xy] = localData[0] * (1.0 / (LOCAL_SIZE_X * LOCAL_SIZE_Y));
	}
}
