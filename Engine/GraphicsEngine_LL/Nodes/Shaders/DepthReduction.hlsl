/*
 * Depth reduction shader (first pass)
 * Input: depth texture
 * Output X: min depth in each tile
 * Output Y: max depth in each tile
 */

Texture2D inputTex : register(t0);
RWTexture2D<float2> outputTex : register(u0);

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

//x: min depth
//y: max depth
groupshared float2 localData[LOCAL_SIZE_X * LOCAL_SIZE_Y];

float LinearizeDepth(float depth)
{
	float near = 0.1;
	float far = 100.0;
	float A = -(far + near) / (far - near);
	float B = -2 * far * near / (far - near);
	float zndc = depth * 2 - 1;

	//view space linear z
	float vsZrecon = -B / (zndc + A);

	//range: [0...1]
	return vsZrecon / -1.0;//far;
};

void Init(uint2 dispatchThreadId, uint groupIndex)
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	if (any(dispatchThreadId.xy >= inputTexSize.xy))
		return;

	float depth = inputTex.Load(int3(dispatchThreadId.xy, 0)).x;

	if (depth < 1.0f && depth > 0.0f)
	{
		//depth = linearize_depth(depth);
		localData[groupIndex].x = min(depth, localData[groupIndex].x);
		localData[groupIndex].y = max(depth, localData[groupIndex].y);
	}
}

void Reduce(uint groupIndex, uint idx)
{
	localData[groupIndex].x = min(localData[groupIndex].x, localData[groupIndex + idx].x);
	localData[groupIndex].y = max(localData[groupIndex].y, localData[groupIndex + idx].y);
}

[numthreads(LOCAL_SIZE_X, LOCAL_SIZE_Y, 1)]
void CSMain(
	uint3 groupId : SV_GroupID, //WorkGroupId
	uint3 groupThreadId : SV_GroupThreadID, //LocalInvocationId
	uint3 dispatchThreadId : SV_DispatchThreadID, //GlobalInvocationId
	uint groupIndex : SV_GroupIndex //LocalInvocationIndex
	)
{
	//localData[groupIndex] = float2(100.0f, 0.0f);
	localData[groupIndex] = float2(1.0f, 0.0f);

	//get data from every second workgroup on the x axis
	uint3 dispatchThreadId2 = uint3(groupId.x * 2, groupId.y, 0.0f);
	dispatchThreadId2.xy = dispatchThreadId2.xy * uint2(LOCAL_SIZE_X, LOCAL_SIZE_Y) + groupThreadId.xy;
	Init(dispatchThreadId2.xy, groupIndex);
	//get data from every other workgroup on the x axis
	Init(uint2(dispatchThreadId2.x + LOCAL_SIZE_X, dispatchThreadId2.y), groupIndex);

	GroupMemoryBarrierWithGroupSync();

	const uint reductionSize = LOCAL_SIZE_X * LOCAL_SIZE_Y;
	for (uint x = reductionSize / 2; x > 0; x >>= 1)
	{
		if (groupIndex < x)
		{
			Reduce(groupIndex, x);
		}

		GroupMemoryBarrierWithGroupSync();
	}

	if (!groupIndex)
	{
		outputTex[groupId.xy].xy = localData[0];
	}
}
