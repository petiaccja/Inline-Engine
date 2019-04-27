/*
 * Luminance reduction shader (final pass)
 * Input: reduction texture
 * Output 0: average luminance
 */

Texture2D inputTex : register(t0);
RWTexture2D<float> outputTex0 : register(u0);

struct Uniforms
{
	float middleGrey, deltaTime;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

#define FLT_MAX 3.402823466e+38

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

//avg luminance
groupshared float localData[LOCAL_SIZE_X * LOCAL_SIZE_Y];

void Init(uint2 dispatchThreadId, uint groupIndex)
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	//uint2 inputTexSize;
	//inputTex.GetDimensions(inputTexSize.x, inputTexSize.y);
	if (any(dispatchThreadId.xy >= inputTexSize.xy))
		return;

	float data = inputTex.Load(int3(dispatchThreadId.xy, 0)).x;

	localData[groupIndex] = data;
}

void Reduce(uint groupIndex, uint idx)
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
	{ //INIT
		uint3 inputTexSize;
		inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

		localData[groupIndex] = 0.0;

		for (uint y = groupThreadId.y * (inputTexSize.y / float(LOCAL_SIZE_Y)); y < (groupThreadId.y + 1) * (inputTexSize.y / float(LOCAL_SIZE_Y)); ++y)
			for (uint x = groupThreadId.x * (inputTexSize.x / float(LOCAL_SIZE_X)); x < (groupThreadId.x + 1) * (inputTexSize.x / float(LOCAL_SIZE_X)); ++x)
			{
				Init(uint2(x, y), groupIndex);
			}

		GroupMemoryBarrierWithGroupSync();
	}

	{ //REDUCTION
		const uint reductionSize = LOCAL_SIZE_X * LOCAL_SIZE_Y;
		for (uint x = reductionSize / 2; x > 0; x >>= 1)
		{
			if (groupIndex < x)
			{
				Reduce(groupIndex, x);
			}

			GroupMemoryBarrierWithGroupSync();
		}
	}

	//CALC FINAL RESULTS
	if (!bool(groupIndex))
	{
		float avgLum = localData[0] * (1.0 / (LOCAL_SIZE_X * LOCAL_SIZE_Y));

		const float delta = 0.0001;

		float currLum = uniforms.middleGrey / (exp(avgLum) - delta);
		float lastLum = outputTex0[uint2(0, 0)];
		//curr_lum = check_lum(curr_lum);
		//last_lum = check_lum(last_lum);

		const float tau = 0.5; //TODO separate uptau, downtau, based on where the current luminance is relative to the last lum
		float adaptedLum = lastLum + (currLum - lastLum) * (1 - exp(-uniforms.deltaTime * tau));

		outputTex0[uint2(0, 0)] = adaptedLum;
	}
}
