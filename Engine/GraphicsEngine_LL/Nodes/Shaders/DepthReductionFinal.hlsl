/*
 * Depth reduction shader (final pass)
 * Input: reduction texture
 * Output 0: Light MVP matrices for each cascade
 * Output 1: Shadow matrices for each cascade
 * Output 2: CSM splits for each cascade
 */

Texture2D inputTex : register(t0);
RWTexture2D<float4> outputTex0 : register(u0);
RWTexture2D<float4> outputTex1 : register(u1);
RWTexture2D<float2> outputTex2 : register(u2);

struct Uniforms
{
	float4x4 invVP;
	float4x4 bias_mx, inv_mv;
	float4 cam_pos, cam_view_dir, cam_up_vector;
	float4 light_cam_pos, light_cam_view_dir, light_cam_up_vector;
	float cam_near, cam_far, tex_size;
	float dummy;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

#define FLT_MAX 3.402823466e+38

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

//x: min depth
//y: max depth
groupshared float2 localData[LOCAL_SIZE_X * LOCAL_SIZE_Y];

struct camera
{
	float3 pos, view_dir, up_vector;
};

float linearize_depth(float depth, float near, float far)
{
	float A = -(far + near) / (far - near);
	float B = -2 * far * near / (far - near);
	float zndc = depth * 2 - 1;

	//view space linear z
	float vs_zrecon = -B / (zndc + A);

	//range: [0...1]
	return vs_zrecon / -far;
};

camera lookat_func(float3 eye, float3 lookat, float3 up)
{
	camera c;
	c.view_dir = normalize(lookat - eye);
	c.up_vector = normalize(up);
	c.pos = eye;
	float3 right = normalize(cross(c.view_dir, c.up_vector));
	c.up_vector = normalize(cross(right, c.view_dir));
	return c;
};

float4x4 create_identity()
{
	float4x4 r;
	r[0] = float4(1, 0, 0, 0);
	r[1] = float4(0, 1, 0, 0);
	r[2] = float4(0, 0, 1, 0);
	r[3] = float4(0, 0, 0, 1);
	return r;
}

float4x4 ortographic(float left, float right, float bottom, float top, float near, float far)
{
	float4x4 r = create_identity();

	r[0].x = 2.0 / (right - left);
	r[1].y = 2.0 / (top - bottom);
	r[2].z = -2.0 / (far - near);
	
	//r[3].x = -((right + left) / (right - left));
	//r[3].y = -((top + bottom) / (top - bottom));
	//r[3].z = -((far + near) / (far - near));
	r[0].w = -((right + left) / (right - left));
	r[1].w = -((top + bottom) / (top - bottom));
	r[2].w = -((far + near) / (far - near));
	
	r[3].w = 1.0;

	return r;
}

float4x4 create_translation(float3 vec)
{
//	return float4x4(1, 0, 0, 0,
//		0, 1, 0, 0,
//		0, 0, 1, 0,
//		vec.x, vec.y, vec.z, 1);
	return float4x4(1, 0, 0, vec.x,
		0, 1, 0, vec.y,
		0, 0, 1, vec.z,
		0, 0, 0, 1);
}

float4x4 get_camera_matrix(camera c)
{
	float3 x = cross(c.view_dir, c.up_vector);

	float4x4 m;
	m[0] = float4(x, 0);
	m[1] = float4(c.up_vector, 0);
	m[2] = float4(-c.view_dir, 0);
	m[3] = float4(float3(0.0f, 0.0f, 0.0f), 1);
	//m = transpose(m);

	return mul(m, create_translation(-c.pos));
}

//heavily based on mjp shadow sample
float4x4 efficient_shadow_split_matrix(int idx, float4x4 invVP, float2 frustum_splits[4], camera cam, camera light_cam, float shadow_map_size)
{
	//frustum corners in ndc space
	float3 ndc_frustum_corners[8] =
	{
		float3(-1.0f, 1.0f, -1.0f),
		float3(1.0f, 1.0f, -1.0f),
		float3(1.0f, -1.0f, -1.0f),
		float3(-1.0f, -1.0f, -1.0f),
		float3(-1.0f, 1.0f, 1.0f),
		float3(1.0f, 1.0f, 1.0f),
		float3(1.0f, -1.0f, 1.0f),
		float3(-1.0f, -1.0f, 1.0f),
	};

	float3 ws_frustum_corners[8];

	float4x4 inv_viewproj = invVP;

	for (int c = 0; c < 8; ++c)
	{
		float4 trans = mul(inv_viewproj, float4(ndc_frustum_corners[c], 1));
		ws_frustum_corners[c] = trans.xyz / trans.w;
	}

	//calc this splits frustum corners
	for (int i = 0; i < 4; ++i)
	{
		float3 cornerRay = ws_frustum_corners[i + 4] - ws_frustum_corners[i];
		float3 nearCornerRay = cornerRay * frustum_splits[idx].x;
		float3 farCornerRay = cornerRay * frustum_splits[idx].y;
		ws_frustum_corners[i + 4] = ws_frustum_corners[i] + farCornerRay;
		ws_frustum_corners[i] = ws_frustum_corners[i] + nearCornerRay;
	}

	//calc split centroid center
	float3 centroid_center = float3(0, 0, 0);
	for (int j = 0; j < 8; ++j)
	{
		centroid_center += ws_frustum_corners[j];
	}
	centroid_center /= 8.0f;

	float3 up = normalize(cross(cam.view_dir, cam.up_vector));

	float3 light_cam_pos = centroid_center;
	float3 light_cam_lookat = centroid_center + light_cam.view_dir;

	camera new_light_cam = lookat_func(light_cam_pos, light_cam_lookat, up);
	float4x4 light_view_mat = get_camera_matrix(new_light_cam);

	float3 mins = float3(FLT_MAX, FLT_MAX, FLT_MAX);
	float3 maxes = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int k = 0; k < 8; ++k)
	{
		float3 corner = (mul(light_view_mat, float4(ws_frustum_corners[k], 1))).xyz;
		mins = min(mins, corner);
		maxes = max(maxes, corner);
	}

	float3 maxExtents = maxes;
	float3 minExtents = mins;

	float filter_size = 3;
	float scale = (shadow_map_size + filter_size) / shadow_map_size;

	maxExtents *= scale;
	maxExtents.z /= scale;
	minExtents *= scale;
	minExtents.z /= scale;

	float3 cascadeExtents = maxExtents - minExtents;

	float3 cam_pos = centroid_center + light_cam.view_dir * -abs(minExtents.z);

	float4x4 split_ortho_matrix = ortographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, cascadeExtents.z);

	camera split_shadow_cam = lookat_func(cam_pos, centroid_center, up);

	return mul(split_ortho_matrix, get_camera_matrix(split_shadow_cam));
	//camera sanity_cam = lookat_func(float3(0,0,1), float3(0,-1,0), float3(0,0,1));
	//return mul(split_ortho_matrix, get_camera_matrix(sanity_cam));
	//return ortographic(-100, 100, -100, 100, 0.0f, 100);
	//return mul(ortographic(-25, 25, -25, 25, 0.0f, 25), light_view_mat);
}

float log_partition_from_range(uint part, float minz, float maxz)
{
	float z = maxz;
	if (part < 4)
	{
		float ratio = maxz / minz;
		float power = float(part) * 0.25;
		z = minz * pow(ratio, power);
	}

	return z;
}

void calc_frustum_splits(float cam_near, float cam_far, out float2 frustum_splits[4])
{
	for (int c = 0; c < 4; ++c)
	{
		frustum_splits[c].x = log_partition_from_range(c, cam_near, cam_far);
		frustum_splits[c].y = log_partition_from_range(c + 1, cam_near, cam_far);
	}
}

void init(uint2 dispatchThreadId, uint groupIndex)
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	if (any(dispatchThreadId.xy >= inputTexSize.xy))
		return;

	float2 data = inputTex.Load(int3(dispatchThreadId.xy, 0)).xy;

	localData[groupIndex].x = min(data.x, localData[groupIndex].x);
	localData[groupIndex].y = max(data.y, localData[groupIndex].y);
}

void reduce(uint groupIndex, uint idx)
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
	{ //INIT
		uint3 inputTexSize;
		inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

		localData[groupIndex] = float2(1.0f, 0.0f);

		for (uint y = groupThreadId.y * (inputTexSize.y / LOCAL_SIZE_Y); y < (groupThreadId.y + 1) * (inputTexSize.y / LOCAL_SIZE_Y); ++y)
			for (uint x = groupThreadId.x * (inputTexSize.x / LOCAL_SIZE_X); x < (groupThreadId.x + 1) * (inputTexSize.x / LOCAL_SIZE_X); ++x)
			{
				init(uint2(x, y), groupIndex);
			}

		GroupMemoryBarrierWithGroupSync();
	}

	{ //REDUCTION
		const uint reductionSize = LOCAL_SIZE_X * LOCAL_SIZE_Y;
		for (uint x = reductionSize / 2; x > 0; x >>= 1)
		{
			if (groupIndex < x)
			{
				reduce(groupIndex, x);
			}

			GroupMemoryBarrierWithGroupSync();
		}
	}

	//CALC FINAL RESULTS
	if (!bool(groupIndex))
	{
		//construct matrix here
		float near, far;

		{
			float minDepth = localData[0].x;
			float maxDepth = localData[0].y;

			float linearMinDepth = linearize_depth(minDepth, uniforms.cam_near, uniforms.cam_far);
			float linearMaxDepth = linearize_depth(maxDepth, uniforms.cam_near, uniforms.cam_far);

			near = linearMinDepth * uniforms.cam_far;
			far = linearMaxDepth * uniforms.cam_far;
		}

		float4x4 light_mvp[4];
		float4x4 shadow_mat[4];

		{
			float2 norm_frustum_splits[4];

			{
				float2 frustum_splits[4];
				calc_frustum_splits(near, far, frustum_splits);

				for (int c = 0; c < 4; ++c)
					norm_frustum_splits[c] = (frustum_splits[c] - uniforms.cam_near) / (uniforms.cam_far - uniforms.cam_near);

				//frustum_splits[3].y = uniforms.cam_far;
				for (uint d = 0; d < 4; ++d)
					outputTex2[uint2(d,0)] = frustum_splits[d];
			}

			{
				camera cam, light_cam;
				cam.pos = uniforms.cam_pos.xyz;
				cam.view_dir = uniforms.cam_view_dir.xyz;
				cam.up_vector = uniforms.cam_up_vector.xyz;

				light_cam.pos = uniforms.light_cam_pos.xyz;
				light_cam.view_dir = uniforms.light_cam_view_dir.xyz;
				light_cam.up_vector = uniforms.light_cam_up_vector.xyz;
				for (int c = 0; c < 4; ++c)
					light_mvp[c] = efficient_shadow_split_matrix(c, uniforms.invVP, norm_frustum_splits, cam, light_cam, uniforms.tex_size);
			}
		}

		for (int c = 0; c < 4; ++c)
			shadow_mat[c] = mul(uniforms.bias_mx, mul(light_mvp[c], uniforms.inv_mv));

		for (uint d = 0; d < 4; ++d)
		{
			for (uint e = 0; e < 4; ++e)
			{
				outputTex0[uint2(d * 4 + e,0)] = light_mvp[d][e];
				outputTex1[uint2(d * 4 + e,0)] = shadow_mat[d][e];
			}
		}
	}
}
