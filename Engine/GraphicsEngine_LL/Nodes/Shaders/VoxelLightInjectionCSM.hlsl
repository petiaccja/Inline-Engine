/*
* Voxel Light Injection from CSM shader
* Input: Cascaded shadow maps, voxelized scene in 3D texture
* Output: voxels injected into light voxel 3D texture
*/

struct Uniforms
{
	float4x4 model, viewProj, invView;
	float3 voxelCenter; float voxelSize;
	float4 farPlaneData0, farPlaneData1;
	float4 wsCamPos;
	int voxelDimension; int inputMipLevel; int outputMipLevel; int dummy;
	float nearPlane, farPlane;
};


ConstantBuffer<Uniforms> uniforms : register(b0);
//RWTexture3D<uint> voxelTex : register(u0);
RWTexture3D<uint> voxelLightTex : register(u1);
Texture3D<float4> voxelTex : register(t0);
Texture2DArray<float> shadowCSMTex : register(t1);
Texture2D<float4> shadowCSMExtentsTex : register(t2);

SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD0;
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

float3 getCSMTexelWSPos(float depth, float3 camPos, float3 camViewDir, float3 camUpDir, float3 extents, float2 texCoord)
{
	float3 camRightDir = normalize(cross(camViewDir, camUpDir));

	//x right
	//y forward
	//z up

	float3 wsPos = camPos +
		depth * extents.z * camViewDir +
		(texCoord.x - 0.5) * extents.x * camRightDir + 
		(1-texCoord.y - 0.5) * extents.y * camUpDir; 

	return wsPos;
	//return float3((texCoord.x - 0.5) * extents.x * 0.5, (texCoord.y - 0.5) * extents.y * 0.5, depth * extents.z);
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

void PSMain(PS_Input input)
{
	//TODO combine light color, voxel albedo etc. into a lit voxel value
	//store it into the light voxel UAV

	for (int c = 0; c < 4; ++c)
	{ //for each cascade
		float depth = shadowCSMTex.Sample(samp0, float3(input.texCoord, float(c)));

		if (depth<0.0001 || depth >0.9999)
		{
			continue;
		}

		float3 camPos, camViewDir, camUpDir, extents;
		float4 tmp;
		tmp = shadowCSMExtentsTex.Load(int3(c * 3 + 0, 0, 0));
		camPos = tmp.xyz;
		extents.x = tmp.w;
		tmp = shadowCSMExtentsTex.Load(int3(c * 3 + 1, 0, 0));
		camViewDir = tmp.xyz;
		extents.y = tmp.w;
		tmp = shadowCSMExtentsTex.Load(int3(c * 3 + 2, 0, 0));
		camUpDir = tmp.xyz;
		extents.z = tmp.w;
		
		float3 wsPos = getCSMTexelWSPos(depth, camPos, camViewDir, camUpDir, extents, input.texCoord); 

		//voxel space [-1...1]
		float3 voxelPos = (wsPos - uniforms.voxelCenter) / (uniforms.voxelSize * uniforms.voxelDimension * 0.5);

		//target voxel coords [0...255]
		uint3 insertionPos = (voxelPos * 0.5 + 0.5) * uniforms.voxelDimension;

		//float4 albedo = decodeColor(voxelTex[insertionPos]);
		float4 albedo = voxelTex[insertionPos];

		if (albedo.w < 0.001)
		{
			continue;
		}

		//directional light
		//TODO sample this!
		float3 normal = float3(0, 0, 1); //up
		float3 lightDir = camViewDir;
		float nDotl = max(dot(normal, -lightDir), 0.0);

		InterlockedMax(voxelLightTex[insertionPos], encodeColor(float4(albedo.xyz * nDotl, 1.0)*255.0));
		//InterlockedMax(voxelLightTex[insertionPos], encodeColor(float4(wsPos * 0.05, 1.0)*255.0));
	}
}
