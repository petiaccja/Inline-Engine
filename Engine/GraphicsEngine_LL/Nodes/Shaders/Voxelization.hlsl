/*
* Voxelization shader
* Input: Mesh + model matrix
* Output: voxels inserted into R32U 3D voxel texture UAV
*/

struct Uniforms
{
	float4x4 model;
	float3 voxelCenter; float voxelSize;
	int voxelDimension; int inputMipLevel;
};


ConstantBuffer<Uniforms> uniforms : register(b0);
RWTexture3D<uint> voxelTex : register(u0);
RWTexture3D<uint> voxelSecondaryTex : register(u1);
Texture2D<float4> albedoTex : register(t0);

SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);

struct GS_Input
{
	float4 position	: SV_POSITION;
	float2 texCoord	: TEXCOORD0;
};

struct PS_Input
{
	float4 position	: SV_POSITION;
	float3 voxelPos	: TEXCOORD0;
	float2 texCoord	: TEXCOORD1;
};

//encodes rgba8 color [0...255] into uint
uint encodeColor(float4 color)
{
	return uint(color.x) | uint(color.y) << 8 | uint(color.z) << 16 | uint(color.w) << 24;
}

//decode rgba8 color [0...1] from uint
float4 decodeColor(uint color)
{
	return float4(
		float(color & 0x000000ff) / 255.0,
		float((color >> 8) & 0x000000ff) / 255.0,
		float((color >> 16) & 0x000000ff) / 255.0,
		float((color >> 24) & 0x000000ff) / 255.0
		);
}

GS_Input VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEX_COORD)
{
	GS_Input result;

    result.position = mul(position, uniforms.model);
	result.texCoord = texCoord.xy;

	return result;
}

[maxvertexcount(3)]
void GSMain(triangle GS_Input input[3], inout TriangleStream<PS_Input> OutputStream)
{
	PS_Input output = (PS_Input)0;

	//calc triangle face normal
	float3 p1 = input[1].position.xyz - input[0].position.xyz;
	float3 p2 = input[2].position.xyz - input[0].position.xyz;
	float3 normal = abs(normalize(cross(p1, p2)));

	//dominant axis selection
	//0 = x, 1 = y, 2 = z
	uint axis = 0;
	if (normal.x >= normal.y && normal.x >= normal.z)
	{
		axis = 0;
	}
	else if (normal.y >= normal.x && normal.y >= normal.z)
	{
		axis = 1;
	}
	else
	{
		axis = 2;
	}
	
	for (uint i = 0; i<3; i++)
	{
		//voxel space [-1...1]
		float3 voxelPos = (input[i].position.xyz - uniforms.voxelCenter) / (uniforms.voxelSize * uniforms.voxelDimension * 0.5);

		//target voxel coords [0...255]
		float3 insertionPos = (voxelPos * 0.5 + 0.5) * uniforms.voxelDimension;

		//project onto dominant axis
		if (axis == 0)
		{
			voxelPos = voxelPos.zyx;
		}
		else if (axis == 1)
		{
			voxelPos = voxelPos.xzy;
		}

		//these coords are only used for rasteriation
		output.position = float4(voxelPos.xy, 1, 1);
		output.voxelPos = insertionPos;
		output.texCoord = input[i].texCoord;

		OutputStream.Append(output);
	}

	OutputStream.RestartStrip();
}

void atomicAdd(RWTexture3D<uint> tex, float val, uint3 target)
{
	uint newVal = asuint(val);
	uint prevVal = 0; uint curVal;
	// Loop as long as destination value gets changed by other threads
	[allow_uav_condition]
	while (true)
	{
		InterlockedCompareExchange(tex[target.xyz], prevVal, newVal, curVal);
		if (curVal == prevVal)
		{
			break;
		}

		prevVal = curVal;
		newVal = asuint(val + asfloat(curVal));
	}
}

void atomicAvg(RWTexture3D<uint> tex, float4 color, uint3 target)
{
	//w channel is used for atomic moving average
	//so keep it 1
	color.w = 1;

	color.rgb *= 255.0;
	uint newVal = encodeColor(color);
	uint prevStoredVal = 0; uint curStoredVal;
	// Loop as long as destination value gets changed by other threads
	[allow_uav_condition]
	while (true)
	{
		//Atomically compares the destination with the comparison value. 
		//If they are identical, the destination is overwritten with the input value. 
		//The original value is set to the destination's original value.
		//                          dest                  compare        input      original
		InterlockedCompareExchange(tex[target.xyz], prevStoredVal, newVal, curStoredVal);
		if (curStoredVal == prevStoredVal)
		{
			break;
		}

		prevStoredVal = curStoredVal;
		float4 rval = decodeColor(curStoredVal);
		rval.xyz = (rval.xyz* rval.w); // Denormalize
		float4 curValF = rval + color; // Add new value
		curValF.xyz /= (curValF.w); // Renormalize
		newVal = encodeColor(curValF);
	}
}

void PSMain(PS_Input input)
{
	//float4 albedo = float4(input.texCoord, 0.0, 1.0);
	float4 albedo = albedoTex.Sample(samp0, input.texCoord);

	uint3 target = input.voxelPos;
	
	//TODO flicker for small objects

	//TODO write out texture color, opacity, normal, roughness metalness

	//InterlockedMax(voxelTex[target.xyz], encodeColor(albedo*255.0));
	atomicAvg(voxelTex, albedo, target);
	//atomicAvg(voxelSecondaryTex, albedo.wwww, target);
	atomicAvg(voxelSecondaryTex, float4(1,1,1,1), target);
}
