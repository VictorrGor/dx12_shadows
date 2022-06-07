#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <memory>
#include "ConstantBuffer.h"

struct FrameResource final
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	std::unique_ptr<ModelCB> mModelCB;
	std::unique_ptr<FrameCB> mFrameCB;
	UINT64 mFenceValue;

	FrameResource(ID3D12Device* _device, UINT _frameBufferCount, UINT _objectsCount, D3D12_CPU_DESCRIPTOR_HANDLE _hSRV);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
};
