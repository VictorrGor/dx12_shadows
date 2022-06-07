#pragma once
#include <d3d12.h>
#include <DirectXMath.h>

struct Light
{
	DirectX::XMFLOAT4 light_color;
	DirectX::XMFLOAT4 color_ambient;	
	DirectX::XMFLOAT3 lightPos;
	float range;
};