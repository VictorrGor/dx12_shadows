#include "ComputeHeader.hlsli"


void buildCompactCluster(uint3 DTid : SV_DispatchThreadID)
{
	uint clusterID = DTid.x + DTid.y * clusterSize.x + DTid.z * clusterSize.x * clusterSize.y;
	if (activeClusters[clusterID])
	{
		uint offset = rwActiveClusterCounter.IncrementCounter();
		compactActiveClusters[offset] = clusterID;
	}
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	buildCompactCluster(DTid);
}