
struct Transform
{
	float4x4 MVP;
};

struct Color
{
	float4 color;
};

ConstantBuffer<Transform> transform : register(b0);
ConstantBuffer<Color> color : register(b1);

struct PS_Input
{
	float4 position : SV_POSITION;
};

PS_Input VSMain(float2 position : POSITION)
{
	PS_Input result;

	float4 pos = {position.x, position.y, 0, 1};
	result.position = mul(transform.MVP, pos);

	return result;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
	return color.color;
}
