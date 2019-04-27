/*
* Voxel Visualizer shader
* Input: R32U 3D voxel texture
* Output: voxels rendered into render target
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
Texture3D<float4> voxelTex : register(t0);

struct GS_Input
{
	float4 position	: SV_POSITION;
	float4 voxel	: TEXCOORD;
};

struct PS_Input
{
	float4 position		: SV_POSITION;
	float4 voxelColor	: TEXCOORD;
};

//encodes rgba8 color [0...255] into uint
uint EncodeColor(float4 color)
{
	return uint(color.x) | uint(color.y) << 8 | uint(color.z) << 16 | uint(color.w) << 24;
}

//decode rgba8 color [0...1] from uint
float4 DecodeColor(uint color)
{
	return float4(
		float(color & 0x000000ff) / 255.0,
		float((color >> 8) & 0x000000ff) / 255.0,
		float((color >> 16) & 0x000000ff) / 255.0,
		float((color >> 24) & 0x000000ff) / 255.0
		);
}

GS_Input VSMain(uint id : SV_VertexID)
{
	GS_Input result;

	uint dim = uniforms.voxelDimension;
	float size = uniforms.voxelSize;
	float3 center = uniforms.voxelCenter;

	//[0...255]
	uint4 pos;
	pos.x = id % dim;
	pos.y = id / (dim * dim);
	pos.z = (id / dim) % dim;
	pos.w = 0;

	//float4 voxel = decodeColor(voxelTex.Load(pos));
	float4 voxel = voxelTex.Load(pos);

	float3 realPos = ((float3(pos.xyz)/float(uniforms.voxelDimension)) * 2.0 - 1.0)*float(uniforms.voxelDimension)*size*0.5;
	result.position = float4(realPos, 1);

	result.voxel = voxel;

	return result;
}

[maxvertexcount(36)]
void GSMain(point GS_Input input[1], inout TriangleStream<PS_Input> OutputStream)
{
	PS_Input output = (PS_Input)0;

	//if empty voxel
	if (input[0].voxel.w < 0.0001)
	{
		return; // skip
	}

	float4 normalX = float4(uniforms.voxelSize, 0, 0, 0);
	float4 normalY = float4(0, uniforms.voxelSize, 0, 0);
	float4 normalZ = float4(0, 0, uniforms.voxelSize, 0);

	float4 pos = input[0].position;

	output.voxelColor = input[0].voxel;

	//front quad
	output.position = mul(pos, uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalX), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalZ), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalX + normalZ), uniforms.viewProj);
	OutputStream.Append(output);
	OutputStream.RestartStrip();

	//left quad
	output.position = mul(pos, uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalZ), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalY), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalY + normalZ), uniforms.viewProj);
	OutputStream.Append(output);
	OutputStream.RestartStrip();

	//right quad
	output.position = mul(pos + normalX, uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalX + normalY), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalX + normalZ), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalX + normalY + normalZ), uniforms.viewProj);
	OutputStream.Append(output);
	OutputStream.RestartStrip();

	//back quad
	output.position = mul(pos + normalY, uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalY + normalZ), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalY + normalX), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalY + normalX + normalZ), uniforms.viewProj);
	OutputStream.Append(output);
	OutputStream.RestartStrip();

	//top quad
	output.position = mul(pos + normalZ, uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalZ + normalX), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalZ + normalY), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalZ + normalY + normalX), uniforms.viewProj);
	OutputStream.Append(output);
	OutputStream.RestartStrip();

	//bottom quad
	output.position = mul(pos, uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalY), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalX), uniforms.viewProj);
	OutputStream.Append(output);
	output.position = mul((pos + normalY + normalX), uniforms.viewProj);
	OutputStream.Append(output);
	OutputStream.RestartStrip();
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	//return float4(1,1,1,1);
	return input.voxelColor;
}
