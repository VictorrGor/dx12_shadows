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
	float4 viewTileMin = float4(outMinMaxPointClusters[iTile * 2], 1);//mul(float4(outMinMaxPointClusters[iTile * 2], 1), viewMx);
	float4 viewTileMax = float4(outMinMaxPointClusters[iTile * 2 + 1], 1);//mul(float4(outMinMaxPointClusters[iTile * 2 + 1], 1), viewMx);
	//float dist = sqDistancePointAABB(viewLightPos.xyz / viewLightPos.w, viewTileMin.xyz / viewTileMin.w, viewTileMax.xyz / viewTileMax.w);
	float dist = sqDistancePointAABB(viewLightPos.xyz, viewTileMin.xyz, viewTileMax.xyz);
	return radius * radius >= dist;
}

groupshared uint gClusterLightOffset;

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint compactOffset = DTid.x;
	uint iTile = compactActiveClusters[compactOffset];

	uint actualLight[ILIGHT_COUNT];
	uint counter = 0;
	for (uint iLight = 0; iLight < ILIGHT_COUNT; ++iLight)
	{
		if (testSphereAABBColision(iLight, iTile))
		{
			actualLight[counter] = iLight;
			counter += 1;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	
	if (counter)
	{
		uint clusterLightOffset = 0;
		InterlockedAdd(gClusterLightOffset, counter, clusterLightOffset);

		uint arrayIndex = iTile * 2;
		clusterNumLightPairs[float2(arrayIndex / 4096, arrayIndex % 4096)] = counter;
		arrayIndex = arrayIndex + 1;
		clusterNumLightPairs[float2(arrayIndex / 4096, arrayIndex % 4096)] = clusterLightOffset;

		for (uint iLight = 0; iLight < counter; ++iLight)
		{
			arrayIndex = (clusterLightOffset + iLight);
			clustersLightList[float2(arrayIndex / 4096, arrayIndex % 4096)] = actualLight[iLight];
		}
	}
}