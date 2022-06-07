
cbuffer Camera: register(b0)
{
	float4x4 vp[4];
};

struct DrawConstants
{
	uint camNum;
};
ConstantBuffer<DrawConstants> actualCamera : register(b1);

cbuffer Model: register(b2)
{
	float4x4 model;
};


struct LightPoint
{
	float4 light_color;
	float4 color_ambient;
	float3 lightPos;
	float range;
};


struct VS_IN
{
	float3 pos : POSITION;
};


float4 main(VS_IN inp) : SV_POSITION
{
	return mul(mul(float4(inp.pos, 1), vp[1]), model);
}