#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>
#include <Windows.h>

#include "Helpers.h"
#include "Light.h"

struct CameraCBData
{
	DirectX::XMMATRIX mVPMx[12];
};

struct FrameCBData
{
	Light mLL[ILIGHT_COUNT];
	DirectX::XMFLOAT3 mCameraPos;
	UINT mNumSlices;
	DirectX::XMFLOAT2 mTileSize;
	DirectX::XMFLOAT2 mNearFar;
	DirectX::XMUINT3 mDispatchSize;
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