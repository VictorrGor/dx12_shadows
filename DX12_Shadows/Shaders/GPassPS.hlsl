struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct PS_OUT {
	float4 texture0 : SV_TARGET0;//R8G8B8A8
	float4 texture1 : SV_TARGET1;//R32G32B32A32
};

PS_OUT main(PS_INPUT inp)
{
	PS_OUT res = (PS_OUT)0;
	inp.normal = normalize(inp.normal);
	res.texture0 = float4(inp.color.xyz, sign(inp.normal.z));
	res.texture1 = float4(inp.normal.x, inp.normal.y, 0, 0);
	return res;
}