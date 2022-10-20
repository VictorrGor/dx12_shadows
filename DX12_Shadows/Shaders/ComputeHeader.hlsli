
#include "CommonDefenitions.hlsli"



struct PointLight
{
	float4 color;
	float4 color_ambient;
	float3 position;
	float range;
};

struct cluster {   // A cluster volume is represented using an AABB
	float4 minPoint; // We use vec4s instead of a vec3 for memory alignment purposes
	float4 maxPoint;
};

cbuffer CSBuffer: register(b0)
{
	float2 tileSize; ///@todo Why float4
	float4x4 inverseProjection;
	uint2 screenSize;
	float zNear;
	float zFar;
	uint3 dispatchSize;
	uint numSlices; //== dispatchSize.z but (maybe not in all CS?)

	float4x4 viewMx;
}

cbuffer lights : register(b1)
{
	PointLight lights[ILIGHT_COUNT];
}

struct IndirectCommand
{
	uint thread_x;
	uint thread_y;
	uint thread_z;
};


RWTexture1D<float3>	outMinMaxPointClusters : register(u0);
//RWTexture1D<float3>	allPoints			   : register(u1); //clusters debug info
RWTexture1D<uint>	activeClusters		   : register(u2);
RWTexture1D<uint>	compactActiveClusters  : register(u3);
RWBuffer<uint>		rwActiveClusterCounter : register(u4);
RWTexture2D<uint>	clusterNumLightPairs   : register(u5);
RWTexture2D<uint>	clustersLightList	   : register(u6);

Texture2D zTexture : register(t0);
SamplerState ss : register(s0);