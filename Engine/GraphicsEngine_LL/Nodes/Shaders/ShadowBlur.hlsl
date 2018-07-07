/*
* Shadow blur shader
* Input: layered penumbra texture, layered shadow texture
* Output: blurred shadows
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

Texture2D inputTex0 : register(t0); //depth texture
//TODO normal tex
Texture2DArray<float> inputTex1 : register(t1); //csm minfilter map
TextureCube<float> inputTex2 : register(t2); //cube minfilter map
Texture2D inputTex3 : register(t3); //layered penumbra texture
Texture2D inputTex4 : register(t4); //layered shadow texture
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

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

float2 get_step_size( float2 direction, float3 normal, float depth, float threshold )
{
  return direction 
         //* light_size * light_size //included in the penumbra
         * sqrt( max( dot( float3( 0.0, 0.0, 1.0 ), normal ), threshold ) )
         * (1 / (depth/* * depth * 100*/));
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

float4 PSMain(PS_Input input) : SV_TARGET
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);
	
	float depth = inputTex0.Sample(samp0, input.texCoord).x;
	
	if(depth > 0.999)
	{
		discard;
	}
	
	float linearDepth = linearize_depth(depth, uniforms.nearPlane, uniforms.farPlane);
	
	float3 farPlaneLL = uniforms.farPlaneData0.xyz;
	float3 farPlaneUR = float3(uniforms.farPlaneData0.w, uniforms.farPlaneData1.xy);

	float2 uv = float2(input.texCoord.x, 1 - input.texCoord.y);
	float3 vsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, uv) / uniforms.farPlane, 1.0) * linearDepth;
	
	//TODO replace with proper normals
	float3 vsDepthNormal = -normalize(cross(ddy(vsPos.xyz), ddx(vsPos.xyz)));
	
	float4 hardShadow = inputTex4.Sample(samp0, input.texCoord);
	
	const float aniso_threshold = 0.25; //TODO make it uniform
	const float2 direction = float2(0.89442719082100156952748325334158, 0.44721359585778655717526397765935) * 1.11803398875;
	float2 step_size = get_step_size( direction, vsDepthNormal, linearDepth, aniso_threshold );
	
	
	
	
	
	
	
	

	float4 blurredResultLayers = float4(step_size, 0.0, 0.0);
	
	return blurredResultLayers;
}
