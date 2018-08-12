#ifndef NORMAL_IMPLEMENTATION
#define NORMAL_IMPLEMENTATION return float3(0,0,1);
#endif

#ifndef TANGENT_IMPLEMENTATION
#define TANGENT_IMPLEMENTATION return float3(0,0,1);
#endif

#ifndef BITANGENT_IMPLEMENTATION
#define BITANGENT_IMPLEMENTATION return float3(0,0,1);
#endif

#ifndef COLOR_IMPLEMENTATION
#define COLOR_IMPLEMENTATION return float4(0,0,0,1);
#endif

#ifndef TEXCOORD_IMPLEMENTATION
#define TEXCOORD_IMPLEMENTATION return float2(0,0);
#endif


float3 GetNormal()
{
	NORMAL_IMPLEMENTATION
}

float3 GetTangent()
{
    TANGENT_IMPLEMENTATION
}

float3 GetBitangent()
{
    BITANGENT_IMPLEMENTATION
}

float4 GetColor()
{
	COLOR_IMPLEMENTATION
}

float2 GetTexcoord(int i)
{
	TEXCOORD_IMPLEMENTATION
}

void main(uint not_a_node) {}