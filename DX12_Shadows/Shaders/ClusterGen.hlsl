///http://www.aortiz.me/2018/12/21/CG.html#clustered-shading

#include "ComputeHeader.hlsli"

float4 screen2View(float4 pointSS)
{
	float2 texCoord = pointSS.xy / screenSize.xy; 
	float4 clip = float4(float2(texCoord.x, 1 - texCoord.y) * 2 - 1, pointSS.z, pointSS.w);//NDC

	float4 pointVS = mul(clip, inverseProjection);
	pointVS = pointVS / pointVS.w;

	return pointVS;
}

float3 lineIntersectionToZPlane(float3 A, float3 B, float zDistance)
{
	const float3 normal = float3(0, 0, 1);
	float3 ab = B - A;
	float t = (zDistance - dot(normal, A)) / dot(normal, ab);
	return A + ab * t;
}

[numthreads(1, 1, 1)]
void main( uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tileIndex = dispatchThreadID.x + dispatchSize.x * dispatchThreadID.y + 
		dispatchSize.x * dispatchSize.y * dispatchThreadID.z;
	const float3 eyePos = float3(0, 0, 0);

	float4 maxPointSS = float4((dispatchThreadID.x + 1) * tileSize.x, (dispatchThreadID.y + 1) * tileSize.y, 0, 1);
	float4 minPointSS = float4(dispatchThreadID.x * tileSize.x, dispatchThreadID.y * tileSize.y, 0, 1);

	float3 maxPointVS = screen2View(maxPointSS).xyz;
	float3 minPointVS = screen2View(minPointSS).xyz;

	float tileNear = zNear * pow(zFar / zNear, dispatchThreadID.z / float(TG_SIZE_Z));
	float tileFar  = zNear * pow(zFar / zNear, (dispatchThreadID.z + 1) / float(TG_SIZE_Z));

	float3 minPointNear = lineIntersectionToZPlane(eyePos, minPointVS, tileNear);
	float3 minPointFar  = lineIntersectionToZPlane(eyePos, minPointVS, tileFar);
	float3 maxPointNear = lineIntersectionToZPlane(eyePos, maxPointVS, tileNear);
	float3 maxPointFar  = lineIntersectionToZPlane(eyePos, maxPointVS, tileFar);

	float3 minPointAABB = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
	float3 maxPointAABB = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));

	outMinMaxPointClusters[tileIndex * 2] = minPointAABB;
	outMinMaxPointClusters[tileIndex * 2 + 1] = maxPointAABB;
	float3 diff = maxPointAABB - minPointAABB;
	allPoints[tileIndex * 8] = minPointAABB;
	allPoints[tileIndex * 8 + 1] = minPointAABB + float3(diff.x, 0, 0);
	allPoints[tileIndex * 8 + 2] = minPointAABB + float3(diff.x, 0, diff.z);
	allPoints[tileIndex * 8 + 3] = minPointAABB + float3(0     , 0, diff.z);
	allPoints[tileIndex * 8 + 4] = minPointAABB + float3(diff.x, diff.y, diff.z);
	allPoints[tileIndex * 8 + 5] = minPointAABB + float3(diff.x, diff.y, 0);
	allPoints[tileIndex * 8 + 6] = minPointAABB + float3(diff.x, diff.y, diff.z);
	allPoints[tileIndex * 8 + 7] = minPointAABB + float3(0	   , diff.y, diff.z);
}