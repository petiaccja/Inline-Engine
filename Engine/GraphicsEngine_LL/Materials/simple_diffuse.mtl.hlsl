float4 main(float4 diffuse) {
	return dot(-g_lightDir, g_normal) * diffuse * g_lightColor;
}