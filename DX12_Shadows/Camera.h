#pragma once
#include <d3d12.h>
#include <DirectXMath.h>


class Camera final
{
	DirectX::XMFLOAT3 mEye;
	DirectX::XMFLOAT3 mLookAt;
	DirectX::XMFLOAT3 mUp;
	DirectX::XMFLOAT3 mArrow;
	DirectX::XMMATRIX mViewMx;
public:
	Camera(DirectX::XMFLOAT3 _eye, DirectX::XMFLOAT3 _lookAt, DirectX::XMFLOAT3 _up = { 0, 1, 0 });
	const DirectX::XMMATRIX& GetViewMatrix() const;
	const DirectX::XMFLOAT3& GetEyePosition() const;
	void SetEyePosition(const DirectX::XMFLOAT3& _eye);
	
	void MoveForward (float _speed = 1.);
	void MoveBackward(float _speed = 1.);
	void MoveLeft    (float _speed = 1.);
	void MoveRight   (float _speed = 1.);
	void RotateCamera(int _deltaX, int _deltaY);
};