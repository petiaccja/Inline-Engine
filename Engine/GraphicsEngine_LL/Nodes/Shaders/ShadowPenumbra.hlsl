/*
* Shadow layered penumbra shader
* Input: Shadow map, minfilter texture, depth texture
* Output: layered penumbra texture, layered shadow texture
* (penumbra size in each layer, binary shadow value)
*/

struct Uniforms
{
	float2 direction;
	float lightSize;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex0 : register(t0); //depth texture
TextureCube inputTex1 : register(t1); //shadow cube map
SamplerState samp0 : register(s0);

#include "CSMSample.hlsl"

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
	
	{ //csm 
		//blocker comes from minfilter tex
		//ls is light size
		float penumbra = abs(estimate_penumbra( abs(lsprojz), abs(blocker) )) * ls * ls;
		float shadow = float(distance_from_light < shadow_coord.z);
	}
	
	{ //point lights
		{ //TODO for each point light
			//blocker comes from minfilter tex
			//ls is light size
			float penumbra = abs(estimate_penumbra( abs(lsprojz), abs(blocker) )) * ls * ls;
			float shadow = float(distance_from_light < shadow_coord.z);
		}
	}
	
	output.layeredShadows = float4(0,0,0,0);
	output.layeredPenumbra = float4(0,0,0,0);

	return output;
}
