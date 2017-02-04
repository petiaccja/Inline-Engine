/*
 * Depth reduction shader (first pass)
 * Input: depth texture
 * Output X: min depth in each tile
 * Output Y: max depth in each tile
 */

Texture2D inputTex : register(t0)
RWTexture2D<float4> outputTex : register(u0)

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

//x: min depth
//y: max depth
groupshared float2 localData[LOCAL_SIZE_X * LOCAL_SIZE_Y];

void init(uint2 dispatchThreadId, groupIndex)
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	if (any(dispatchThreadId.xy >= inputTexSize.xy))
		return;

	float depth = inputTex.Load(dispatchThreadId.xy, 0);

	if (depth < 1.0f && depth > 0.0f)
	{
		localData[groupIndex].x = min(depth, localData[groupIndex].x);
		localData[groupIndex].y = max(depth, localData[groupIndex].y);
	}
}

[numthreads(LOCAL_SIZE_X, LOCAL_SIZE_Y, 1)]
void CSMain(
	uint3 groupId : SV_GroupID, //WorkGroupId
	uint3 groupThreadId : SV_GroupThreadID, //LocalInvocationId
	uint3 dispatchThreadId : SV_DispatchThreadID, //GlobalInvocationId
	uint groupIndex : SV_GroupIndex //LocalInvocationIndex
	)
{
	localData[groupIndex] = float2(0.0f, 0.0f);
	uint3 dispatchThreadId2 = uint3(groupId.x * 2, groupId.y * 2, 0.0f);
	dispatchThreadId2.xy = dispatchThreadId2.xy * uint2(LOCAL_SIZE_X, LOCAL_SIZE_Y) + groupThreadId;
	init(dispatchThreadId2.xy, groupIndex);
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
		outputTex[groupId] = localData[0];
	}
}
