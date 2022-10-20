///http://www.aortiz.me/2018/12/21/CG.html#clustered-shading

#include "ComputeHeader.hlsli"

float4 screen2View(float4 pointSS)
{
	float2 texCoord = pointSS.xy / screenSize.xy; 
	float4 clip = float4(float2(texCoord.x, 1. - texCoord.y) * 2 - 1, pointSS.z, pointSS.w);//NDC

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
	uint tileIndex = dispatchThreadID.x + dispatchSize.x * dispatchThreadID.y + dispatchSize.x * dispatchSize.y * dispatchThreadID.z;
	const float3 eyePos = float3(0, 0, 0);

	float tileNear = zNear * pow(zFar / zNear, float(dispatchThreadID.z) / dispatchSize.z);
	float tileFar = zNear * pow(zFar / zNear, float(dispatchThreadID.z + 1) / dispatchSize.z);

	float4 maxPointSSNear = float4(float(dispatchThreadID.x + 1) * tileSize.x, 
		float(dispatchThreadID.y + 1) * tileSize.y / screenSize.y, tileNear, 1);
	float4 minPointSSNear = float4(float(dispatchThreadID.x) * tileSize.x, 
		float(dispatchThreadID.y) * tileSize.y / screenSize.y, tileNear, 1);

	float4 maxPointSSFar = float4(float(dispatchThreadID.x + 1) * tileSize.x, 
		float(dispatchThreadID.y + 1) * tileSize.y, tileFar, 1);
	float4 minPointSSFar = float4(float(dispatchThreadID.x) * tileSize.x, 
		float(dispatchThreadID.y) * tileSize.y, tileFar, 1);

	float3 maxPointVSNear = screen2View(maxPointSSNear).xyz;
	float3 minPointVSNear = screen2View(minPointSSNear).xyz;
	float3 maxPointVSFar  = screen2View(maxPointSSFar).xyz;
	float3 minPointVSFar  = screen2View(minPointSSFar).xyz;

	//float3 minPointNear = lineIntersectionToZPlane(eyePos, minPointVS, tileNear);
	//float3 minPointFar  = lineIntersectionToZPlane(eyePos, minPointVS, tileFar);
	//float3 maxPointNear = lineIntersectionToZPlane(eyePos, maxPointVS, tileNear);
	//float3 maxPointFar  = lineIntersectionToZPlane(eyePos, maxPointVS, tileFar);

	float3 minPointAABB = min(min(minPointVSNear, minPointVSFar), min(maxPointVSNear, maxPointVSFar));
	float3 maxPointAABB = max(max(minPointVSNear, minPointVSFar), max(maxPointVSNear, maxPointVSFar));

	outMinMaxPointClusters[tileIndex * 2] = minPointAABB;
	outMinMaxPointClusters[tileIndex * 2 + 1] = maxPointAABB;
}