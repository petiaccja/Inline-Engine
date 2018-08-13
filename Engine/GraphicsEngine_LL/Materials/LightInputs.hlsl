#ifndef LIGHTCOUNT_IMPLEMENTATION
#define LIGHTCOUNT_IMPLEMENTATION return 0;
#endif

#ifndef LIGHTDIR_IMPLEMENTATION
#define LIGHTDIR_IMPLEMENTATION return float3(0,0,-1);
#endif

#ifndef LIGHTCOLOR_IMPLEMENTATION
#define LIGHTCOLOR_IMPLEMENTATION return float3(1,1,1);
#endif


int GetLightCount()
{
	LIGHTCOUNT_IMPLEMENTATION
}


float3 GetLightDir(int i)
{
	LIGHTDIR_IMPLEMENTATION
}


float3 GetLightColor(int i)
{
	LIGHTCOLOR_IMPLEMENTATION
}

void main(uint not_a_node) {}