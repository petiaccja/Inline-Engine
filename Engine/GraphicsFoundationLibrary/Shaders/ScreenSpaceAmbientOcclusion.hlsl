/*
* Screen space ambient occlusion shader
* Input: depth texture
* Output: ambient occlusion
*/

struct Uniforms
{
	float4x4 invVP, oldVP;
	float4 farPlaneData0, farPlaneData1;
	float nearPlane, farPlane, wsRadius, scaleFactor;
	float temporalIndex;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D depthTex : register(t0);
SamplerState samp0 : register(s0);
SamplerState samp1 : register(s1);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD0;
};

static const float pi = 3.14159265;

//white noise
//[0...1]
float Rand(float seed)
{
	return frac(sin(seed) * 43758.5453123);
}

//TODO: supply this from uniform/texture
float GetHalton(uint i, uint b)
{
	float f = 1.0;
	float r = 0.0;

	while (i > 0)
	{
		f = f / float(b);
		r = r + f * float(i % b);
		i = i / b;
	}

	return r;
}

//transforms a direction into the coordinate system
//defined by the normal vector
float3 TransNormal(float3 n, float3 d)
{
	float3 a, b;

	if (abs(n.z) < 1.0)
	{
		a = normalize(cross(float3(0, 0, 1), n));
	}
	else
	{
		a = normalize(cross(float3(1, 0, 0), n));
	}

	b = normalize(cross(n, a));

	return a * d.x + b * d.y + n * d.z;
}

float FalloffFunc(float distSqr)
{
	float falloffStart = 0.4;
	float falloffEnd = 2.0;
	float falloffStartSqr = falloffStart*falloffStart;
	float falloffEndSqr = falloffEnd*falloffEnd;
	return 2.0 * clamp((distSqr - falloffStartSqr) / (falloffEndSqr - falloffStartSqr), 0.0, 1.0);
}

float3 MultiBounce(float ao, float3 albedo)
{
	float3 a = 2.0404 * albedo - 0.3324;
	float3 b = -4.7951 * albedo + 0.6417;
	float3 c = 2.7552 * albedo + 0.6903;

	return max(ao, ((ao * a + b) * ao + c) * ao);
}

float LinearizeDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vsZrecon = B / (zndc - A);

	//range: [0...far]
	return vsZrecon;// / far;
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
	depthTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	float seed = input.position.x * inputTexSize.y + input.position.y;

	//[0...1]
	float ndcDepth = depthTex.Sample(samp0, input.texCoord).x;

	if (ndcDepth > 0.9999)
	{
		return 0.0;
	}

	//[0...far]
	float linearDepth = LinearizeDepth(ndcDepth, uniforms.nearPlane, uniforms.farPlane);

	float3 farPlaneLL = uniforms.farPlaneData0.xyz;
	float3 farPlaneUR = float3(uniforms.farPlaneData0.w, uniforms.farPlaneData1.xy);

	float2 uv = float2(input.texCoord.x, 1 - input.texCoord.y);
	float3 vsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, uv) / uniforms.farPlane, 1.0) * linearDepth;

	//return float4(vsPos, 1.0);

	float3 vsViewDir = normalize(-vsPos);

	//TODO replace with proper normals
	float3 vsDepthNormal = -normalize(cross(ddy(vsPos.xyz), ddx(vsPos.xyz)));

	float ssRadius = 50.0;// min(uniforms.wsRadius * uniforms.scaleFactor / vsPos.z, 100.0);
	//float ssRadius = min(uniforms.wsRadius * uniforms.scaleFactor / vsPos.z, 100.0);

	//return float4(vsPos, 1.0);

	float ao = 0.0;

	float2 spatialSamples = fmod(input.position.xy, 4);
	float2 spatialSamples2 = fmod(input.position.xy, 2);
	float spatialIndex = spatialSamples.x * 4 + spatialSamples.y;
	float spatialIndex2 = spatialSamples2.x * 2 + spatialSamples2.y;
	float numTemporalSamples = 6;
	float numSpatialSamples = 16;
	float numSpatialSamples2 = 4;
	float numEffectiveSamples = numTemporalSamples * numSpatialSamples;

	//const float numDirs = 70;
	//for (float d = 0; d < numDirs; ++d)
	{
		float2 randomFactor = float2(GetHalton(spatialIndex + uniforms.temporalIndex * numSpatialSamples, 2), GetHalton(spatialIndex + uniforms.temporalIndex * numSpatialSamples, 3));
		//float2 randomFactor = float2(getHalton(100.0+spatialIndex, 2), getHalton(100.0 + spatialIndex, 3));
		//float2 randomFactor = float2(getHalton(seed + d, 2), getHalton(seed + d, 3));
		//float2 randomFactor = float2(rand(seed*numDirs + d), rand(seed*numDirs + d));

		//return float4(randomFactor, 0, 1);

		float2 ssDirN = float2(cos(randomFactor.x*pi), sin(randomFactor.x*pi));
		//float2 ssDirN = float2(cos(d / numDirs*pi), sin(d / numDirs*pi));
		float2 ssDir = ssDirN / inputTexSize.xy;

		//return float4(ssDir, 0, 1);

		float2 ssPos = uv - ssRadius * ssDir;

		//return float4(ssDir, 0, 1);
		//return float4(ssPos, 0, 1);

		//theta1 and theta2
		float2 horizons = float2(-1.0, -1.0);

		const float numSteps = 4.0;
		for (float c = 0; c < numSteps; ++c)
		{
			float2 currSSPos = ssPos + (c / numSteps) * ssDir * ssRadius;

			float currDepth = depthTex.Sample(samp0, float2(currSSPos.x, 1.0 - currSSPos.y)).x;
			float currLinearDepth = LinearizeDepth(currDepth, uniforms.nearPlane, uniforms.farPlane);
			float3 currVsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, currSSPos) / uniforms.farPlane, 1.0) * currLinearDepth;

			float3 diff = currVsPos - vsPos;
			float diffLengthSqr = dot(diff, diff);
			float3 vsCurrDir = normalize(diff);

			float cosAngle = dot(vsCurrDir, vsViewDir);

			float falloff = FalloffFunc(diffLengthSqr);
			horizons.y = max(horizons.y, cosAngle-falloff);
		}

		ssPos = uv;

		for (float e = 1; e <= numSteps; ++e)
		{
			float2 currSSPos = ssPos + (e / numSteps) * ssDir * ssRadius;

			float currDepth = depthTex.Sample(samp0, float2(currSSPos.x, 1.0 - currSSPos.y)).x;
			float currLinearDepth = LinearizeDepth(currDepth, uniforms.nearPlane, uniforms.farPlane);
			float3 currVsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, currSSPos) / uniforms.farPlane, 1.0) * currLinearDepth;
			
			float3 diff = currVsPos - vsPos;
			float diffLengthSqr = dot(diff, diff);
			float3 vsCurrDir = normalize(diff);

			float cosAngle = dot(vsCurrDir, vsViewDir);

			float falloff = FalloffFunc(diffLengthSqr);
			horizons.x = max(horizons.x, cosAngle- falloff);
		}

		//return float4(horizons, 0, 1);

		horizons = acos(horizons);

		//return float4(horizons, 0, 1);

		//TODO revise this...
		float3 bitangent = normalize(cross(float3(ssDirN, 0.0), vsViewDir));
		float3 tangent = cross(vsViewDir, bitangent);
		float3 nx = vsDepthNormal - bitangent * dot(vsDepthNormal, bitangent);
		float nxLength = length(nx);
		float cosXi = dot(nx, tangent) / (nxLength + 1e-5);
		float gamma = acos(cosXi) - pi*0.5; //-pi*0.5 because cosXi is sinGamma
		float cosGamma = dot(nx, vsViewDir) / (nxLength + 1e-5);
		float sinGamma2 = -2.0 * cosXi;

		horizons.x = gamma + max(-horizons.x - gamma, -pi*0.5);
		horizons.y = gamma + min(horizons.y - gamma, pi*0.5);

		ao += nxLength * 0.25 * (
			(-cos(2.0*horizons.x - gamma) + cosGamma + horizons.x * sinGamma2) +
			(-cos(2.0*horizons.y - gamma) + cosGamma + horizons.y * sinGamma2)
			);
	}

	//ao = ao / numDirs;

	//TODO sample real albedo
	//return float4(multiBounce(ao, float3(1, 1, 1)), 1.0);
	return ao;

	//return float4(haltonFactor, 0, 1);
	//return float4(float2(rand(seed), rand(seed)), 0, 1);
	//return float4(vsDepthNormal, 1.0);
	//return ssRadius*0.1;
	//return float4(vsViewDir, 1.0);
}
