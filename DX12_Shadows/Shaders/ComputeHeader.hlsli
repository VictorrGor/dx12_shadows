#define TG_SIZE_X 16
#define TG_SIZE_Y 9
#define TG_SIZE_Z 10
#include "CommonDefenitions.hlsli"



struct PointLight
{
	float4 color;
	float3 position;
	float range;
};

struct cluster {   // A cluster volume is represented using an AABB
	float4 minPoint; // We use vec4s instead of a vec3 for memory alignment purposes
	float4 maxPoint;
};

cbuffer CSBuffer: register(b0)
{
	float4 tileSize;
	float4x4 inverseProjection;
	uint2 screenSize;
	float zNear;
	float zFar;
	uint3 dispatchSize;
	uint numSlices;

	float4x4 viewMx;
}

cbuffer lights : register(b1)
{
	PointLight lights[ILIGHT_COUNT];
}


RWTexture1D<float3>			outMinMaxPointClusters : register(u0);
RWTexture1D<float3>			allPoints			   : register(u1);
RWTexture1D<int>			activeClusters		   : register(u2);
RWTexture1D<uint>			compactActiveClusters  : register(u3);
RWStructuredBuffer<uint>	rwActiveClusterCounter : register(u4);
RWTexture2D<uint>			clusterNumLightPairs   : register(u5);
RWTexture2D<uint>			clustersLightList	   : register(u6);
//RWStructuredBuffer<uint>	rwCLPairsCounter	   : register(u7);

Texture2D zTexture : register(t0);
SamplerState ss : register(s0);