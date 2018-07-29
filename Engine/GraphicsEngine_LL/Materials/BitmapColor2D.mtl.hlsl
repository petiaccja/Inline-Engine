float4 main(MapColor2D map) {
	return float4(GammaToLinear(map.tex.Sample(map.samp, g_tex0).xyz), 1.0);
}