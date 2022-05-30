#pragma once
#include <d3d12.h>
#include <DirectXMath.h>

struct Light
{
	DirectX::XMMATRIX light_view;      
	DirectX::XMMATRIX light_projection;
	DirectX::XMFLOAT4 light_color;
	DirectX::XMFLOAT4 color_ambient;	
	DirectX::XMFLOAT3 lightPos;
	float range;
};