float4 main(float4 diffuse) {
	return float4(saturate(dot(-g_lightDir, g_normal)) * diffuse * g_lightColor, 1.0f) + 0.5*float4(0.6, 0, 0, 1.0f);;
}