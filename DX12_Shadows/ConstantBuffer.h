#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>
#include <Windows.h>

#include "Helpers.h"
#include "Light.h"

//struct ModelCBData
//{
//	DirectX::XMMATRIX mMVP;
//	DirectX::XMMATRIX mModel;
//};
//
//class ConstantBuffer
//{
//protected:
//	Microsoft::WRL::ComPtr<ID3D12Resource> mBuffer;
//	UINT8* mCbvDataBegin;
//	D3D12_CPU_DESCRIPTOR_HANDLE mCPUDesc;
//	UINT mBufferSize;
//public:
//	virtual void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc, UINT _objectsCount) = 0;
//	ID3D12Resource* GetResource();
//	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& _handle, UINT _offsetByFrame, UINT _frameCount, UINT _descriptorIncrimentSize);
//};
//
//class ModelCB : public ConstantBuffer
//{
//	ModelCBData mData;
//	DirectX::XMMATRIX mModel;
//	DirectX::XMMATRIX mView;
//	DirectX::XMMATRIX mProjection;
//public:
//	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc, UINT _objectsCount);
//	void SetResources(const DirectX::XMMATRIX& _model, const DirectX::XMMATRIX& _view, const DirectX::XMMATRIX& _projection, UINT _elemsOffset);
//	void SetModelMatrix(const DirectX::XMMATRIX& _model, UINT _elemsOffset);
//	void SetViewMatrix(const DirectX::XMMATRIX& _view, UINT _elemsOffset);
//	~ModelCB();
//};
//
//struct FrameCBData
//{
//	Light mLight;
//	DirectX::XMFLOAT3 mCameraPos;
//};
//
//class FrameCB : public ConstantBuffer
//{
//	FrameCBData mData;
//public:
//	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc, UINT _frameBufferCount);
//	void SetResources(const Light& _light, const DirectX::XMFLOAT3& _cameraPos, UINT _frameNum);
//	void SetCameraPosition(const DirectX::XMFLOAT3& _pos, UINT _frameNum);
//	void SetLightPos(const DirectX::XMFLOAT3& _light_pos, UINT _frameNum);
//	void SetLight(const Light& _light, UINT _frameNum);
//};

struct CameraCBData
{
	DirectX::XMMATRIX mVPMx[4];
};

struct FrameCBData
{
	Light mLL;
	DirectX::XMFLOAT3 mCameraPos;
};

template<typename TData>
class ConstantBuffer
{
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> mBuffer;
	UINT8* mCbvDataBegin;
	D3D12_CPU_DESCRIPTOR_HANDLE mCPUDesc;
	UINT mBufferSize;
	TData mData;
public:
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _CPUDesc, UINT _objectsCount)
	{
	    mBufferSize = CalcConstantBufferByteSize(sizeof(TData));
	    ThrowIfFailed(_device->CreateCommittedResource(
	        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
	        D3D12_HEAP_FLAG_NONE,
	        &CD3DX12_RESOURCE_DESC::Buffer(mBufferSize * _objectsCount),
	        D3D12_RESOURCE_STATE_GENERIC_READ,
	        nullptr,
	        IID_PPV_ARGS(&mBuffer)));
	
	    mCPUDesc = _CPUDesc;
	
	    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	    cbvDesc.BufferLocation = mBuffer->GetGPUVirtualAddress();
	    cbvDesc.SizeInBytes = mBufferSize;
	
	    UINT handleOffset = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mCPUDesc);
	    for (UINT iObjects = 0; iObjects < _objectsCount; ++iObjects)
	    {
	        _device->CreateConstantBufferView(&cbvDesc, handle);
	        cbvDesc.BufferLocation += mBufferSize;
	        handle.Offset(handleOffset);
	    }
	
	    CD3DX12_RANGE readRange(0, 0);      
	    ThrowIfFailed(mBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mCbvDataBegin)), "Cannt create CB!");
	};
	ConstantBuffer()
	{
		mCbvDataBegin = nullptr;
	}
	~ConstantBuffer()
	{
		if (mCbvDataBegin)
		{
			mBuffer->Unmap(0, nullptr);
			mCbvDataBegin = nullptr;
		}
	};
	//CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& _handle, UINT _offsetByFrame, UINT _frameCount, UINT _descriptorIncrimentSize);
	void SetResources(const TData& _data, UINT _elementsOffset)
	{
		mData = _data;
		memcpy(mCbvDataBegin + _elementsOffset * mBufferSize, &mData, sizeof(TData));
	};
	/*void SetResources(void* _data, UINT _dataSize, UINT _dataOffset, UINT _elementsOffset)
	{
		memcpy(&mData + _dataOffset, _data, _dataSize);
		memcpy(mCbvDataBegin + _elementsOffset * mBufferSize, &mData, sizeof(TData));
	};*/
};