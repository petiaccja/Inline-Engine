float main(MapColor2D roughness) {
	return roughness.tex.Sample(roughness.samp, g_tex0).x;
}