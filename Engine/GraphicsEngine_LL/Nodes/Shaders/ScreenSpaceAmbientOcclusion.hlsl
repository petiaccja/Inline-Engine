/*
* Screen space ambient occlusion shader
* Input: depth texture
* Output: ambient occlusion
*/

struct Uniforms
{
	float4 farPlaneData0, farPlaneData1;
	float nearPlane, farPlane, wsRadius, scaleFactor;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D depthTex : register(t0);
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX_COORD0;
};

static const float pi = 3.14159265;

//white noise
//[0...1]
float rand(float seed)
{
	return frac(sin(seed) * 43758.5453123);
}

//TODO: supply this from uniform/texture
float getHalton(uint i, uint b)
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
float3 trans_normal(float3 n, float3 d)
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

float linearize_depth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vs_zrecon = B / (zndc - A);

	//range: [0...far]
	return vs_zrecon;// / far;
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
	uint3 inputTexSize;
	depthTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	float seed = input.position.x * inputTexSize.y + input.position.y;

	//[0...1]
	float ndcDepth = depthTex.Sample(samp0, input.texcoord).x;

	if (ndcDepth > 0.9999)
	{
		return 0.0;
	}

	//[0...far]
	float linearDepth = linearize_depth(ndcDepth, uniforms.nearPlane, uniforms.farPlane);

	float3 farPlaneLL = uniforms.farPlaneData0.xyz;
	float3 farPlaneUR = float3(uniforms.farPlaneData0.w, uniforms.farPlaneData1.xy);

	float2 uv = float2(input.texcoord.x, 1 - input.texcoord.y);
	float3 vsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, uv) / uniforms.farPlane, 1.0) * linearDepth;

	//return float4(vsPos, 1.0);

	float3 vsViewDir = normalize(-vsPos);

	//TODO replace with proper normals
	float3 vsDepthNormal = -normalize(cross(ddy(vsPos.xyz), ddx(vsPos.xyz)));

	float ssRadius = 50.0;// min(uniforms.wsRadius * uniforms.scaleFactor / vsPos.z, 100.0);
	//float ssRadius = min(uniforms.wsRadius * uniforms.scaleFactor / vsPos.z, 100.0);

	//return float4(vsPos, 1.0);

	float ao = 0.0;

	const float numDirs = 40;
	for (float d = 0; d < numDirs; ++d)
	{
		float2 randomFactor = float2(getHalton(seed*numDirs + d, 2), getHalton(seed*numDirs + d, 3));
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

		const float numSteps = 8.0;
		for (float c = 0; c < numSteps*0.5; ++c)
		{
			float2 currSSPos = ssPos + (c / numSteps) * ssDir * ssRadius * 2.0;

			float currDepth = depthTex.Sample(samp0, float2(currSSPos.x, 1.0 - currSSPos.y)).x;
			float currLinearDepth = linearize_depth(currDepth, uniforms.nearPlane, uniforms.farPlane);
			float3 currVsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, currSSPos) / uniforms.farPlane, 1.0) * currLinearDepth;

			float3 vsCurrDir = normalize(currVsPos - vsPos);

			float cosAngle = dot(vsCurrDir, vsViewDir);

			horizons.y = max(horizons.y, cosAngle);
		}

		for (float c = numSteps*0.5 + 1.0; c <= numSteps; ++c)
		{
			float2 currSSPos = ssPos + (c / numSteps) * ssDir * ssRadius * 2.0;

			float currDepth = depthTex.Sample(samp0, float2(currSSPos.x, 1.0 - currSSPos.y)).x;
			float currLinearDepth = linearize_depth(currDepth, uniforms.nearPlane, uniforms.farPlane);
			float3 currVsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, currSSPos) / uniforms.farPlane, 1.0) * currLinearDepth;
			
			float3 vsCurrDir = normalize(currVsPos - vsPos);

			float cosAngle = dot(vsCurrDir, vsViewDir);

			horizons.x = max(horizons.x, cosAngle);
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

	ao = ao / numDirs;

	//return float4(haltonFactor, 0, 1);
	//return float4(float2(rand(seed), rand(seed)), 0, 1);
	//return float4(vsDepthNormal, 1.0);
	//return ssRadius*0.1;
	//return float4(vsViewDir, 1.0);
	return ao;
}
