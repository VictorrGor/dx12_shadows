#include "ClusterResources.h"

//void ClustersResources::CreatePSO(ID3D12Device8* _device)
//{
//}
//
//void ClustersResources::CreateResources(ID3D12Device8* _device)
//{
//	auto descPtrGPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mUAVDescHeapGPU->GetCPUDescriptorHandleForHeapStart());
//	auto descPtrCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mUAVDescHeapCPU->GetCPUDescriptorHandleForHeapStart());
//	mClusterGenBuf = std::make_unique<ConstantBuffer<ClusterGenData>>();
//	mClusterGenBuf->Initialize(_device, descPtrGPU, 1);
//	descPtrGPU.Offset(1, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//	descPtrCPU.Offset(1, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//
//	D3D12_RESOURCE_DESC uavDesc = CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT_R32G32B32A32_FLOAT,
//		mClusterCountX * mClusterCountY * mClusterCountZ, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
//	D3D12_UNORDERED_ACCESS_VIEW_DESC uavViewDesc = {};
//	uavViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
//	uavViewDesc.Texture1D.MipSlice = 0;
//	uavViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
//	
//
//	_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//		D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&mOutMinPointClusters));
//	_device->CreateUnorderedAccessView(mOutMinPointClusters.Get(), nullptr, &uavViewDesc, descPtrGPU);
//	_device->CreateUnorderedAccessView(mOutMinPointClusters.Get(), nullptr, &uavViewDesc, descPtrCPU);
//	descPtrGPU.Offset(1, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//	descPtrCPU.Offset(1, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//	_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//		D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&mOutMaxPointClusters));
//	_device->CreateUnorderedAccessView(mOutMinPointClusters.Get(), nullptr, &uavViewDesc, descPtrGPU);
//	_device->CreateUnorderedAccessView(mOutMinPointClusters.Get(), nullptr, &uavViewDesc, descPtrCPU);
//}
//
//ClustersResources::ClustersResources(ID3D12Device8* _device, float _zNear, float _zFar,
//	DirectX::XMMATRIX& _projection, DirectX::XMUINT2& _screenSize)
//{
//
//}
//
//void ClustersResources::CreateUAVDescriptorHeap(ID3D12Device8* _device)
//{
//}
//
//void ClustersResources::ComputeClusters(ID3D12Device8* _device)
//{
//	mComputeCA->Reset();
//	mComputeGCL->Reset(mComputeCA.Get(), nullptr);
//
//	FLOAT clearColor[4] = { 0,0,0,0 };
//	ID3D12DescriptorHeap* ppHeaps[] = { mUAVDescHeapGPU.Get() };
//	mComputeGCL->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
//	mComputeGCL->SetPipelineState(mGenClustersPSO.Get());
//	mComputeGCL->SetComputeRootSignature(mRS.Get());
//	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mOutMinPointClusters.Get(), 
//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	mComputeGCL->ResourceBarrier(1, &barrier);
//	barrier = CD3DX12_RESOURCE_BARRIER::Transition(mOutMaxPointClusters.Get(), 
//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	mComputeGCL->ResourceBarrier(1, &barrier);
//
//	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(mUAVDescHeapGPU->GetGPUDescriptorHandleForHeapStart());
//	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mUAVDescHeapCPU->GetCPUDescriptorHandleForHeapStart());
//	mComputeGCL->SetComputeRootDescriptorTable(0, mUAVDescHeapGPU->GetGPUDescriptorHandleForHeapStart());
//	hGPU.Offset(1, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//	hCPU.Offset(1, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//	mComputeGCL->ClearUnorderedAccessViewFloat(hGPU, hCPU, mOutMinPointClusters.Get(), clearColor, 0, NULL);
//	hGPU.Offset(1, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//	hCPU.Offset(1, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
//	mComputeGCL->ClearUnorderedAccessViewFloat(hGPU, hCPU, mOutMaxPointClusters.Get(), clearColor, 0, NULL);
//	
//	mComputeGCL->Dispatch(mClusterCountX, mClusterCountY, mClusterCountZ);
//	ThrowIfFailed(mComputeGCL->Close());
//
//	ID3D12CommandList* const comandList = { mComputeGCL.Get() };
//	mComputeCQ->ExecuteCommandLists(1, &comandList);
//	auto ++uFenceValue;
//	mComputeFence->Signal();
//}
