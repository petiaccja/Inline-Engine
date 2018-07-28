/*
 * Volumetric lighting shader
 * Input: depth texture, lit opaque scene texture, culled sdfs per tile
 * Output: scattered light blended on top of scene
 */

Texture2D depthTex : register(t0);
Texture2D<float4> inputColorTex : register(t1);
Texture2D<uint> sdfCullTex : register(t2);
Texture2D<uint> lightCullTex : register(t3);
RWTexture2D<float4> volDstTex0 : register(u0);
RWTexture2D<float4> volDstTex1 : register(u1);
RWTexture2D<float4> dstTex : register(u2);

#include "CSMSample"

struct SdfData
{
	float4 vsPosition;
	float radius;
	float3 dummy;
};

struct LightData
{
	float4 diffuseLightColor;
	float4 vsPosition;
	float attenuationEnd;
	float3 dummy;
};

struct Uniforms
{
	SdfData sd[10];
	LightData ld[10];
	float4x4 v, p;
	float4x4 invVP, oldVP;
	float camNear, camFar, dummy1, dummy2;
	uint numSdfs, numWorkgroupsX, numWorkgroupsY; float haltonFactor;
	float4 sunDirection;
	float4 sunColor;
	float4 camPos;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

static const float epsilon = 0.0025;
static const float pi = 3.14159265;

#define FLT_MAX 3.402823466e+38

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

float LinearizeDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vsZrecon = B / (zndc - A);

	//range: [0...far]
	return vsZrecon;// / far;
};

float Sphere(float3 rayOri, float3 rayDir, float3 center, float radius)
{
	return distance(rayOri, center) - radius;
}

float3 SphereNormal(float3 rayOri, float3 rayDir, float3 spherePos, float sphereRad)
{
	return normalize(float3(
		sphere(rayOri + float3(epsilon, 0, 0), rayDir, spherePos, sphereRad) - Sphere(rayOri + float3(-epsilon, 0, 0), rayDir, spherePos, sphereRad),
		sphere(rayOri + float3(0, epsilon, 0), rayDir, spherePos, sphereRad) - Sphere(rayOri + float3(0, -epsilon, 0), rayDir, spherePos, sphereRad),
		sphere(rayOri + float3(0, 0, epsilon), rayDir, spherePos, sphereRad) - Sphere(rayOri + float3(0, 0, -epsilon), rayDir, spherePos, sphereRad)
	));
}

//neutral for now
float PhaseFunction()
{
	return 1.0 / (4.0*3.14);
}

float GetNextStepSize(float currStep, float maxSteps, float currPos, float minz, float maxz)
{
	float z = maxz;
	float ratio = maxz / minz;
	float power = currStep / maxSteps;
	z = minz * pow(ratio, power);

	return z - currPos;
}

[numthreads(LOCAL_SIZE_X, LOCAL_SIZE_Y, 1)]
void CSMain(
	uint3 groupId : SV_GroupID, //WorkGroupId
	uint3 groupThreadId : SV_GroupThreadID, //LocalInvocationId
	uint3 dispatchThreadId : SV_DispatchThreadID, //GlobalInvocationId
	uint groupIndex : SV_GroupIndex //LocalInvocationIndex
)
{
	uint3 inputTexSize;
	inputColorTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	//[0...1]
	float ndcDepth = depthTex.Load(int3(dispatchThreadId.xy, 0)).x;
	//[0...far]
	float linearDepth = LinearizeDepth(ndcDepth, uniforms.camNear, uniforms.camFar);

	uint localNumOfSdfs = sdfCullTex.Load(int3(groupId.x * uniforms.numWorkgroupsY + groupId.y, 0, 0));
	uint localNumOfLights = lightCullTex.Load(int3(groupId.x * uniforms.numWorkgroupsY + groupId.y, 0, 0));

	float4 outColor = inputColorTex.Load(int3(dispatchThreadId.xy, 0));

	float2 uv = (float2(dispatchThreadId.xy) + 0.5) / float2(inputTexSize.xy);
	uv.y = 1.0 - uv.y;

	float3 rayDir, rayOri;

	float4 rayOriTmp = float4(uv * 2 - 1, 0, 1); //ndc pos on near plane
	rayOriTmp = mul(rayOriTmp, uniforms.invVP);
	rayOri = rayOriTmp.xyz / rayOriTmp.w;
	rayDir = normalize(rayOri - uniforms.camPos.xyz);

	float4 reprojPosTmp = float4(uv * 2 - 1, ndcDepth, 1); //ndc pos on near plane
	reprojPosTmp = mul(reprojPosTmp, uniforms.invVP);
	reprojPosTmp /= reprojPosTmp.w;
	float4 reprojPos = mul(reprojPosTmp, uniforms.oldVP);
	reprojPos /= reprojPos.w;

	float t = 0;
	float maxDist = min(linearDepth - 0.1, 32.0);

	float transmittance = 1.0;
	float3 scatteredLight = float3(0, 0, 0);

	float maxSteps = 64.0;
	float initialSkip = 0.1;
	//float stepSize = getNextStepSize(0, maxSteps, initialSkip, initialSkip, maxDist);
	float stepSize = maxDist / maxSteps;

	//initial step
	t += initialSkip;

	//2x2 initial jitter pattern 
	float jitter = (dispatchThreadId.x + dispatchThreadId.y) % 2 ? stepSize*0.5 : 0.0;

	t += jitter;

	for (float d = 0; d < maxSteps; ++d)
	{
		float3 evalPos = rayOri + rayDir * (t + stepSize * uniforms.haltonFactor);

		float3 vsEvalPos = mul(float4(evalPos, 1.0), uniforms.v).xyz;

		if (t > maxDist || transmittance < 0.0001)
		{
			break;
		}

		float muS = 0.0; //calculate this from SDFs
		float muA = 0.0; //could calc this from SDFs as well
		float muE = 0.0;
		float phase = 0.0;
		float phaseCounter = 0.0;

		for (uint c = 0; c < localNumOfSdfs; ++c)
		{
			uint index = sdfCullTex.Load(int3(groupId.x * uniforms.numWorkgroupsY + groupId.y, c + 1, 0));
			//world space
			float3 pos = uniforms.sd[index].vsPosition;
			float radius = uniforms.sd[index].radius;

			//TODO sample texture or something...
			float dist = max(-Sphere(evalPos, rayDir, pos, radius), 0.0);

			//add together scattering
			muS += dist * 30.0 / radius;

			//TODO: get this from somewhere
			phase += PhaseFunction();
			phaseCounter++;
		}

		//constant world fog
		muS += 0.2;
		phase += PhaseFunction();
		phaseCounter++;

		if (phaseCounter > 0.0)
		{
			phase /= phaseCounter;
		}

		muE = max(muA + muS, 0.0001);

		float3 lighting = float3(0, 0, 0);

		for (uint e = 0; e < localNumOfLights; ++e)
		{
			uint index = lightCullTex.Load(int3(groupId.x * uniforms.numWorkgroupsY + groupId.y, e + 1, 0));
			//world space
			float3 pos = uniforms.ld[index].vsPosition;
			float attEnd = uniforms.ld[index].attenuationEnd;
			float3 lightColor = uniforms.ld[index].diffuseLightColor;

			float3 lightDir = pos - evalPos.xyz;
			float distance = length(lightDir);
			lightDir = normalize(lightDir);

			float attenuation = clamp((attEnd - distance) / attEnd, 0.0, 1.0);

			//TODO sample shadow map
			lighting += lightColor * attenuation * 10.0;
		}

		float shadow = GetShadow(float4(vsEvalPos, 1.0)).x;

		//eval directional light
		//TODO sample shadow map
		lighting += uniforms.sunColor * 10.0 * shadow;

		float3 S = lighting * muS * phase; //TODO: volumetric shadow maps
		float3 Sint = (S - S * exp(-muE * stepSize)) / muE; // integrate along the current step segment
		scatteredLight += transmittance * Sint; // accumulate and also take into account the transmittance from previous steps

		// Evaluate transmittance to view independentely
		transmittance *= exp(-muE * stepSize);

		t += stepSize;

		//stepSize = getNextStepSize(d+1, maxSteps, t, initialSkip, maxDist);
	}

	//outColor = float4(local_num_of_sdfs, 0, 0, 1);
	//outColor = float4(linear_depth, linear_depth, linear_depth, linear_depth);

	//blend 5% of current result into accumulated
	float blendFactor = 0.05;
	float4 result;
	//result = lerp(volDstTex1[dispatchThreadId.xy], float4(scatteredLight, transmittance), blendFactor);
	int2 reprojCoord = (float2(reprojPos.x, -reprojPos.y) * 0.5 + 0.5) * float2(inputTexSize.xy);
	if (reprojCoord.x >= 0 && reprojCoord.x < inputTexSize.x &&
		reprojCoord.y >= 0 && reprojCoord.y < inputTexSize.y)
	{
		float4 prevResult = volDstTex1[reprojCoord];
		//lerp: x*(1-s) + y*s
		float4 blendedResult = lerp(prevResult, float4(scatteredLight, transmittance), blendFactor);
		//if there's no history available (because opaque occluded area) then just use current
		result = lerp(float4(scatteredLight, transmittance), blendedResult, exp(-8.0*prevResult.w));
		//result = blendedResult;
	}
	else
	{
		result = float4(scatteredLight, transmittance);
	}
	volDstTex0[dispatchThreadId.xy] = result;
	dstTex[dispatchThreadId.xy] = outColor * result.w + float4(result.xyz, 0.0); //TODO volumetric shadows
	//dstTex[dispatchThreadId.xy] = float4(float2(reprojPos.xy*0.5+0.5) * int2(inputTexSize.xy), 0, 1);
}
