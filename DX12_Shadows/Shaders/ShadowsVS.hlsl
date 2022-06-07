
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

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

VS_OUTPUT main(VS_INPUT inp)
{
	VS_OUTPUT res = (VS_OUTPUT)0;
	
	res.worldPos = float4(inp.pos, 1);
	res.worldPos = mul(res.worldPos, model);

	res.pos = mul(res.worldPos, vp[0]);
	res.normal = mul(float4(inp.normal, 1), model).xyz;
	res.color = mul(float4(inp.normal, 1), model); //inp.color;
	return res;
}