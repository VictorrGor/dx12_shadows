#include "ComputeHeader.hlsli"

void markActiveClusters(uint2 pixelID)
{
	float2 screenCoord = pixelID.xy / screenSize.xy;
	float z = zTexture.SampleLevel(ss, screenCoord, 0).x;
	uint clusterID = getClusterIndex(float3(pixelID, z), tileSize.xy, numSlices, float2(zNear, zFar), dispatchSize.xy);
	activeClusters[clusterID] = 1;
}

[numthreads(1, 1, 1)]
void main( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	markActiveClusters(dispatchThreadID.xy);
}