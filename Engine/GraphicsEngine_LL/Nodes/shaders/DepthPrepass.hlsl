
struct Transform
{
	float4x4 MVP;
};


ConstantBuffer<Transform> transform : register(b0);

struct PS_Input
{
	float4 position : SV_POSITION;
};


PS_Input VSMain(float4 position : POSITION)
{
	PS_Input result;

    result.position = mul(position, transform.MVP);

	return result;
}


void PSMain(PS_Input input)
{
}
