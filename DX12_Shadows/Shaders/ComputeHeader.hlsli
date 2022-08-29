#define TG_SIZE_X 16
#define TG_SIZE_Y 9
#define TG_SIZE_Z 10

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

//ClusterGen
cbuffer CSBuffer: register(b0)
{
	float4 tileSize;
	float4x4 inverseProjection;
	uint2 screenSize;
	float zNear;
	float zFar;
	uint3 dispatchSize;
	uint numSlices;
}


RWTexture1D<float3> outMinMaxPointClusters : register(u0);
RWTexture1D<float3> allPoints : register(u1);

//BuildActiveClusters
RWTexture1D<int> activeClusters : register(u2);
Texture2D zTexture;
SamplerState ss : register(s0);

//BuildCompactCluster
RWTexture1D<uint> compactActiveClusters : register(u3);
RWStructedBuffer<uint> rwActiveClusterCounter : register(u4);

cbuffer dispatchSize : register(b1)
{
	float3 dispatchSize;
}


cbuffer lights : register(b2)
{
	PointLight lights[10];
}

cbuffer vp : register(b3)
{
	float4x4 viewMx;
}