/*
 * Light culing shader
 * Input: depth texture
 * Output 0: light indices + num of lights
 */

Texture2D inputTex : register(t0);
RWTexture2D<uint> outputTex0 : register(u0);

struct LightData
{
	float4 vsPosition;
	float attenuationEnd;
	float3 dummy;
};

struct Uniforms
{
	LightData ld[10];
	float4x4 p;
	float4 farPlane0, farPlane1;
	float camNear, camFar; 
	uint numLights, numWorkgroupsX, numWorkgroupsY;
	float3 dummy;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

#define FLT_MAX 3.402823466e+38

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

groupshared float localFar, localNear;
groupshared int localNumLightsInput, localNumLightsOutput;
groupshared float4 localLL, localUR;
groupshared int localLights[1024];
groupshared uint localMinDepth;
groupshared uint localMaxDepth;
groupshared uint localDepthMask;

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
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	//per-pixel depth
	float4 rawDepth = inputTex.Load(int3(dispatchThreadId.xy, 0));
	
	//outputTex0[dispatchThreadId.xy] = asuint(linearize_depth(raw_depth.x, uniforms.cam_near, uniforms.cam_far));
	//return;

	//min/max depth in tile
	float maxDepth = 0;
	float minDepth = 1;

	if (groupIndex == 0)
	{
		localLL = float4(uniforms.farPlane0.xyz, 1.0);
		localUR = float4(uniforms.farPlane0.w, uniforms.farPlane1.xy, 1.0);
		localFar = uniforms.camFar; 
		localNear = uniforms.camNear; 
		localNumLightsInput = uniforms.numLights;

		localNumLightsOutput = 0;

		localMaxDepth = 0;
		localMinDepth = 0x7f7fffff; // max float value
		localDepthMask = 0;
	}

	GroupMemoryBarrierWithGroupSync(); //local memory barrier

	float far = localFar;
	float near = localNear;

	//WARNING: need to linearize the depth in order to make it work...
	rawDepth.x = LinearizeDepth(rawDepth.x, near, far);

	int numOfLights = localNumLightsInput;
	float3 ll, ur;
	ll = localLL.xyz;
	ur = localUR.xyz;

	//check for skybox
	bool earlyRejection = (rawDepth.x > 0.999 || rawDepth.x < 0.001);

	if (!earlyRejection)
	{
		float tmpDepth = rawDepth.x;

		minDepth = min(minDepth, tmpDepth);
		maxDepth = max(maxDepth, tmpDepth);

		if (maxDepth >= minDepth)
		{
			InterlockedMin(localMinDepth, asuint(minDepth));
			InterlockedMax(localMaxDepth, asuint(maxDepth));
		}
	}

	GroupMemoryBarrierWithGroupSync(); //local memory barrier

	maxDepth = asfloat(localMaxDepth);
	minDepth = asfloat(localMinDepth);

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
	frustumPlanes[4] = float4(0.0, 0.0, 1.0, minDepth * far); 
	frustumPlanes[5] = float4(0.0, 0.0, 1.0, maxDepth * far); 
		   
	frustumPlanes[0].xyz = normalize(frustumPlanes[0].xyz);
	frustumPlanes[1].xyz = normalize(frustumPlanes[1].xyz);
	frustumPlanes[2].xyz = normalize(frustumPlanes[2].xyz);
	frustumPlanes[3].xyz = normalize(frustumPlanes[3].xyz);

	/*
	* Calculate per tile depth mask for 2.5D light culling
	*/

	/** //TODO
	float vs_min_depth = min_depth * far;
	float vs_max_depth = max_depth * far;
	float vs_depth = raw_depth.x * far;

	float range = abs(vs_max_depth - vs_min_depth + 0.00001) / 32.0; //depth range in each tile

	vs_depth -= vs_min_depth; //so that min = 0
	float depth_slot = floor(vs_depth / range);

	//determine the cell for each pixel in the tile
	if (!early_rejection)
	{
		//depth_mask = depth_mask | (1 << depth_slot)
		InterlockedOr(localDepthMask, 1 << uint(depth_slot));
	}

	GroupMemoryBarrierWithGroupSync(); //local memory barrier
	/**/

	for (uint c = groupIndex; c < numOfLights; c += LOCAL_SIZE_X * LOCAL_SIZE_Y)
	{
		bool inFrustum = true;
		int index = int(c);

		float attEnd = uniforms.ld[index].attenuationEnd;
		float3 lightPos = uniforms.ld[index].vsPosition.xyz;
		float4 lp = float4(lightPos, 1.0);

		/** //TODO
		//calculate per light bitmask
		uint light_bitmask = 0;

		float light_z_min = light_pos.z + att_end; //light z min [0 ... 1000]
		float light_z_max = light_pos.z - att_end; //light z max [0 ... 1000]
		light_z_min -= vs_min_depth; //so that min = 0
		light_z_max -= vs_min_depth; //so that min = 0
		float depth_slot_min = floor(light_z_min / range);
		float depth_slot_max = floor(light_z_max / range);

		if (!((depth_slot_max > 31.0 &&
			depth_slot_min > 31.0) ||
			(depth_slot_min < 0.0 &&
				depth_slot_max < 0.0)))
		{
			if (depth_slot_max > 30.0)
				light_bitmask = uint(~0);
			else
				light_bitmask = (1 << (uint(depth_slot_max)+1)) - 1;

			if (depth_slot_min > 0.0)
				light_bitmask -= (1 << uint(depth_slot_min)) - 1;
		}

		in_frustum = in_frustum && bool(localDepthMask & light_bitmask);
		/**/

		//manual unroll
		{
			float e = dot(frustumPlanes[0], lp);
			inFrustum = inFrustum && (e >= -attEnd);
		}
		{
			float e = dot(frustumPlanes[1], lp);
			inFrustum = inFrustum && (e >= -attEnd);
		}
		{
			float e = dot(frustumPlanes[2], lp);
			inFrustum = inFrustum && (e >= -attEnd);
		}
		{
			float e = dot(frustumPlanes[3], lp);
			inFrustum = inFrustum && (e >= -attEnd);
		}
		{
			float e = dot(frustumPlanes[4], lp);
			inFrustum = inFrustum && (e >= -attEnd);
		}
		{
			float e = dot(frustumPlanes[5], lp);
			inFrustum = inFrustum && (e >= -attEnd);
		}

		if (inFrustum)
		{
			localLights[localNumLightsOutput] = int(index);
			InterlockedAdd(localNumLightsOutput, 1);
		}
	}

	//localLights[localNumLightsOutput] = int(0);
	//localNumLightsOutput = 1;

	GroupMemoryBarrierWithGroupSync(); //local memory barrier

	if (groupIndex == 0)
	{
		outputTex0[int2(groupId.x * uniforms.numWorkgroupsY + groupId.y, 0)] = uint(localNumLightsOutput);
	}

	for (uint c = groupIndex; c < localNumLightsOutput; c += LOCAL_SIZE_X * LOCAL_SIZE_Y)
	{
		outputTex0[int2(groupId.x * uniforms.numWorkgroupsY + groupId.y, c + 1)] = uint(localLights[c]);
	}
}
