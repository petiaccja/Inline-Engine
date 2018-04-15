// Bias/scale helper functions
float2 undoVelocityBiasScale(float2 v)
{
	return v * 2.0 - 1.0;
}

float2 doVelocityBiasScale(float2 v)
{
	return (v + 1.0) * 0.5;
}

float2 encodeVelocity(float4 position, float4 prevPosition, float halfExposureFramerate, float maxMotionBlurRadius)
{
	float2 qx = ((position.xy / position.w)
		- (prevPosition.xy / prevPosition.w))
		* halfExposureFramerate;
	float lengthQX = length(qx);

	float weight = max(0.5, min(lengthQX, maxMotionBlurRadius)) / (lengthQX + 0.001);

	return doVelocityBiasScale(qx * weight);
}

//spheremap normal compression
float2 encodeNormal(float3 n)
{
	float p = sqrt(n.z * 8.0 + 8.0);
	return float2(n.xy / p + 0.5);
}

float3 decodeNormal(float2 enc)
{
	float2 fenc = enc * 4.0 - 2.0;
	float f = dot(fenc, fenc);
	float g = sqrt(1.0 - f / 4.0);
	float3 n;
	n.xy = fenc * g;
	n.z = 1.0 - f / 2.0;
	return n;
}

float3 ycocg_to_rgb_helper(float3 color)
{
	color.y -= 0.5;
	color.z -= 0.5;
	return float3(color.x + color.y - color.z, color.x + color.z, color.x - color.y - color.z);
}

float ycocg_edge_filter(float2 center, float2 a0, float2 a1, float2 a2, float2 a3)
{
	const float thresh = 30.0 / 255.0;

	float4 lum = float4(a0.x, a1.x, a2.x, a3.x);
	float4 w = 1.0 - step(thresh, abs(lum - center.x));
	float ww = w.x + w.y + w.z + w.w;

	//Handle the special case where all the weights are zero.
	//In HDR scenes it's better to set the chrominance to zero.
	//Here we just use the chrominance of the first neighbor.
	w.x = (ww == 0.0) ? 1.0 : w.x;
	ww = (ww == 0.0) ? 1.0 : ww;

	return (w.x * a0.y + w.y * a1.y + w.z * a2.y + w.w * a3.y) / ww;
}

float2 rgb_to_ycocg(float3 color, int2 frag_coord)
{
	float3 ycocg = float3(0.25 * color.x + 0.5 * color.y + 0.25 * color.z,
		0.5  * color.x - 0.5 * color.z + 0.5,
		-0.25 * color.x + 0.5 * color.y - 0.25 * color.z + 0.5);
	return
		((frag_coord.x & 1) == (frag_coord.y & 1))
		//( mod( frag_coord.x, 2.0 ) == mod( frag_coord.y, 2.0 ) )
		? ycocg.xz : ycocg.xy;
}

//tex is assumed to store ycocg in RG channels
float3 ycocg_to_rgb(int2 frag_coord, float2 center, float2 a0, float2 a1, float2 a2, float2 a3)
{
	//vec2 a0 = texelFetch(tex, ivec3(frag_coord + vec2(1, 0), index), 0).xy;
	//vec2 a1 = texelFetch(tex, ivec3(frag_coord - vec2(1, 0), index), 0).xy;
	//vec2 a2 = texelFetch(tex, ivec3(frag_coord + vec2(0, 1), index), 0).xy;
	//vec2 a3 = texelFetch(tex, ivec3(frag_coord - vec2(0, 1), index), 0).xy;
	float chroma = ycocg_edge_filter(center, a0, a1, a2, a3);

	float3 col = float3(center, chroma);
	col.xyz =
		((frag_coord.x & 1) == (frag_coord.y & 1))
		//( mod( frag_coord.x, 2.0 ) == mod( frag_coord.y, 2.0 ) )
		? col.xzy : col.xyz;
	return ycocg_to_rgb_helper(col);
}