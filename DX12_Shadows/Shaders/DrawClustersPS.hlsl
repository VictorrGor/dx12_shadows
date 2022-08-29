struct INPUT
{
	float4 clusterPoint : SV_POSITION;
	uint clusterID : INDEX;
};

float4 main(INPUT inp) : SV_TARGET
{
	return float4(cos(inp.clusterID), cos(inp.clusterID), .0f, 1.0f);
}