void main(MapColor2D image, out float4 color, out float r, out float g, out float b, out float a, float2 coords = float2(1.0f / 0.0f, 1.0f / 0.0f))
{
    if (isinf(coords.x))
    {
        coords = GetTexcoord(0);
    }
    color = image.tex.Sample(image.samp, float3(coords, 0));
    r = color.r;
    g = color.g;
    b = color.b;
    a = color.a;
}