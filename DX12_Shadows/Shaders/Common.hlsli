
cbuffer ModelCB: register(b0)
{
	float4x4 mvp;
	float4x4 model;
	float4 padding[8];
};
