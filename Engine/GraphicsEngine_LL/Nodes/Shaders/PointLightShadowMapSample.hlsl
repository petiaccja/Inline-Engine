TextureCube<float> pointLightShadowMapTex : register(t400);

float getPointLightShadow(float4 vs_pos)
{
	return pointLightShadowMapTex.SampleLevel(theSampler, float3(0,0,0), 0.0).x;
}
