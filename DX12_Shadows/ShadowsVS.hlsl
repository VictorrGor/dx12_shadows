
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
	
	res.pos = float4(inp.pos, 1);
	res.normal = inp.normal;
	res.color = inp.color;
	return res;
}