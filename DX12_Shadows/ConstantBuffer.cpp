//#include "ConstantBuffer.h"
//
//
//void ModelCB::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _CPUDesc, UINT _objectsCount)
//{
//    mBufferSize = CalcConstantBufferByteSize(sizeof(ModelCBData));
//    ThrowIfFailed(_device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//        D3D12_HEAP_FLAG_NONE,
//        &CD3DX12_RESOURCE_DESC::Buffer(mBufferSize * _objectsCount),
//        D3D12_RESOURCE_STATE_GENERIC_READ,
//        nullptr,
//        IID_PPV_ARGS(&mBuffer)));
//
//    mCPUDesc = _CPUDesc;
//
//    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
//    cbvDesc.BufferLocation = mBuffer->GetGPUVirtualAddress();
//    cbvDesc.SizeInBytes = mBufferSize;
//
//    UINT handleOffset = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mCPUDesc);
//    for (UINT iObjects = 0; iObjects < _objectsCount; ++iObjects)
//    {
//        _device->CreateConstantBufferView(&cbvDesc, handle);
//        cbvDesc.BufferLocation += mBufferSize;
//        handle.Offset(handleOffset);
//    }
//
//    CD3DX12_RANGE readRange(0, 0);      
//    ThrowIfFailed(mBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mCbvDataBegin)), "Cannt create CB!");
//  }
//
//void ModelCB::SetResources(const DirectX::XMMATRIX& _model, const DirectX::XMMATRIX& _view, const DirectX::XMMATRIX& _projection, UINT _elemsOffset)
//{
//    mModel = DirectX::XMMatrixTranspose(_model);
//    mView = DirectX::XMMatrixTranspose(_view);
//    mProjection = DirectX::XMMatrixTranspose(_projection);
//
//    mData.mModel = mModel;
//	mData.mMVP = mProjection * mView * mModel;
//    memcpy(mCbvDataBegin + _elemsOffset * mBufferSize, &mData, sizeof(ModelCBData));
//}
//
//ModelCB::~ModelCB()
//{ 
//    mBuffer->Unmap(0, nullptr);
//}
//
//void ModelCB::SetModelMatrix(const DirectX::XMMATRIX& _model, UINT _elemsOffset)
//{
//    mModel = DirectX::XMMatrixTranspose(_model);
//    mData.mModel = mModel;
//    mData.mMVP = mProjection * mView * mModel;
//    memcpy(mCbvDataBegin + _elemsOffset * mBufferSize, &mData, sizeof(ModelCBData));
//}
//
//void ModelCB::SetViewMatrix(const DirectX::XMMATRIX& _view, UINT _elemsOffset)
//{
//    mView = DirectX::XMMatrixTranspose(_view);
//    mData.mMVP = mProjection * mView * mModel;
//    memcpy(mCbvDataBegin + _elemsOffset * mBufferSize, &mData, sizeof(ModelCBData));
//}
//
//void FrameCB::SetResources(const Light& _light, const DirectX::XMFLOAT3& _cameraPos, UINT _frameNum)
//{
//    mData.mLight = _light;
//    mData.mCameraPos = _cameraPos;
//    memcpy(mCbvDataBegin + _frameNum * mBufferSize, &mData, sizeof(FrameCBData));
//}
//
//void FrameCB::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _CPUDesc, UINT _frameBufferCount)
//{
//    mBufferSize = CalcConstantBufferByteSize(sizeof(FrameCBData));
//    ThrowIfFailed(_device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//        D3D12_HEAP_FLAG_NONE,
//        &CD3DX12_RESOURCE_DESC::Buffer(mBufferSize * _frameBufferCount),
//        D3D12_RESOURCE_STATE_GENERIC_READ,
//        nullptr,
//        IID_PPV_ARGS(&mBuffer)));
//    mCPUDesc = _CPUDesc;
//
//    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mCPUDesc);
//    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
//    cbvDesc.BufferLocation = mBuffer->GetGPUVirtualAddress();
//    cbvDesc.SizeInBytes = mBufferSize;
//    UINT handleOffset = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//    for (UINT iFrame = 0; iFrame < _frameBufferCount; ++iFrame)
//    {
//        _device->CreateConstantBufferView(&cbvDesc, handle);
//        cbvDesc.BufferLocation += mBufferSize;
//        handle.Offset(handleOffset);
//    }
//    ThrowIfFailed(mBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mCbvDataBegin)), "Cannt create CB!");
//}
//
//void FrameCB::SetCameraPosition(const DirectX::XMFLOAT3& _pos, UINT _frameNum)
//{
//    mData.mCameraPos = _pos;
//    memcpy(mCbvDataBegin + _frameNum * mBufferSize, &mData, sizeof(FrameCBData));
//}
//
//void FrameCB::SetLightPos(const DirectX::XMFLOAT3& _light_pos, UINT _frameNum)
//{
//    mData.mLight.lightPos = _light_pos;
//    memcpy(mCbvDataBegin + _frameNum * mBufferSize, &mData, sizeof(FrameCBData));
//}
//
//void FrameCB::SetLight(const Light& _light, UINT _frameNum)
//{
//    mData.mLight = _light;
//    memcpy(mCbvDataBegin + _frameNum * mBufferSize, &mData, sizeof(FrameCBData));
//}
//
//ID3D12Resource* ConstantBuffer::GetResource()
//{
//    return mBuffer.Get();
//}
//
//
//CD3DX12_GPU_DESCRIPTOR_HANDLE ConstantBuffer::GetGPUDescriptorHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& _handle, 
//                    UINT _offsetByFrame, UINT _frameCount, UINT _descriptorIncrimentSize)
//{
//    return CD3DX12_GPU_DESCRIPTOR_HANDLE(_handle, _offsetByFrame * _frameCount, _descriptorIncrimentSize);
//}
