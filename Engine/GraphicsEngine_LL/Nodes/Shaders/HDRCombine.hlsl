/*
* HDR Combine shader
* Input0: HDR color texture
* Input1: average luminance texture
* Output: LDR color texture
*/

struct Uniforms
{
	float exposure, bloom_weight;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
Texture2D luminanceTex : register(t1);
Texture2D bloomTex : register(t2);
SamplerState samp0 : register(s0);

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

float3 tonemap_func(float3 x, float a, float b, float c, float d, float e, float f)
{
	return ((x * (a * x + c * b) + d * e) / (x * (a * x + b) + d * f)) - e / f;
}

float3 tonemap(float3 col)
{
	//vec3 x = max( vec3(0), col - vec3(0.004));
	//return ( x * (6.2 * x + 0.5) ) / ( x * ( 6.2 * x + 1.7 ) + 0.06 );

	float a = 0.22; //Shoulder Strength
	float b = 0.30; //Linear Strength
	float c = 0.10; //Linear Angle
	float d = 0.20; //Toe Strength
	float e = 0.01; //Toe Numerator
	float f = 0.30; //Toe Denominator
	float linear_white = 11.2; //Linear White Point Value (11.2)
							   //Note: E/F = Toe Angle

	return tonemap_func(col, a, b, c, d, e, f) / tonemap_func(float3(linear_white, linear_white, linear_white), a, b, c, d, e, f);
}

//NOTE: actually, just use SRGB, it's got better quality!
float3 linear_to_gamma(float3 col)
{
	return pow(col, float3(1 / 2.2, 1 / 2.2, 1 / 2.2));
}

float3 gamma_to_linear(float3 col)
{
	return pow(col, float3(2.2, 2.2, 2.2));
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float4 inputData = inputTex.Sample(samp0, input.texcoord);
	float4 bloomData = bloomTex.Sample(samp0, input.texcoord);

	inputData += bloomData * uniforms.bloom_weight;
	
	float3 hdrColor = max(inputData, float4(0, 0, 0, 0)).xyz;
	float avg_lum = luminanceTex.Sample(samp0, input.texcoord);

	float3 ldr = tonemap(hdrColor * avg_lum * (uniforms.exposure + 1.0));

	ldr = linear_to_gamma(ldr);

	return float4(ldr, 1.0);
}
