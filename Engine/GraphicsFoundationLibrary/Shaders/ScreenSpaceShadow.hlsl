/*
* Screen space shadow shader
* Input: depth texture
* Output: shadow texture
*/

struct Uniforms
{
	float4x4 projSS;
	float4 vsSunDirection;
	float nearPlane, farPlane, stride, jitter;
	float4 farPlaneData0, farPlaneData1;
	float maxDistance;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

Texture2D inputTex : register(t0);
SamplerState samp0 : register(s0);

struct PS_Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEX_COORD0;
};

float LinearizeDepth(float depth, float near, float far)
{
	float A = far / (far - near);
	float B = -far * near / (far - near);
	float zndc = depth;

	//view space linear z
	float vsZrecon = B / (zndc - A);

	//range: [0...far]
	return vsZrecon;// / far;
};

float DistanceSquared(float2 a, float2 b) 
{ 
	a -= b; return dot(a, a); 
}

// Returns true if the ray hit something
float TraceScreenSpaceRay1(
	// Camera-space ray origin, which must be within the view volume
	float3 csOrig,
	// Unit length camera-space ray direction
	float3 csDir,
	// A projection matrix that maps to pixel coordinates (not [-1, +1]
	// normalized device coordinates)
	float4x4 proj,
	// Dimensions of csZBuffer
	float2 csZBufferSize,
	// Camera space thickness to ascribe to each pixel in the depth buffer
	float zThickness,
	// (Negative number)
	float nearPlaneZ,
	// Step in horizontal or vertical pixels between samples. This is a float
	// because integer math is slow on GPUs, but should be set to an integer >= 1
	float stride,
	// Number between 0 and 1 for how far to bump the ray in stride units
	// to conceal banding artifacts
	float jitter,
	// Maximum number of iterations. Higher gives better images but may be slow
	const float maxSteps,
	// Maximum camera-space distance to trace before returning a miss
	float maxDistance,
	// Pixel coordinates of the first intersection with the scene
	out float2 hitPixel,
	// Camera space location of the ray hit
	out float3 hitPoint) 
{
	// Clip to the near plane    
	float rayLength = maxDistance;//((csOrig.z + csDir.z * maxDistance) < nearPlaneZ) ?
		//div by csDir.z b/c near dist to near plane is constant, we need the dist to the point on the near plane from a point in the scene
		//(nearPlaneZ - csOrig.z) / csDir.z : maxDistance;
	float3 csEndPoint = csOrig + csDir * rayLength;

	// Project into homogeneous clip space
	float4 H0 = mul(float4(csOrig, 1.0), proj);
	float4 H1 = mul(float4(csEndPoint, 1.0), proj);
	float k0 = 1.0 / H0.w, k1 = 1.0 / H1.w;

	// The interpolated homogeneous version of the camera-space points  
	float3 Q0 = csOrig * k0;
	float3 Q1 = csEndPoint * k1;

	// Screen-space endpoints
	float2 P0 = H0.xy * k0;
	float2 P1 = H1.xy * k1;

	// If the line is degenerate, make it cover at least one pixel
	// to avoid handling zero-pixel extent as a special case later
	P1 += (DistanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0;
	float2 delta = P1 - P0;

	// Permute so that the primary iteration is in x to collapse
	// all quadrant-specific DDA cases later
	bool permute = false;
	if (abs(delta.x) < abs(delta.y)) 
	{
		// This is a more-vertical line
		permute = true; 
		delta = delta.yx; 
		P0 = P0.yx; 
		P1 = P1.yx;
	}

	float stepDir = delta.x < 0.0 ? -1.0 : 1.0;
	float invdx = stepDir / delta.x;

	// Track the derivatives of Q and k
	float3  dQ = (Q1 - Q0) * invdx;
	float dk = (k1 - k0) * invdx;
	float2  dP = float2(stepDir, delta.y * invdx);

	// Scale derivatives by the desired pixel stride and then
	// offset the starting values by the jitter fraction
	dP *= stride; 
	dQ *= stride; 
	dk *= stride;
	P0 += dP * jitter; 
	Q0 += dQ * jitter; 
	k0 += dk * jitter;

	// Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
	float3 Q = Q0;

	// Adjust end condition for iteration direction
	float  end = P1.x * stepDir;

	float k = k0, stepCount = 0.0, rayZ = csOrig.z;
	float sceneZMax = rayZ - 100; //-100 so it's always true for first check
	for (float2 P = P0;
		true
		//is it over in: screen space xy, # of steps
		&& ((P.x * stepDir) <= end) 
		&& (stepCount < maxSteps)
		&& (sceneZMax != 0)
		//step through: screen space xy coords, homogeneous z coords, homogeneous divisor
		;P += dP, Q.z += dQ.z, k += dk, ++stepCount) 
	{
		hitPixel = permute ? P.yx : P;
		// You may need hitPixel.y = csZBufferSize.y - hitPixel.y; here if your vertical axis
		// is different than ours in screen space
		float depth = inputTex.Load(int3(hitPixel, 0)).x;
		sceneZMax = LinearizeDepth(depth, uniforms.nearPlane, uniforms.farPlane);
		rayZ = Q.z / k;

		if (sceneZMax < rayZ - zThickness)
		{
			break;
		}
	}

	// Advance Q based on the number of steps
	Q.xy += dQ.xy * stepCount;
	hitPoint = Q * (1.0 / k);
	bool res = (sceneZMax < rayZ - zThickness) && (hitPixel.x > 0.0 && hitPixel.y > 0.0 && hitPixel.x < csZBufferSize.x && hitPixel.y < csZBufferSize.y) && (abs(sceneZMax - rayZ) < 3.0);
	return !res;
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

float PSMain(PS_Input input) : SV_TARGET
{
	uint3 inputTexSize;
	inputTex.GetDimensions(0, inputTexSize.x, inputTexSize.y, inputTexSize.z);

	//[0...1]
	float ndcDepth = inputTex.Sample(samp0, input.texCoord).x;

	if (ndcDepth > 0.9999)
	{
		return 1.0;
	}

	//[0...far]
	float linearDepth = LinearizeDepth(ndcDepth, uniforms.nearPlane, uniforms.farPlane);

	float3 farPlaneLL = uniforms.farPlaneData0.xyz;
	float3 farPlaneUR = float3(uniforms.farPlaneData0.w, uniforms.farPlaneData1.xy);

	float2 uv = float2(input.texCoord.x, 1 - input.texCoord.y);
	float3 vsPos = float3(lerp(farPlaneLL.xy, farPlaneUR.xy, uv) / uniforms.farPlane, 1.0) * linearDepth;
	
	float3 hitPoint;
	float2 hitPixel;
	float res = TraceScreenSpaceRay1(vsPos, uniforms.vsSunDirection.xyz, uniforms.projSS, inputTexSize.xy, 0.001, uniforms.nearPlane, uniforms.stride, uniforms.jitter, 10, 0.1, hitPixel, hitPoint);

	return res;
}
