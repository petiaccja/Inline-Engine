float4 main(MapColor2D map) {
	return map.tex.Sample(map.samp, g_tex0);
}