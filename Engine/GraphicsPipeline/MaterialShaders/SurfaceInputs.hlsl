void main(out float4 color, out float3 normal, out float2 texCoord0, out float3 tangent, out float3 bitangent)
{
    normal = GetNormal();
    tangent = GetTangent();
    bitangent = GetBitangent();
    color = GetColor();
    texCoord0 = GetTexcoord(0);
}