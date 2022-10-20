#define ILIGHT_COUNT 20

uint getDepthSlice(float z, uint nSlices, float zNear, float zFar)
{
	return (log(z) * (float)nSlices - (float)nSlices * log(zNear)) / log(zFar / zNear);
}

//pixelCoord - screen coordinates. tile size = (tile_pixel_size)/(screen_size)
uint getClusterIndex(float3 pixelCoord, float2 tileSizes, uint nSlices, float2 zNearFar, float2 dispatchSize)
{
	uint slice = getDepthSlice(pixelCoord.z, nSlices, zNearFar.x, zNearFar.y);
	uint3 clusterIndex = uint3(pixelCoord.x / tileSizes.x, pixelCoord.y / tileSizes.y, slice);
	return clusterIndex.x + clusterIndex.y * dispatchSize.x + clusterIndex.z * (dispatchSize.x * dispatchSize.y);
}
