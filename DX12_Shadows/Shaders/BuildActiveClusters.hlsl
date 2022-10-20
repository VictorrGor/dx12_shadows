#include "ComputeHeader.hlsli"
//zTexture t0
//CSBuffer b0
//activeClusters u2


void markActiveClusters(uint2 pixelID)
{
	float2 screenCoord = float2((float)pixelID.x / (screenSize.x - 1), (float)pixelID.y / (screenSize.y - 1));//(float2)pixelID.xy / screenSize.xy;
	float z = zTexture.SampleLevel(ss, screenCoord, 0).x;
	uint clusterID = getClusterIndex(float3(screenCoord, z), tileSize.xy, numSlices, float2(zNear, zFar), dispatchSize.xy);
	activeClusters[clusterID] = 1;
}

[numthreads(1, 1, 1)]
void main( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	markActiveClusters(dispatchThreadID.xy);
}