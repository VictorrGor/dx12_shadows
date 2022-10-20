#include "ComputeHeader.hlsli"
//compactActiveClusters - u3
//activeClusters - u2
//rwActiveClusterCounter - u4
//cbuffer - b0


void buildCompactCluster(uint3 DTid : SV_DispatchThreadID)
{
	uint clusterID = DTid.x + DTid.y * dispatchSize.x + DTid.z * dispatchSize.x * dispatchSize.y;
	if (activeClusters[clusterID])
	{
		uint offset = 0; 
		InterlockedAdd(rwActiveClusterCounter[0], 1, offset);
		rwActiveClusterCounter[1] = 1;
		rwActiveClusterCounter[2] = 1;
		compactActiveClusters[offset] = clusterID;
	}
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	buildCompactCluster(DTid);
}