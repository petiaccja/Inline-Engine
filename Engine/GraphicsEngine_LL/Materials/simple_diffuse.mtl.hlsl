float4 main(float4 diffuse) {
	return float4(saturate(dot(-g_lightDir, g_normal)) * diffuse * 2.0 * g_lightColor, 1.0f) + float4(get_tiled_lighting(g_ndcPos, diffuse, g_normal, g_vsPos), 0.0);// +0.5*float4(0.6, 0, 0, 1.0f);;

	// Normal debugging
	//return float4(g_normal*0.5f+float3(0.5f, 0.5f, 0.5f), 1.0f);
}