float main(MapColor2D metalness) {
	return metalness.tex.Sample(metalness.samp, g_tex0).x;
}