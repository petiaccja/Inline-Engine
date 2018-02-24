/*
* Voxel Final Gather shader
* Input0: voxel light tex
* Input1: depth tex
* Output: scene with added GI
*/

struct Uniforms
{
	float4x4 model, viewProj;
	float3 voxelCenter; float voxelSize;
	float4 farPlaneData0, farPlaneData1;
	float4 vsCamPos;
	int voxelDimension; int inputMipLevel; int outputMipLevel; int dummy;
	float nearPlane, farPlane;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture3D inputTex0 : register(t0); //voxel light tex
Texture3D inputTex1 : register(t1); //voxel tex
Texture2D inputTex2 : register(t2); //depth tex

SamplerState samp0 : register(s0); //point
SamplerState samp1 : register(s1); //bilinear
SamplerState samp2 : register(s2); //trilinear

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

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
};


PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float depth = inputTex2.Sample(samp0, input.texcoord);

	if (depth > 0.9999)
	{
		return 0.0;
	}

	float linearDepth = linearize_depth(depth, uniforms.nearPlane, uniforms.farPlane);

	float3 farPlaneLL = uniforms.farPlaneData0.xyz;
	float3 farPlaneUR = float3(uniforms.farPlaneData0.w, uniforms.farPlaneData1.xy);

	float2 uv = float2(input.texcoord.x, 1 - input.texcoord.y);
	float3 vsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, uv) / uniforms.farPlane, 1.0) * linearDepth;

	float3 vsViewDir = normalize(vsPos - uniforms.vsCamPos.xyz);

	//TODO replace with proper normals
	float3 vsDepthNormal = -normalize(cross(ddy(vsPos.xyz), ddx(vsPos.xyz)));

	float3 perfectReflectionDir = reflect(vsViewDir, vsDepthNormal);

	//return float4(linearDepth, linearDepth, linearDepth, linearDepth);
	//return float4(vsPos, 1.0);
	//return float4(vsViewDir, 1.0);
	//return float4(vsDepthNormal, 1.0);
	return float4(perfectReflectionDir, 1.0);
}
