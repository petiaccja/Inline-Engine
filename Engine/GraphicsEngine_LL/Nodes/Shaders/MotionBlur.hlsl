/*
* Motion Blur Gather shader
* Input: HDR color texture, velocity texture, neighborhood max texture, depth texture
* Output: blurred HDR color texture
*/

struct Uniforms
{
	float maxMotionBlurRadius;
	float reconstructionFilterTaps;
	float halfExposure;
	float maxSampleTapDistance;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0); //HDR texture
Texture2D velocityTex : register(t1); //RG8 tex
Texture2D neighborMaxTex : register(t2); //RG8 tex
Texture2D depthTex : register(t3); //depth tex
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
};

float2 undoVelocityBiasScale(float2 v)
{
	return v * 2.0 - 1.0;
}

float2 doVelocityBiasScale(float2 v)
{
	return (v + 1.0) * 0.5;
}

///////////////////////////
//depth compare functions
float cone(float magDiff, float magVel)
{
	return 1.0 - abs(magDiff) / magVel;
}

float cylinder(float magDiff, float magVel)
{
	return 1.0 - smoothstep(0.95 * magVel, 1.05 * magVel, abs(magDiff));
}

float softDepthCompare(float za, float zb)
{
	return clamp((1.0 - (za - zb) / 0.1), 0.0, 1.0);
}
///////////////////////////

float linearize_depth(float depth)
{
	float near = 0.1;
	float far = 100.0;
	float A = -(far + near) / (far - near);
	float B = -2 * far * near / (far - near);
	float zndc = depth * 2 - 1;

	//view space linear z
	float vs_zrecon = -B / (zndc + A);

	//range: [0...1]
	return vs_zrecon * far / -1.0;//far;
};

float rand(float2 n) {
	return frac(sin(dot(n, float2(12.9898, 4.1414))) * 43758.5453);
}

float pseudoRandom(float2 p) {
	float2 ip = floor(p);
	float2 u = frac(p);
	u = u*u*(3.0 - 2.0*u);

	float res = lerp(
		lerp(rand(ip), rand(ip + float2(1.0, 0.0)), u.x),
		lerp(rand(ip + float2(0.0, 1.0)), rand(ip + float2(1.0, 1.0)), u.x), u.y);
	return res*res;
}

PS_Input VSMain(float4 position : POSITION, float4 texcoord : TEX_COORD)
{
	PS_Input result;

	result.position = position;
	result.texcoord = texcoord.xy;

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	// X (position of current fragment)
	int2 currPixelPos = int2(input.position.xy);

	// C[X] (sample from color buffer at X)
	float4 currColor = inputTex.Load(int3(currPixelPos, 0));

	//return currColor;

	// VN = NeighborMax[X/k] (sample from NeighborMax buffer; holds dominant
	// half-velocity for the current fragment's neighborhood tile)
	float2 dominantVelocity = undoVelocityBiasScale(neighborMaxTex.Load(int3(currPixelPos / (int)uniforms.maxMotionBlurRadius,0)).xy);
	float lenDomVel = length(dominantVelocity);

	// Weighting, correcting and clamping half-velocity
	float tempDomVel = lenDomVel * uniforms.halfExposure;
	bool flag = (tempDomVel >= 0.01);
	tempDomVel = clamp(tempDomVel, 0.1, uniforms.maxMotionBlurRadius);

	//If the velocities are too short, we simply show the color texel and exit
	if (tempDomVel <= (0.001 + 0.25))
	{
		return currColor;
	}

	// Weighting, correcting and clamping half-velocity
	if (flag)
	{
		dominantVelocity *= (tempDomVel / lenDomVel);
		lenDomVel = length(dominantVelocity);
	}

	//return max(float4(dominantVelocity, 0, 0), float4(0, 0, 0, 0));

	// V[X] (sample from half-velocity buffer at X)
	float2 velocity = undoVelocityBiasScale(velocityTex.Load(int3(currPixelPos, 0)).xy);
	float lenVel = length(velocity);

	// Weighting, correcting and clamping half-velocity
	float tempVel = lenVel * uniforms.halfExposure;
	bool flag2 = (tempVel >= 0.01);
	tempVel = clamp(tempVel, 0.1, uniforms.maxMotionBlurRadius);
	if (flag2)
	{
		velocity *= (tempVel / lenVel);
		lenVel = length(velocity);
	}

	//return max(float4(velocity, 0, 0), float4(0, 0, 0, 0));

	// Random value in [-0.5, 0.5]
	float random = pseudoRandom(currPixelPos) - 0.5;

	//return float4(random, 0, 0, 1);

	// Z[X] (depth value at X)
	float currDepth = -linearize_depth(depthTex.Load(int3(currPixelPos, 0)));

	//return float4(-currDepth, 0, 0, 1);

	// If V[X] (for current fragment) is too small,
	// then we use VN (for current tile)
	float2 correctedVel = (lenVel < 1.5) ? normalize(dominantVelocity) : normalize(velocity);

	//return max(float4(correctedVel, 0, 0), float4(0, 0, 0, 0));

	// Weight value (suggested by the article authors' implementation)
	float weight = uniforms.reconstructionFilterTaps / 60.0 / tempVel;

	// Cumulative sum (initialized to the current fragment,
	// since we skip it in the loop)
	float3 sum = currColor.xyz * weight;

	//return float4(sum, 1.0);
	//return max(float4(sum, 1.0), float4(0, 0, 0, 0));

	// Index for same fragment
	int selfIndex = (int(uniforms.reconstructionFilterTaps) - 1) / 2;

	//return float4(float(selfIndex), 0, 0, 1);

	// Iterate once for reconstruction sample tap
	for (int i = 0; i < int(uniforms.reconstructionFilterTaps); ++i)
	{
		// Skip the same fragment
		if (i == selfIndex) { continue; }

		// T (distance between current fragment and sample tap)
		// NOTE: we are not sampling adjacent ones; we are extending our taps
		//       a little further
		float distCurrSample = lerp(-uniforms.maxSampleTapDistance, uniforms.maxSampleTapDistance,
			(float(i) + random + 1.0) / (uniforms.reconstructionFilterTaps + 1.0));

		// The authors' implementation suggests alternating between the
		// corrected velocity and the neighborhood's
		float2 switchVel = ((i & 1) == 1) ? correctedVel : dominantVelocity;

		// Y (position of current sample tap)
		int2 ySamplePos = int2(currPixelPos + int2(switchVel * distCurrSample + float2(0.5, 0.5)));

		// V[Y] (sample from half-velocity buffer at Y)
		float2 velSampleY = undoVelocityBiasScale(velocityTex.Load(int3(ySamplePos, 0)).xy);
		float lenVelSampleY = length(velSampleY);

		// Weighting, correcting and clamping half-velocity
		float templVelSampleY = lenVelSampleY * uniforms.halfExposure;
		bool flag3 = (templVelSampleY >= 0.01);
		templVelSampleY = clamp(templVelSampleY, 0.1, uniforms.maxMotionBlurRadius);
		if (flag3)
		{
			velSampleY *= (templVelSampleY / lenVelSampleY);
			lenVelSampleY = length(velSampleY);
		}

		// Z[Y] (depth value at Y)
		float depthSampleY = -linearize_depth(depthTex.Load(int3(ySamplePos, 0)));

		// alpha = foreground contribution + background contribution + 
		//         blur of both foreground and background
		float alphaY = softDepthCompare(currDepth, depthSampleY) * cone(distCurrSample, templVelSampleY) +
			softDepthCompare(depthSampleY, currDepth) * cone(distCurrSample, tempVel) +
			cylinder(distCurrSample, templVelSampleY) * cylinder(distCurrSample, tempVel) * 2.0;

		// Applying to weight and weighted sum
		weight += alphaY;
		sum += (alphaY * inputTex.Load(int3(ySamplePos, 0)).xyz);
	}

	return float4(sum / weight, 1.0);
	//return max(float4(sum / weight, 1.0), float4(0, 0, 0, 0));
}
