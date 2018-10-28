TextureCube<float> pointLightShadowMapTex : register(t400);

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

float3 GetPointLightShadow(float3 dir, float dist
#ifdef POINT_EXTENDED_INFO
, out float4 shadowCoord
#endif
)
{
	float3 wsDir = mul(float4(dir, 0.0), uniforms.invV).xyz;
	#ifdef POINT_EXTENDED_INFO
	shadowCoord = float4(wsDir, 1.0);
	#endif
	float depth = pointLightShadowMapTex.SampleLevel(theSampler, wsDir, 0.0).x;
	float linearDepth = LinearizeDepth(depth, 0.1, 100);
	float bias = 0.05;
	float shadowTerm = float(linearDepth > dist - bias);	
	return float3(shadowTerm, shadowTerm, shadowTerm);
	//return float3(linearDepth, linearDepth, linearDepth)*0.01;
	//return float3(depth, depth, depth);
	//return float3(dist, dist, dist);
	//return ws_dir;
	//return float3(ws_dir.xy, 0.0);
}
