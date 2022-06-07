#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT3 _eye, DirectX::XMFLOAT3 _lookAt, DirectX::XMFLOAT3 _up)
{
	mEye = _eye;
	mLookAt = _lookAt;
	mUp = _up;

	mViewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mEye),
		DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mUp));
}

const DirectX::XMMATRIX& Camera::GetViewMatrix() const
{
	return mViewMx;
}

const DirectX::XMFLOAT3& Camera::GetEyePosition() const
{
	return mEye;
}

void Camera::SetEyePosition(const DirectX::XMFLOAT3& _eye)
{
	mEye = _eye;
	mViewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mEye),
		DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mUp));
}
