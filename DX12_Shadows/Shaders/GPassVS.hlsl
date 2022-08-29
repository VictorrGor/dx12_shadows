
cbuffer Camera: register(b0)
{
	float4x4 vp[4];
};

cbuffer Model: register(b1)
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

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

VS_OUTPUT main(VS_INPUT inp)
{
	VS_OUTPUT res = (VS_OUTPUT)0;

	res.pos = mul(float4(inp.pos, 1), model);
	res.pos = mul(res.pos, vp[0]);
	res.normal = mul(float4(inp.normal, 1), model).xyz;
	res.color = mul(float4(inp.normal, 1), model); //inp.color;
	return res;
}