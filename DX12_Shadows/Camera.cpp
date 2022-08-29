#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT3 _eye, DirectX::XMFLOAT3 _lookAt, DirectX::XMFLOAT3 _up)
{
	mEye = _eye;
	mLookAt = _lookAt;
	mUp = _up;
	DirectX::XMStoreFloat3(&mArrow, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mEye))));

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
	DirectX::XMStoreFloat3(&mArrow, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mEye))));
	mViewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mEye),
		DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mUp));
}

void Camera::MoveForward(float _speed)
{
	auto scaledArrow = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&mArrow), _speed);
	DirectX::XMStoreFloat3(&mEye, DirectX::XMVectorAdd(scaledArrow, DirectX::XMLoadFloat3(&mEye)));
	DirectX::XMStoreFloat3(&mLookAt, DirectX::XMVectorAdd(scaledArrow, DirectX::XMLoadFloat3(&mLookAt)));
	mViewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mEye),
		DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mUp));
}


void Camera::MoveBackward(float _speed)
{
	auto scaledArrow = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&mArrow), -_speed);
	DirectX::XMStoreFloat3(&mEye, DirectX::XMVectorAdd(scaledArrow, DirectX::XMLoadFloat3(&mEye)));
	DirectX::XMStoreFloat3(&mLookAt, DirectX::XMVectorAdd(scaledArrow, DirectX::XMLoadFloat3(&mLookAt)));
	mViewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mEye),
		DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mUp));
}

void Camera::MoveLeft(float _speed)
{
	auto scaledArrow = DirectX::XMVectorScale(DirectX::XMVector3Normalize(
		DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&mArrow), DirectX::XMLoadFloat3(&mUp))) , _speed);
	DirectX::XMStoreFloat3(&mEye, DirectX::XMVectorAdd(scaledArrow, DirectX::XMLoadFloat3(&mEye)));
	DirectX::XMStoreFloat3(&mLookAt, DirectX::XMVectorAdd(scaledArrow, DirectX::XMLoadFloat3(&mLookAt)));
	mViewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mEye),
		DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mUp));
}

void Camera::MoveRight(float _speed)
{
	auto scaledArrow = DirectX::XMVectorScale(DirectX::XMVector3Normalize(
		DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&mArrow), DirectX::XMLoadFloat3(&mUp))), -_speed);
	DirectX::XMStoreFloat3(&mEye, DirectX::XMVectorAdd(scaledArrow, DirectX::XMLoadFloat3(&mEye)));
	DirectX::XMStoreFloat3(&mLookAt, DirectX::XMVectorAdd(scaledArrow, DirectX::XMLoadFloat3(&mLookAt)));
	mViewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mEye),
		DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mUp));
}

void Camera::RotateCamera(int _deltaX, int _deltaY)
{
	float rotation_speed = 0.01;
	float XoZAngle = rotation_speed * static_cast<float>(_deltaX);
	float YoZAngle = rotation_speed * static_cast<float>(_deltaY);
	auto vUp = DirectX::XMLoadFloat3(&mUp);
	auto vLookAt = DirectX::XMLoadFloat3(&mLookAt);
	auto vEye = DirectX::XMLoadFloat3(&mEye);
	vLookAt = DirectX::XMVectorSubtract(vLookAt, vEye);
	auto vRotatedLook = DirectX::XMVector3Rotate(vLookAt, DirectX::XMQuaternionRotationAxis(vUp, XoZAngle));
	//vRotatedLook = DirectX::XMVectorAdd(vRotatedLook, vEye);
	//DirectX::XMStoreFloat3(&mLookAt, vRotatedLook);
	auto vAxis = DirectX::XMVector3Cross(vUp, vRotatedLook);
	DirectX::XMStoreFloat3(&mLookAt, DirectX::XMVector3Rotate(vRotatedLook, DirectX::XMQuaternionRotationAxis(vAxis, YoZAngle)));
	mViewMx = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mEye),
		DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mUp));
	DirectX::XMStoreFloat3(&mArrow, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&mLookAt), DirectX::XMLoadFloat3(&mEye))));
}