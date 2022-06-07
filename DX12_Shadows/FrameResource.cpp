#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* _device, UINT _frameBufferCount, UINT _objectsCount, D3D12_CPU_DESCRIPTOR_HANDLE _hSRV)
{
	mFenceValue = 0;

	mModelCB = std::make_unique<ModelCB>();
	mModelCB->Initialize(_device, _hSRV, _objectsCount * _frameBufferCount);
	_hSRV.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * _objectsCount * _frameBufferCount;
	mFrameCB = std::make_unique<FrameCB>();
	mFrameCB->Initialize(_device, _hSRV, _frameBufferCount);
}