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
	DirectX::XMMATRIX m_mvp;
	DirectX::XMMATRIX m_model;
	float padding[32];
};
static_assert((sizeof(ModelCBData) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

class ConstantBuffer
{
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_buffer;
	UINT8* m_pCbvDataBegin;
public:
	virtual void Initialize(Microsoft::WRL::ComPtr<ID3D12Device8> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc) = 0;
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAdress();
};

class ModelCB : public ConstantBuffer
{
	ModelCBData m_data;
	const UINT m_constantBufferSize;
	DirectX::XMMATRIX m_model;
	DirectX::XMMATRIX m_view;
	DirectX::XMMATRIX m_projection;
public:
	ModelCB();
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device8> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc);
	void SetResources(const DirectX::XMMATRIX& _model, const DirectX::XMMATRIX& _view, const DirectX::XMMATRIX& _projection);
	void SetModelMatrix(const DirectX::XMMATRIX& _model);
	void SetViewMatrix(const DirectX::XMMATRIX& _view);
	UINT getBufferSize();
	~ModelCB();
};


struct FrameCBData
{
	Light m_light;
	DirectX::XMFLOAT3 m_cameraPos;
	float padding[16];
};
static_assert((sizeof(FrameCBData) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

class FrameCB : public ConstantBuffer
{
	FrameCBData m_data;
	const UINT m_constantBufferSize;
public:
	FrameCB();
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device8> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc);
	void SetResources(const Light& _light, const DirectX::XMFLOAT3& _cameraPos);
	void SetCameraPosition(const DirectX::XMFLOAT3& _pos);
	void SetLightPos(const DirectX::XMFLOAT3& _light_pos);
	void SetLight(const Light& _light);
	UINT getBufferSize();
};