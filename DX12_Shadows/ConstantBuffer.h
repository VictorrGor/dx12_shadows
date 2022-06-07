#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>
#include <Windows.h>

#include "Helpers.h"
#include "Light.h"

struct ModelCBData
{
	DirectX::XMMATRIX mMVP;
	DirectX::XMMATRIX mModel;
};

class ConstantBuffer
{
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> mBuffer;
	UINT8* mCbvDataBegin;
	D3D12_CPU_DESCRIPTOR_HANDLE mCPUDesc;
	UINT mBufferSize;
public:
	virtual void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc, UINT _objectsCount) = 0;
	ID3D12Resource* GetResource();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& _handle, UINT _offsetByFrame, UINT _frameCount, UINT _descriptorIncrimentSize);
};

class ModelCB : public ConstantBuffer
{
	ModelCBData mData;
	DirectX::XMMATRIX mModel;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProjection;
public:
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc, UINT _objectsCount);
	void SetResources(const DirectX::XMMATRIX& _model, const DirectX::XMMATRIX& _view, const DirectX::XMMATRIX& _projection, UINT _elemsOffset);
	void SetModelMatrix(const DirectX::XMMATRIX& _model, UINT _elemsOffset);
	void SetViewMatrix(const DirectX::XMMATRIX& _view, UINT _elemsOffset);
	~ModelCB();
};

struct FrameCBData
{
	Light mLight;
	DirectX::XMFLOAT3 mCameraPos;
};

class FrameCB : public ConstantBuffer
{
	FrameCBData mData;
public:
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc, UINT _frameBufferCount);
	void SetResources(const Light& _light, const DirectX::XMFLOAT3& _cameraPos, UINT _frameNum);
	void SetCameraPosition(const DirectX::XMFLOAT3& _pos, UINT _frameNum);
	void SetLightPos(const DirectX::XMFLOAT3& _light_pos, UINT _frameNum);
	void SetLight(const Light& _light, UINT _frameNum);
};