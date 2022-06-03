
cbuffer ModelCB: register(b0)
{
	float4x4 mvp;
	float4x4 model;
	float4 padding[8];
};


struct VS_IN
{
	float3 pos : POSITION;
};


float4 main(VS_IN inp) : SV_POSITION
{
	return mul(float4(inp.pos, 1), mvp);
}