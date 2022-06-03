#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT3 _eye, DirectX::XMFLOAT3 _look_at, DirectX::XMFLOAT3 _up)
{
	m_eye = _eye;
	m_look_at = _look_at;
	m_up = _up;

	m_viewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_eye),
		DirectX::XMLoadFloat3(&m_look_at), DirectX::XMLoadFloat3(&m_up));
}

const DirectX::XMMATRIX& Camera::GetViewMatrix() const
{
	return m_viewMx;
}

const DirectX::XMFLOAT3& Camera::GetEyePosition() const
{
	return m_eye;
}

void Camera::SetEyePosition(const DirectX::XMFLOAT3& _eye)
{
	m_eye = _eye;
	m_viewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_eye),
		DirectX::XMLoadFloat3(&m_look_at), DirectX::XMLoadFloat3(&m_up));
}
