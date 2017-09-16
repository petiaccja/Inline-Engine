struct Uniforms
{
	float4x4 model, viewProj;
	float3 voxelCenter; float voxelSize;
	int voxelDimension;
};


ConstantBuffer<Uniforms> uniforms : register(b0);
RWTexture3D<float4> voxelTex : register(u0);

struct GS_Input
{
	float4 position	: SV_POSITION;
	float2 texcoord	: TEXCOORD;
};

struct PS_Input
{
	float4 position	: SV_POSITION;
	float3 voxelPos	: POSITION3D;
	float2 texcoord	: TEXCOORD;
};


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
		float3 voxelPos = (input[i].position.xyz - uniforms.voxelCenter) / (uniforms.voxelSize * uniforms.voxelDimension);

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

void PSMain(PS_Input input)
{
	//TODO sample texture here
	float4 albedo = float4(input.texcoord, 0.0, 1.0);

	uint3 target = floor(input.voxelPos);
	
	//TODO write out normal, etc
	//TODO atomic average to avoid flicker...

	voxelTex[target.xyz] = albedo;
}
