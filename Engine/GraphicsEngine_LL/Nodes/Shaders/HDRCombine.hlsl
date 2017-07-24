/*
* HDR Combine shader
* Input0: HDR color texture
* Input1: average luminance texture
* Output: LDR color texture
*/

struct Uniforms
{
	float4x4 lensStarMx;
	float exposure, bloom_weight;
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

float3 color_grading(float3 col)
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

float3 blue_shift(float3 col, float night_factor)
{
	//blue shift, experimental
	const float3 blue_shift = float3(1.05, 0.97, 1.27);
	const float3 lumCoeff = float3(0.2126, 0.7152, 0.0722);
	float3 night_color = dot(col, lumCoeff) * blue_shift;
	float blue_shift_coeff = night_factor;
	return lerp(col, night_color, blue_shift_coeff);
}

/*float3 vignette(float2 texcoord, float3 col)
{
	float vignette_size = 0.5, vignette_amount = 0.5;
	float dist = distance(texcoord, float2(0.5, 0.5));

	return col * smoothstep(0.8, vignette_size * 0.799, dist * (vignette_amount + vignette_size));
}*/

float3 vignette(float2 texcoord, float3 col)
{
	float aperture_diameter = 2.8 * 0.001;
	float lens_to_sensor_dist = 20 * 0.001;
	float aspect_ratio = 16.0 / 9;
	float sensor_width = 35 * 0.001;
	float sensor_height = sensor_width / aspect_ratio;

	float3 aperture_corner_ll = float3(sensor_width * 0.5, sensor_height * 0.5, lens_to_sensor_dist) - float3(aperture_diameter * 0.5, aperture_diameter * 0.5, 0.0);
	float3 aperture_corner_ur = float3(sensor_width * 0.5, sensor_height * 0.5, lens_to_sensor_dist) + float3(aperture_diameter * 0.5, aperture_diameter * 0.5, 0.0);
	float3 aperture_vector = aperture_corner_ur - aperture_corner_ll;

	float3 sensor_corner_ll = float3(0, 0, 0);
	float3 sensor_corner_ur = float3(sensor_width, sensor_height, 0);
	float3 sensor_vector = sensor_corner_ur - sensor_corner_ll;

	float3 point_on_aperture = aperture_corner_ll + float3(aperture_vector.xy * texcoord, 0);
	float3 point_on_sensor = sensor_corner_ll + float3(sensor_vector.xy * texcoord, 0);

	float3 angle_vector = normalize(point_on_aperture - point_on_sensor);
	float3 compare_vector = float3(0, 0, 1);
	
	float angle = dot(angle_vector, compare_vector);
	float angle_sqr = angle * angle;
	return col * angle_sqr * angle_sqr; //(cos(x))^4
	//return col;
}

float3 lens_flare(float2 texcoord)
{
	float3 lens_mod = gamma_to_linear(lensFlareDirtTex.Sample(samp0, texcoord));
	float2 lens_star_tex_coord = mul(float4(texcoord, 1, 0), uniforms.lensStarMx).xy;
	//return gamma_to_linear(lens_mod.xyz);
	//return gamma_to_linear(lensFlareStarTex.Sample(samp0, lens_star_tex_coord));
	lens_mod += gamma_to_linear(lensFlareStarTex.Sample(samp0, lens_star_tex_coord));

	float4 input_tex = lensFlareTex.Sample(samp0, texcoord);

	return max(input_tex.xyz * lens_mod.xyz, 0) * 100;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	float4 inputData = max(inputTex.Load(int3(input.position.xy, 0)), float4(0,0,0,0));
	//float4 inputData = max(lensFlareTex.Load(int3(input.position.xy, 0)), float4(0,0,0,0));
	float4 bloomData = bloomTex.Sample(samp0, input.texcoord);

	inputData.xyz = vignette(input.texcoord, inputData.xyz);

	inputData += bloomData * uniforms.bloom_weight;
	inputData.xyz += lens_flare(input.texcoord);
	
	float3 hdrColor = max(inputData, float4(0, 0, 0, 0)).xyz;
	float avg_lum = luminanceTex.Sample(samp0, input.texcoord);

	//hdrColor = blue_shift(hdrColor, 1.0);

	float3 ldr = tonemap(hdrColor * avg_lum * (uniforms.exposure + 1.0));

	ldr = linear_to_gamma(ldr);

	//ldr = color_grading(ldr);

	return float4(ldr, 1.0);
}
