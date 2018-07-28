/*
* Shadow layered penumbra shader
* Input: Shadow map, minfilter texture, depth texture
* Output: layered penumbra texture, layered shadow texture
* (penumbra size in each layer, binary shadow value)
*/

struct Uniforms
{
	float4x4 invV;
	float4 farPlaneData0, farPlaneData1;
	float lightSize, nearPlane, farPlane, dummy;
	float4 vsLightPos;
	float2 direction;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

#define CSM_EXTENDED_INFO
#include "CSMSample.hlsl"
#define POINT_EXTENDED_INFO
#include "PointLightShadowMapSample.hlsl"

Texture2D inputTex0 : register(t0); //depth texture
Texture2DArray<float> inputTex1 : register(t1); //csm minfilter map
TextureCube<float> inputTex2 : register(t2); //cube minfilter map
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

struct PS_Output
{
	float4 layeredShadows: SV_Target0;
	float4 layeredPenumbra: SV_Target1;
};


float EstimatePenumbra( float receiver, float blocker )
{
	//need to set min to roughly near plane,
	//as we render stuff into cascades even when outside cascade
	//to make sure we have all blockers
	blocker = max(blocker, 0.1); 
	return clamp(abs(receiver - blocker) / blocker, 0.0, 1.0);
}

PS_Input VSMain(uint vertexId : SV_VertexID)
{
	// Triangle strip based on vertex id
	// 3-----2
	// |   / |
	// | /   |
	// 1-----0
	// 0: (1, 0)
	// 1: (0, 0)
	// 2: (1, 1)
	// 3: (0, 1)
    PS_Input output;

    output.texCoord.x = (vertexId & 1) ^ 1; // 1 if bit0 is 0.
    output.texCoord.y = vertexId >> 1; // 1 if bit1 is 1.

    float2 posL = output.texCoord.xy * 2.0f - float2(1, 1);
    output.position = float4(posL, 0.5f, 1.0f);
    output.texCoord.y = 1.f - output.texCoord.y;

    return output;
}

PS_Output PSMain(PS_Input input) : SV_TARGET
{
	PS_Output output;
	
	uint3 inputTexSize;
	inputTex0.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	float depth = inputTex0.Sample(samp0, input.texCoord).x;
	
	if(depth > 0.999)
	{
		discard;
	}
	
	float linearDepth = LinearizeDepth(depth, uniforms.nearPlane, uniforms.farPlane);
	float3 farPlaneLL = uniforms.farPlaneData0.xyz;
	float3 farPlaneUR = float3(uniforms.farPlaneData0.w, uniforms.farPlaneData1.xy);

	float2 uv = float2(input.texCoord.x, 1 - input.texCoord.y);
	float3 vsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, uv) / uniforms.farPlane, 1.0) * linearDepth;
	
	float4 penumbra = float4(0.0, 0.0, 0.0, 0.0);
	float4 shadow = float4(0.0, 0.0, 0.0, 0.0);
	
	{ //csm 
		//blocker comes from minfilter tex
		float4 shadowCoord;
		{
			shadow.x = GetCsmShadow(float4(vsPos, 1.0), shadowCoord).x;
		}
		
		float blocker = clamp(inputTex1.SampleLevel(samp0, GetShadowUv(shadowCoord.xy, shadowCoord.w), 0.0).x, 0.0, 1.0);
		float shadowCoordZ = clamp(shadowCoord.z, 0.0, 1.0);
		
		float ls = uniforms.lightSize;
		penumbra.x = EstimatePenumbra( shadowCoordZ, blocker ) * ls;// * ls;
		//penumbra.z = shadowCoordZ;
		//penumbra.w = blocker;
	}
	
	{ //point lights
		{ //TODO for each point light
			//TODO check for light bounds
			//blocker comes from minfilter tex
			float4 shadowCoord;
			{
				float3 lightDir = uniforms.vsLightPos.xyz - vsPos;
				float distance = length(lightDir);
				shadow.y = GetPointLightShadow(-lightDir, distance, shadowCoord).x;
				shadowCoord.w = distance;
			}
			
			float blocker = inputTex2.SampleLevel(samp0, shadowCoord.xyz, 0.0).x;
			blocker = LinearizeDepth(blocker, 0.1, 100.0);
			float shadowCoordZ = shadowCoord.w;
			
			float ls = uniforms.lightSize;
			penumbra.y = EstimatePenumbra( shadowCoordZ, blocker ) * ls;// * ls;
			//penumbra.z = shadowCoordZ;
			//penumbra.w = blocker;
		}
	}
	
	output.layeredShadows = shadow;//float4(0,0,0,0);
	output.layeredPenumbra = penumbra;

	return output;
}
