#pragma once
#include <d3d12.h>
#include <DirectXMath.h>


class Camera final
{
	DirectX::XMFLOAT3 m_eye;
	DirectX::XMFLOAT3 m_look_at;
	DirectX::XMFLOAT3 m_up;

	DirectX::XMMATRIX m_viewMx;
public:
	Camera(DirectX::XMFLOAT3 _eye, DirectX::XMFLOAT3 _look_at, DirectX::XMFLOAT3 _up = { 0, 1, 0 });
	const DirectX::XMMATRIX& GetViewMatrix() const;
	const DirectX::XMFLOAT3& GetEyePosition() const;
};