#pragma once
#include <d3d12.h>
#include <DirectXMath.h>

#pragma pack(push, 1)
struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT4 color;
};
#pragma pack(pop)