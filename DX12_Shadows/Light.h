#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#define ILIGHT_COUNT 20

struct Light
{
	DirectX::XMFLOAT4 light_color;
	DirectX::XMFLOAT4 color_ambient;	
	DirectX::XMFLOAT3 lightPos;
	float range;
};

struct FrameLights
{
	Light mLights[ILIGHT_COUNT];
};