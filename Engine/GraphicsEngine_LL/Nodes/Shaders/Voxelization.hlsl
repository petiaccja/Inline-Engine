/*
* Voxelization shader
* Input: Mesh + model matrix
* Output: voxels inserted into R32U 3D voxel texture UAV
*/

struct Uniforms
{
	float4x4 model, viewProj;
	float3 voxelCenter; float voxelSize;
	int voxelDimension;
};


ConstantBuffer<Uniforms> uniforms : register(b0);
RWTexture3D<uint> voxelTex : register(u0);

struct GS_Input
{
	float4 position	: SV_POSITION;
	float2 texcoord	: TEXCOORD0;
};

struct PS_Input
{
	float4 position	: SV_POSITION;
	float3 voxelPos	: TEXCOORD0;
	float2 texcoord	: TEXCOORD1;
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
	result.texcoord = texCoord.xy;

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
		output.texcoord = input[i].texcoord;

		OutputStream.Append(output);
	}

	OutputStream.RestartStrip();
}

//doesnt work (driver reset)
void atomicAvg(float4 color, uint3 target)
{
	color *= 255.0;
	uint newVal = encodeColor(color);
	uint prevStoredVal = 0; uint curStoredVal;
	// Loop as long as destination value gets changed by other threads
	[allow_uav_condition]
	while (true)
	{
		InterlockedCompareExchange(voxelTex[target.xyz], prevStoredVal, newVal, curStoredVal);
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
	//TODO sample texture here
	float4 albedo = float4(input.texcoord, 0.0, 1.0);

	uint3 target = input.voxelPos;
	
	//TODO write out texture color, normal, opacity, material?

	InterlockedMax(voxelTex[target.xyz], encodeColor(albedo*255.0));
	//atomicAvg(albedo, target);
}
