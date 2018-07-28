/*
 * Depth reduction shader (final pass)
 * Input: reduction texture
 * Output 0: Light MVP matrices for each cascade
 * Output 1: Shadow matrices for each cascade
 * Output 2: CSM splits for each cascade
 */

Texture2D inputTex : register(t0);
//RWTexture2D<float2> inputTex : register(u3);
RWTexture2D<float4> outputTex0 : register(u0);
RWTexture2D<float4> outputTex1 : register(u1);
RWTexture2D<float2> outputTex2 : register(u2);
RWTexture2D<float4> outputTex3 : register(u3);

struct Uniforms
{
	float4x4 invVP;
	float4x4 biasMx, invMv;
	float4 camPos, camViewDir, camUpVector;
	float4 lightCamPos, lightCamViewDir, lightCamUpVector;
	float camNear, camFar, texSize;
	float dummy;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

#define FLT_MAX 3.402823466e+38

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

//x: min depth
//y: max depth
groupshared float2 localData[LOCAL_SIZE_X * LOCAL_SIZE_Y];

struct Camera
{
	float3 pos, viewDir, upVector;
};

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

Camera LookatFunc(float3 eye, float3 lookat, float3 up)
{
	Camera c;
	c.viewDir = normalize(lookat - eye);
	c.upVector = normalize(up);
	c.pos = eye;
	float3 right = normalize(cross(c.viewDir, c.upVector));
	c.upVector = normalize(cross(right, c.viewDir));
	return c;
};

float4x4 CreateIdentity()
{
	return float4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
}

float4x4 Ortographic(float left, float right, float bottom, float top, float near, float far)
{
	float4x4 r = CreateIdentity();

	// Left handed ortho projection, output interval min [-1, -1, 0] max [1, 1, 1]
	//2 / w     0       0               0
	//0         2 / h   0               0
	//0         0       1 / (zf - zn)   0
	//0         0       zn / (zn - zf)  1

	float width = right - left;
	float height = top - bottom;

	r._m00 = 2.0 / width;
	r._m11 = 2.0 / height;
	r._m22 = 1.0 / (far - near);
	//r._m23 = near / (near - far);	
	r._m32 = near / (near - far);

	r._m33 = 1.0;

	return r;
}

float4x4 CreateTranslation(float3 vec)
{
	return float4x4(
        1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		vec.x, vec.y, vec.z, 1);
}

float4x4 GetCameraMatrix(Camera c)
{
	float3 x = cross(c.viewDir, c.upVector);

	float4x4 m;
	m[0] = float4(x, 0);
	m[1] = float4(c.upVector, 0);
	m[2] = float4(c.viewDir, 0);
	m[3] = float4(float3(0.0f, 0.0f, 0.0f), 1);

	m = transpose(m);

	return mul(CreateTranslation(-c.pos), m);
}

//heavily based on mjp shadow sample
float4x4 EfficientShadowSplitMatrix(int idx, float4x4 invVP, float2 frustumSplits[4], Camera cam, Camera lightCam, float shadowMapSize)
{
	//frustum corners in ndc space
	float3 ndcFrustumCorners[8] =
	{
		float3(-1.0f, 1.0f, 0.0f),
		float3(1.0f, 1.0f, 0.0f),
		float3(1.0f, -1.0f, 0.0f),
		float3(-1.0f, -1.0f, 0.0f),
		float3(-1.0f, 1.0f, 1.0f),
		float3(1.0f, 1.0f, 1.0f),
		float3(1.0f, -1.0f, 1.0f),
		float3(-1.0f, -1.0f, 1.0f),
	};

	float3 wsFrustumCorners[8];

	//calculate world space frustum corners
	for (int c = 0; c < 8; ++c)
	{
		float4 trans = mul(float4(ndcFrustumCorners[c], 1), invVP);
		wsFrustumCorners[c] = trans.xyz / trans.w;
	}

	//calc this splits frustum corners
	//each split is made up of 4 near + 4 far corners
	//calc ray between near-far ws corners, scale by split dist
	//add scaled to near corner
	for (int i = 0; i < 4; ++i)
	{
		float3 cornerRay = wsFrustumCorners[i + 4] - wsFrustumCorners[i];
		float3 nearCornerRay = cornerRay * frustumSplits[idx].x;
		float3 farCornerRay = cornerRay * frustumSplits[idx].y;
		wsFrustumCorners[i + 4] = wsFrustumCorners[i] + farCornerRay;
		wsFrustumCorners[i] = wsFrustumCorners[i] + nearCornerRay;
	}

	//calc split centroid center
	float3 centroidCenter = float3(0, 0, 0);
	for (int j = 0; j < 8; ++j)
	{
		centroidCenter += wsFrustumCorners[j];
	}
	centroidCenter /= 8.0f;

	float3 up = normalize(cross(cam.viewDir, cam.upVector));

	float3 lightCamPos = centroidCenter;
	float3 lightCamLookat = centroidCenter + lightCam.viewDir;

	Camera newLightCam = LookatFunc(lightCamPos, lightCamLookat, up);
	float4x4 lightViewMat = GetCameraMatrix(newLightCam);

	float3 mins = float3(FLT_MAX, FLT_MAX, FLT_MAX);
	float3 maxes = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int k = 0; k < 8; ++k)
	{
		float3 corner = (mul(float4(wsFrustumCorners[k], 1), lightViewMat)).xyz;
		mins = min(mins, corner);
		maxes = max(maxes, corner);
	}

	float3 maxExtents = maxes;
	float3 minExtents = mins;

	float filterSize = 3;
	float scale = (shadowMapSize + filterSize) / shadowMapSize;

	maxExtents *= scale;
	maxExtents.z /= scale;
	minExtents *= scale;
	minExtents.z /= scale;

	float3 cascadeExtents = maxExtents - minExtents;

	float3 camPos = centroidCenter + lightCam.viewDir * -abs(minExtents.z);

	float4x4 splitOrthoMatrix = Ortographic(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, cascadeExtents.z);

	Camera splitShadowCam = LookatFunc(camPos, centroidCenter, up);

	outputTex3[uint2(idx * 3 + 0, 0)] = float4(camPos, cascadeExtents.x);
	outputTex3[uint2(idx * 3 + 1, 0)] = float4(lightCam.viewDir, cascadeExtents.y);
	outputTex3[uint2(idx * 3 + 2, 0)] = float4(splitShadowCam.upVector, cascadeExtents.z);

	return mul(GetCameraMatrix(splitShadowCam), splitOrthoMatrix);
}

float LogPartitionFromRange(uint part, float minz, float maxz)
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

void CalcFrustumSplits(float camNear, float camFar, out float2 frustumSplits[4])
{
	for (int c = 0; c < 4; ++c)
	{
		frustumSplits[c].x = LogPartitionFromRange(c, camNear, camFar);
		frustumSplits[c].y = LogPartitionFromRange(c + 1, camNear, camFar);
	}
}

void Init(uint2 dispatchThreadId, uint groupIndex)
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	if (any(dispatchThreadId.xy >= inputTexSize.xy))
		return;

	float2 data = inputTex.Load(int3(dispatchThreadId.xy, 0)).xy;

	localData[groupIndex].x = min(data.x, localData[groupIndex].x);
	localData[groupIndex].y = max(data.y, localData[groupIndex].y);
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
	{ //INIT
		uint3 inputTexSize;
		inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

		localData[groupIndex] = float2(1.0f, 0.0f);

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
		//construct matrix here
		float near, far;

		{
			float minDepth = localData[0].x;
			float maxDepth = localData[0].y;

			float linearMinDepth = LinearizeDepth(minDepth, uniforms.camNear, uniforms.camFar);
			float linearMaxDepth = LinearizeDepth(maxDepth, uniforms.camNear, uniforms.camFar);
			
			near = linearMinDepth * uniforms.camFar;
			far = linearMaxDepth * uniforms.camFar;
		}

		float4x4 lightMvp[4];
		float4x4 shadowMat[4];

		{
			float2 normFrustumSplits[4];

			{
				float2 frustumSplits[4];
				CalcFrustumSplits(near, far, frustumSplits);

				for (int c = 0; c < 4; ++c)
					normFrustumSplits[c] = (frustumSplits[c] - uniforms.camNear) / (uniforms.camFar - uniforms.camNear);

				for (uint d = 0; d < 4; ++d)
					outputTex2[uint2(d,0)] = frustumSplits[d];
			}

			{
				Camera cam, lightCam;
				cam.pos = uniforms.camPos.xyz;
				cam.viewDir = uniforms.camViewDir.xyz;
				cam.upVector = uniforms.camUpVector.xyz;

				lightCam.pos = uniforms.lightCamPos.xyz;
				lightCam.viewDir = uniforms.lightCamViewDir.xyz;
				lightCam.upVector = uniforms.lightCamUpVector.xyz;
				for (int c = 0; c < 4; ++c)
					lightMvp[c] = EfficientShadowSplitMatrix(c, uniforms.invVP, normFrustumSplits, cam, lightCam, uniforms.texSize);
			}
		}

		for (int c = 0; c < 4; ++c)
			shadowMat[c] = mul(mul(uniforms.invMv, lightMvp[c]), uniforms.biasMx);

		for (uint d = 0; d < 4; ++d)
		{
			for (uint e = 0; e < 4; ++e)
			{
				outputTex0[uint2(d * 4 + e,0)] = lightMvp[d][e];
				outputTex1[uint2(d * 4 + e,0)] = shadowMat[d][e];
			}
		}
	}
}
