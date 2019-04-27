/*
* HDR Combine shader
* Input0: HDR color texture
* Input1: average luminance texture
* Output: LDR color texture
*/

struct Uniforms
{
	float4x4 lensStarMx;
	float exposure, bloomWeight;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
Texture2D luminanceTex : register(t1);
Texture2D bloomTex : register(t2);
TextureCube colorGradingTex : register(t3);
Texture2D lensFlareTex : register(t4);
Texture2D lensFlareDirtTex : register(t5);
Texture2D lensFlareStarTex : register(t6);
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
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


float3 TonemapFunc(float3 x, float a, float b, float c, float d, float e, float f)
{
	return ((x * (a * x + c * b) + d * e) / (x * (a * x + b) + d * f)) - e / f;
}

float3 Tonemap(float3 col)
{
	//vec3 x = max( vec3(0), col - vec3(0.004));
	//return ( x * (6.2 * x + 0.5) ) / ( x * ( 6.2 * x + 1.7 ) + 0.06 );

	float a = 0.22; //Shoulder Strength
	float b = 0.30; //Linear Strength
	float c = 0.10; //Linear Angle
	float d = 0.20; //Toe Strength
	float e = 0.01; //Toe Numerator
	float f = 0.30; //Toe Denominator
	float linearWhite = 11.2; //Linear White Point Value (11.2)
							   //Note: E/F = Toe Angle

	return TonemapFunc(col, a, b, c, d, e, f) / TonemapFunc(float3(linearWhite, linearWhite, linearWhite), a, b, c, d, e, f);
}

//NOTE: actually, just use SRGB, it's got better quality!
float3 LinearToGamma(float3 col)
{
	return pow(col, float3(1 / 2.2, 1 / 2.2, 1 / 2.2));
}

float3 GammaToLinear(float3 col)
{
	return pow(col, float3(2.2, 2.2, 2.2));
}

float3 ColorGrading(float3 col)
{
	uint2 colorGradingTexSize;
	colorGradingTex.GetDimensions(colorGradingTexSize.x, colorGradingTexSize.y);

	float sizeMinusOne = colorGradingTexSize.x - 1.0;
	float sizeTwo = 2.0 * colorGradingTexSize.x;

	float3 scale = float3(sizeMinusOne, sizeMinusOne, sizeMinusOne) / colorGradingTexSize.x;
	float3 offset = 1.0 / float3(sizeTwo, sizeTwo, sizeTwo);

	float3 coord = scale * col + offset;

	col = colorGradingTex.Sample(samp0, coord).xyz;

	return col;
}

float3 BlueShift(float3 col, float nightFactor)
{
	//blue shift, experimental
	const float3 blueShift = float3(1.05, 0.97, 1.27);
	const float3 lumCoeff = float3(0.2126, 0.7152, 0.0722);
	float3 nightColor = dot(col, lumCoeff) * blueShift;
	float blueShiftCoeff = nightFactor;
	return lerp(col, nightColor, blueShiftCoeff);
}

/*float3 vignette(float2 texCoord, float3 col)
{
	float vignette_size = 0.5, vignette_amount = 0.5;
	float dist = distance(texCoord, float2(0.5, 0.5));

	return col * smoothstep(0.8, vignette_size * 0.799, dist * (vignette_amount + vignette_size));
}*/

float3 Vignette(float2 texCoord, float3 col)
{
	float apertureDiameter = 2.8 * 0.001;
	float lensToSensorDist = 20 * 0.001;
	float aspectRatio = 16.0 / 9;
	float sensorWidth = 35 * 0.001;
	float sensorHeight = sensorWidth / aspectRatio;

	float3 apertureCornerLl = float3(sensorWidth * 0.5, sensorHeight * 0.5, lensToSensorDist) - float3(apertureDiameter * 0.5, apertureDiameter * 0.5, 0.0);
	float3 apertureCornerUr = float3(sensorWidth * 0.5, sensorHeight * 0.5, lensToSensorDist) + float3(apertureDiameter * 0.5, apertureDiameter * 0.5, 0.0);
	float3 apertureVector = apertureCornerUr - apertureCornerLl;

	float3 sensorCornerLl = float3(0, 0, 0);
	float3 sensorCornerUr = float3(sensorWidth, sensorHeight, 0);
	float3 sensorVector = sensorCornerUr - sensorCornerLl;

	float3 pointOnAperture = apertureCornerLl + float3(apertureVector.xy * texCoord, 0);
	float3 pointOnSensor = sensorCornerLl + float3(sensorVector.xy * texCoord, 0);

	float3 angleVector = normalize(pointOnAperture - pointOnSensor);
	float3 compareVector = float3(0, 0, 1);
	
	float angle = dot(angleVector, compareVector);
	float angleSqr = angle * angle;
	return col * angleSqr * angleSqr; //(cos(x))^4
	//return col;
}

float3 LensFlare(float2 texCoord)
{
	float3 lensMod = GammaToLinear(lensFlareDirtTex.Sample(samp0, texCoord));
	float2 lensStarTexCoord = mul(float4(texCoord, 1, 0), uniforms.lensStarMx).xy;
	//return gamma_to_linear(lens_mod.xyz);
	//return gamma_to_linear(lensFlareStarTex.Sample(samp0, lens_star_tex_coord));
	lensMod += GammaToLinear(lensFlareStarTex.Sample(samp0, lensStarTexCoord));

	float4 inputTex = lensFlareTex.Sample(samp0, texCoord);

	return max(inputTex.xyz * lensMod.xyz, 0) * 100;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float4 inputData = max(inputTex.Load(int3(input.position.xy, 0)), float4(0,0,0,0));
	//float4 inputData = max(lensFlareTex.Load(int3(input.position.xy, 0)), float4(0,0,0,0));
	float4 bloomData = bloomTex.Sample(samp0, input.texCoord);

	inputData.xyz = Vignette(input.texCoord, inputData.xyz);

	inputData += bloomData * uniforms.bloomWeight;
	inputData.xyz += LensFlare(input.texCoord);
	
	float3 hdrColor = max(inputData, float4(0, 0, 0, 0)).xyz;
	float avgLum = luminanceTex.Sample(samp0, input.texCoord);

	//hdrColor = blue_shift(hdrColor, 1.0);

	float3 ldr = Tonemap(hdrColor * avgLum * uniforms.exposure);

	ldr = LinearToGamma(ldr);

	//ldr = color_grading(ldr);

	return float4(ldr, 1.0);
}
