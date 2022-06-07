
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

cbuffer FrameCB: register(b3)
{
	LightPoint ll;
	float3 cameraPos;
};
