/*
 * Depth reduction shader (final pass)
 * Input: reduction texture
 * Output 0: Light MVP matrices for each cascade
 * Output 1: Shadow matrices for each cascade
 * Output 2: CSM splits for each cascade
 */

Texture2D inputTex : register(t0);
RWTexture2D<float4> outputTex0 : register(u0);

struct light_data
{
	float4 vs_position;
	float attenuation_end;
	int index;
	float2 dummy;
};

struct Uniforms
{
	light_data ld[200];
	float4x4 p;
	float4 far_plane0, far_plane1;
	float cam_near, cam_far; 
	int num_lights;
	float dummy;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

float proj_a, proj_b;

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

	float4 raw_depth = inputTex.Load(int3(dispatchThreadId.xy, 0));

	float max_depth = 0;
	float min_depth = 1;

	if (groupIndex == 0)
	{
		localLL = float4(uniforms.far_plane0.xyz, 1.0);
		localUR = float4(uniforms.far_plane0.w, uniforms.far_plane1.xy, 1.0);
		localFar = nearfar.y; 
		localNear = nearfar.x; 
		localNumLightsInput = uniforms.num_lights;

		localNumLightsOutput = 0;

		localMaxDepth = 0;
		localMinDepth = 0x7f7fffff; // max float value
		localDepthMask = 0;
	}

	GroupMemoryBarrierWithGroupSync(); //local memory barrier

	float far = localFar;
	float near = localNear;

	//WARNING: need to linearize the depth in order to make it work...
	proj_a = -(far + near) / (far - near);
	proj_b = (-2 * far * near) / (far - near);
	float linear_depth = -proj_b / (raw_depth.x * 2 - 1 + proj_a);
	raw_depth.x = linear_depth / -far;

	int num_of_lights = localNumLightsInput;
	vec3 ll, ur;
	ll = localLL.xyz;
	ur = localUR.xyz;

	//check for skybox
	bool early_rejection = (raw_depth.x > 0.999 || raw_depth.x < 0.001);

	if (!early_rejection)
	{
		float tmp_depth = raw_depth.x;

		min_depth = min(min_depth, tmp_depth);
		max_depth = max(max_depth, tmp_depth);

		if (max_depth >= min_depth)
		{
			InterlockedMin(localMinDepth, asuint(min_depth));
			InterlockedMax(localMaxDepth, asuint(max_depth));
		}
	}

	GroupMemoryBarrierWithGroupSync(); //local memory barrier

	max_depth = asfloat(localMaxDepth);
	min_depth = asfloat(localMinDepth);

	float2 tile_scale = float2(inputTexSize.x, inputTexSize.y) * recip(LOCAL_SIZE_X + LOCAL_SIZE_Y);
	float2 tile_bias = tile_scale - float2(groupId.x, groupId.y);

	float proj_11 = uniforms.p[0].x;
	float proj_22 = uniforms.p[1].y;

	float4 c1 = float4(proj_11 * tile_scale.x, 0.0, -tile_bias.x, 0.0);
	float4 c2 = float4(0.0, proj_22 * tile_scale.y, -tile_bias.y, 0.0);
	float4 c4 = float4(0.0, 0.0, -1.0, 0.0);

	float4 frustum_planes[6];

	frustum_planes[0] = c4 - c1;
	frustum_planes[1] = c4 + c1;
	frustum_planes[2] = c4 - c2;
	frustum_planes[3] = c4 + c2;
	frustum_planes[4] = float4(0.0, 0.0, 1.0, -min_depth * far); 
	frustum_planes[5] = float4(0.0, 0.0, 1.0, -max_depth * far); 

	frustum_planes[0].xyz = normalize(frustum_planes[0].xyz);
	frustum_planes[1].xyz = normalize(frustum_planes[1].xyz);
	frustum_planes[2].xyz = normalize(frustum_planes[2].xyz);
	frustum_planes[3].xyz = normalize(frustum_planes[3].xyz);

	///TODO TO BE CONTINUED.

	/*
	* Calculate per tile depth mask for 2.5D light culling
	*/

	/**/
	float vs_min_depth = min_depth * -far;
	float vs_max_depth = max_depth * -far;
	float vs_depth = raw_depth.x * -far;

	float range = abs(vs_max_depth - vs_min_depth + 0.00001) / 32.0; //depth range in each tile

	vs_depth -= vs_min_depth; //so that min = 0
	float depth_slot = floor(vs_depth / range);

	//determine the cell for each pixel in the tile
	if (!early_rejection)
	{
		//depth_mask = depth_mask | (1 << depth_slot)
		atomicOr(local_depth_mask, 1 << uint(depth_slot));
	}

	barrier();
	/**/

	for (uint c = workgroup_index; c < num_of_lights; c += local_size.x * local_size.y)
	{
		bool in_frustum = true;
		int index = int(c);

		float att_end = sld.d[index].attenuation_end;
		vec3 light_pos = sld.d[index].vs_position.xyz;
		vec4 lp = vec4(light_pos, 1.0);

		/**/
		//calculate per light bitmask
		uint light_bitmask = 0;

		float light_z_min = -(light_pos.z + att_end); //light z min [0 ... 1000]
		float light_z_max = -(light_pos.z - att_end); //light z max [0 ... 1000]
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

		in_frustum = in_frustum && bool(local_depth_mask & light_bitmask);
		/**/

		//manual unroll
		{
			float e = dot(frustum_planes[0], lp);
			in_frustum = in_frustum && (e >= -att_end);
		}
		{
			float e = dot(frustum_planes[1], lp);
			in_frustum = in_frustum && (e >= -att_end);
		}
		{
			float e = dot(frustum_planes[2], lp);
			in_frustum = in_frustum && (e >= -att_end);
		}
		{
			float e = dot(frustum_planes[3], lp);
			in_frustum = in_frustum && (e >= -att_end);
		}
		{
			float e = dot(frustum_planes[4], lp);
			in_frustum = in_frustum && (e <= att_end);
		}
		{
			float e = dot(frustum_planes[5], lp);
			in_frustum = in_frustum && (e >= -att_end);
		}

		if (in_frustum)
		{
			int li = atomicAdd(local_num_of_lights, 1);
			local_lights[li] = int(index);
		}
	}

	barrier(); //local memory barrier

	if (workgroup_index == 0)
	{
		imageStore(result, int((group_id.x * group_size.y + group_id.y) * 1024), uvec4(local_num_of_lights));
		//imageStore( result, int((group_id.x * group_size.y + group_id.y) * 1024), uvec4(abs(sld.d[0].spot_direction.z*10)) );
	}

	for (uint c = workgroup_index; c < local_num_of_lights; c += local_size.x * local_size.y)
	{
		imageStore(result, int((group_id.x * group_size.y + group_id.y) * 1024 + c + 1), uvec4(local_lights[c]));
	}
}
