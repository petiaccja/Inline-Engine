TextureCube<float> pointLightShadowMapTex : register(t400);

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

float3 getPointLightShadow(float3 dir, float dist)
{
	float3 ws_dir = mul(float4(dir, 0.0), uniforms.invV).xyz;
	float depth = pointLightShadowMapTex.SampleLevel(theSampler, ws_dir, 0.0).x;
	float linearDepth = linearize_depth(depth, 0.1, 100);
	float bias = 0.05;
	float shadowTerm = float(linearDepth > dist - bias);
	return float3(shadowTerm, shadowTerm, shadowTerm);
	//return float3(linearDepth, linearDepth, linearDepth)*0.01;
	//return float3(depth, depth, depth);
	//return float3(dist, dist, dist);
	//return ws_dir;
	//return float3(ws_dir.xy, 0.0);
}
