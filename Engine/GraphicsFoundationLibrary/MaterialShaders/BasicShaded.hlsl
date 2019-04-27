float4 main(float4 diffuse)
{
    float3 lightColor = float3(0, 0, 0);
    for (int i = 0; i < GetLightCount(); ++i)
    {
        float coeff = saturate(-dot(GetLightDir(i), GetNormal()));
        lightColor += coeff * GetLightColor(i);
    }
    return float4(lightColor * diffuse.xyz, diffuse.w);
}