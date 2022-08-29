#include "ComputeHeader.hlsli"

float sqDistancePointAABB(float3 pt, float3 tileMin, float3 tileMax)
{
	float sqDist = 0;
	if (pt.x < tileMin.x)
	{
		sqDist += (tileMin.x - pt.x) * (tileMin.x - pt.x);
	}
	if (pt.x > tileMax.x)
	{
		sqDist += (pt.x - tileMax.x) * (pt.x - tileMax.x);
	}
	if (pt.y < tileMin.y)
	{
		sqDist += (tileMin.y - pt.y) * (tileMin.y - pt.y);
	}
	if (pt.y > tileMax.y)
	{
		sqDist += (pt.y - tileMax.y) * (pt.y - tileMax.y);
	}
	if (pt.z < tileMin.z)
	{
		sqDist += (tileMin.z - pt.z) * (tileMin.z - pt.z);
	}
	if (pt.z > tileMax.z)
	{
		sqDist += (pt.z - tileMax.z) * (pt.z - tileMax.z);
	}
	return sqDist;
}

bool testSphereAABBColision(uint iLight, uint iTile)
{
	float radius = lights[iLight].range;
	float4 viewLightPos = mul(float4(lights[iLight].position, 1), viewMx);
	float4 viewTileMin = mul(float4(outMinMaxPointClusters[iTile * 2].position, 1), viewMx);
	float4 viewTileMax = mul(float4(outMinMaxPointClusters[iTile * 2 + 1].position, 1), viewMx);
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint compactOffset = DTid.x
	testSphereAABBColision();
}