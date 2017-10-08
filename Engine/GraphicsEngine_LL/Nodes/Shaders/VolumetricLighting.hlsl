/*
 * Volumetric lighting shader
 * Input: depth texture, lit opaque scene texture, culled sdfs per tile
 * Output: scattered light blended on top of scene
 */

Texture2D depthTex : register(t0);
Texture2D<float4> inputColorTex : register(t1);
Texture2D<uint> sdfCullTex : register(t2);
Texture2D<uint> lightCullTex : register(t3);
RWTexture2D<float4> dstTex : register(u0);

#include "CSMSample"

struct sdf_data
{
	float4 vs_position;
	float radius;
	float3 dummy;
};

struct light_data
{
	float4 diffuseLightColor;
	float4 vs_position;
	float attenuation_end;
	float3 dummy;
};

struct Uniforms
{
	sdf_data sd[10];
	light_data ld[10];
	float4x4 v, p;
	float4x4 invVP;
	float cam_near, cam_far, dummy1, dummy2;
	uint num_sdfs, num_workgroups_x, num_workgroups_y; float dummy;
	float4 sun_direction;
	float4 sun_color;
	float4 cam_pos;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

static const float epsilon = 0.0025;
static const float pi = 3.14159265;

#define FLT_MAX 3.402823466e+38

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

float linearize_depth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vs_zrecon = B / (zndc - A);

	//range: [0...far]
	return vs_zrecon;// / far;
};

float sphere(float3 rayOri, float3 rayDir, float3 center, float radius)
{
	return distance(rayOri, center) - radius;
}

float3 sphereNormal(float3 rayOri, float3 rayDir, float3 spherePos, float sphereRad)
{
	return normalize(float3(
		sphere(rayOri + float3(epsilon, 0, 0), rayDir, spherePos, sphereRad) - sphere(rayOri + float3(-epsilon, 0, 0), rayDir, spherePos, sphereRad),
		sphere(rayOri + float3(0, epsilon, 0), rayDir, spherePos, sphereRad) - sphere(rayOri + float3(0, -epsilon, 0), rayDir, spherePos, sphereRad),
		sphere(rayOri + float3(0, 0, epsilon), rayDir, spherePos, sphereRad) - sphere(rayOri + float3(0, 0, -epsilon), rayDir, spherePos, sphereRad)
	));
}

//neutral for now
float phaseFunction()
{
	return 1.0 / (4.0*3.14);
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

	//[0...far]
	float linear_depth = linearize_depth(depthTex.Load(int3(dispatchThreadId.xy, 0)).x, uniforms.cam_near, uniforms.cam_far);

	uint local_num_of_sdfs = sdfCullTex.Load(int3(groupId.x * uniforms.num_workgroups_y + groupId.y, 0, 0));
	uint local_num_of_lights = lightCullTex.Load(int3(groupId.x * uniforms.num_workgroups_y + groupId.y, 0, 0));

	float4 outColor = inputColorTex.Load(int3(dispatchThreadId.xy, 0));

	float2 uv = (float2(dispatchThreadId.xy)+0.5) / float2(inputTexSize.xy);
	uv.y = 1.0 - uv.y;

	float3 rayDir, rayOri;

	float4 rayOriTmp = float4(uv * 2 - 1, 0, 1); //ndc pos on near plane
	rayOriTmp = mul(rayOriTmp, uniforms.invVP);
	rayOri = rayOriTmp.xyz / rayOriTmp.w;

	rayDir = normalize(rayOri - uniforms.cam_pos.xyz);

	float t = 0;
	float maxDist = linear_depth;

	float transmittance = 1.0;
	float3 scatteredLight = float3(0, 0, 0);

	float stepSize = 0.1;
	float steps = maxDist / stepSize;

	for (float d = 0; d < steps; ++d)
	{
		float3 evalPos = rayOri + rayDir * t;

		float3 vsEvalPos = mul(float4(evalPos, 1.0), uniforms.v).xyz;

		if(t > maxDist)
		{
			break;
		}

		float muS = 0.0; //calculate this from SDFs
		float muA = 0.0; //could calc this from SDFs as well
		float muE = 0.0;
		float phase = 0.0; 
		float phaseCounter = 0.0;

		for (uint c = 0; c < local_num_of_sdfs; ++c)
		{
			uint index = sdfCullTex.Load(int3(groupId.x * uniforms.num_workgroups_y + groupId.y, c + 1, 0));
			//world space
			float3 pos = uniforms.sd[index].vs_position;
			float radius = uniforms.sd[index].radius;

			//TODO sample texture or something...
			float dist = max(-sphere(evalPos, rayDir, pos, radius), 0.0);

			//add together scattering
			muS += dist * 30.0 / radius;

			//TODO: get this from somewhere
			phase += phaseFunction();
			phaseCounter++;
		}

		//constant world fog
		muS += 0.2;
		phase += phaseFunction();
		phaseCounter++;

		if (phaseCounter > 0.0)
		{
			phase /= phaseCounter;
		}

		muE = max(muA + muS, 0.0001);

		float3 lighting = float3(0, 0, 0);

		for (uint e = 0; e < local_num_of_lights; ++e)
		{
			uint index = lightCullTex.Load(int3(groupId.x * uniforms.num_workgroups_y + groupId.y, e + 1, 0));
			//world space
			float3 pos = uniforms.ld[index].vs_position;
			float att_end = uniforms.ld[index].attenuation_end;
			float3 light_color = uniforms.ld[index].diffuseLightColor;

			float3 light_dir = pos - evalPos.xyz;
			float distance = length(light_dir);
			light_dir = normalize(light_dir);

			float attenuation = clamp((att_end - distance) / att_end, 0.0, 1.0);

			//TODO sample shadow map
			lighting += light_color * attenuation * 10.0;
		}

		//eval directional light
		//TODO sample shadow map
		lighting += uniforms.sun_color * 10.0;

		float shadow = get_shadow(float4(vsEvalPos, 1.0)).x;

		float3 S = lighting * shadow * muS * phase; //TODO: volumetric shadow maps
		float3 Sint = (S - S * exp(-muE * stepSize)) / muE; // integrate along the current step segment
		scatteredLight += transmittance * Sint; // accumulate and also take into account the transmittance from previous steps

		// Evaluate transmittance to view independentely
		transmittance *= exp(-muE * stepSize);

		t += stepSize;
	}

	//outColor = float4(local_num_of_sdfs, 0, 0, 1);
	//outColor = float4(linear_depth, linear_depth, linear_depth, linear_depth);

	dstTex[dispatchThreadId.xy] = float4(outColor.xyz * transmittance + scatteredLight, 1.0); //TODO volumetric shadows
}
