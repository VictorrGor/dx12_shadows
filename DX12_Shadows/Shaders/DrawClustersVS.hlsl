
cbuffer cube :register(b0)
{
	float4x4 mvp;
};

RWTexture1D<float3> clusterPoints : register(u0);


struct OUT
{
	float4 clusterPoint : SV_POSITION;
	uint clusterID : INDEX;
};

OUT main(float3 pos : POSITION, uint index : SV_VertexID, uint instanceID : SV_InstanceID)
{
	OUT o = (OUT)0;
	o.clusterPoint = float4(clusterPoints[instanceID * 8 + index], 1);
	o.clusterPoint = mul(o.clusterPoint, mvp);
	o.clusterID = instanceID;
	return o;
}