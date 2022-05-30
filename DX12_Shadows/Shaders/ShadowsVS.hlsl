
cbuffer ModelCB: register(b0)
{
	float4x4 mvp;
	float4x4 model;
	float4 padding[8];
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
	res.pos = mul(res.worldPos, mvp);
	res.worldPos = mul(res.worldPos, model);
	res.normal = mul(float4(inp.normal, 1), model).xyz;
	res.color = mul(float4(inp.normal, 1), model); //inp.color;
	return res;
}