#include "ComputeHeader.hlsli"

uint getDepthSlice(float z)
{
	return (log(z) * numSlices - numSlices * log(nearZ)) / log(farZ / nearZ);
}

uint getClusterIndex(float3 pixelCoord)
{
	uint slice = getDepthSlice(pixelCoord.z);
	uint3 clusterIndex = uint3(pixelCoord.x / tileSize.x, pixelCoord.y / tileSize.y, slice);
	return clusterIndex.x + clusterIndex.y * numClusters.x + clusterIndex.z * (numCluster.x * numCluster.z);
}

void markActiveClusters(uint2 pixelID)
{
	float2 screenCoord = pixelID.xy / screenSize.xy;
	float z = CameraDepthMap.Sample(ss, screenCoord);
	uint clusterID = getClusterIndex(float3(pixelID, z));
	activeClusters[clusterID] = 1;
}

[numthreads(1, 1, 1)]
void main( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	markActiveClusters(dispatchThreadID.xy);
}