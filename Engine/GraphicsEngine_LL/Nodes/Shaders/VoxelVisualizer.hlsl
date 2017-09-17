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
	float4 voxel	: TEXCOORD;
};

struct PS_Input
{
	float4 position		: SV_POSITION;
	float4 voxelColor	: TEXCOORD;
};


GS_Input VSMain(uint id : SV_VertexID)
{
	GS_Input result;

	uint dim = uniforms.voxelDimension;
	float size = uniforms.voxelSize;
	float3 center = uniforms.voxelCenter;

	//[0...255]
	uint3 pos;
	pos.x = id % dim;
	pos.y = id / (dim * dim);
	pos.z = (id / dim) % dim;

	float4 voxel = voxelTex.Load(pos);

	float3 realPos = ((float3(pos)/float(uniforms.voxelDimension)) * 2.0 - 1.0)*float(uniforms.voxelDimension)*size*0.5;
	result.position = float4(realPos, 1);

	result.voxel = voxel;

	return result;
}

[maxvertexcount(36)]
void GSMain(point GS_Input input[1], inout TriangleStream<PS_Input> OutputStream)
{
	PS_Input output = (PS_Input)0;

	//if empty voxel
	if (input[0].voxel.w < 0.1)
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
