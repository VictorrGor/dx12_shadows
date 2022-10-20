#include "Render.h"


void RenderSys::CreateForwardCBDesriptorHeap()
{
	mCBDescHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 15, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	CreateFrameResources();
	CreateConstantBuffers();
	mShadowMap = std::make_unique<ShadowMap>(mDevice, mWidth, mHeight);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetCPUDescriptorHandleForHeapStart());
	hSRV.Offset(8, mSRVDesricptorHandleOffset);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDSV = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDSDescHeap->GetCPUDescriptorHandleForHeapStart());
	hDSV.Offset(1, mSRVDesricptorHandleOffset);
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU_SRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetGPUDescriptorHandleForHeapStart());
	hGPU_SRV.Offset(8, mSRVDesricptorHandleOffset);
	mShadowMap->SetDescriptors(hSRV, hGPU_SRV, hDSV);
}

void RenderSys::CreateDeferredCBDescriptionHead()
{
	mCBDescHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 25, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	auto CBHead = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetCPUDescriptorHandleForHeapStart());

	CreateFrameResources();
	CreateConstantBuffers();
	mShadowMap = std::make_unique<ShadowMap>(mDevice, mWidth, mHeight);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hSRV = CBHead;
	hSRV.Offset(8, mSRVDesricptorHandleOffset);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDSV = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDSDescHeap->GetCPUDescriptorHandleForHeapStart());
	hDSV.Offset(1, mSRVDesricptorHandleOffset);
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU_SRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetGPUDescriptorHandleForHeapStart());
	hGPU_SRV.Offset(8, mSRVDesricptorHandleOffset);
	mShadowMap->SetDescriptors(hSRV, hGPU_SRV, hDSV);
	
	auto hGBufferResource = CBHead; hGBufferResource.Offset(9, mSRVDesricptorHandleOffset);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	mDevice->CreateShaderResourceView(mGTexture[0].Get(), &srvDesc, hGBufferResource);

	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hGBufferResource.Offset(1, mSRVDesricptorHandleOffset);
	mDevice->CreateShaderResourceView(mGTexture[1].Get(), &srvDesc, hGBufferResource);

	auto hCameraCB = hGBufferResource; hCameraCB.Offset(1, mSRVDesricptorHandleOffset);
	//mDeferredCameraCB = std::make_unique<ConstantBuffer<DirectX::XMMATRIX>>();
	//mDeferredCameraCB->Initialize(pDevice, hCameraCB, g_numFrames);

	auto hCameraDepthMap = hCameraCB; hCameraDepthMap.Offset(g_numFrames, mSRVDesricptorHandleOffset);
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	mDevice->CreateShaderResourceView(mDepthStencilBuffer.Get(), &srvDesc, hCameraDepthMap);
}

void RenderSys::CreateClusterShader()
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	ComPtr<ID3DBlob> error, shader;
	CompileShader(L"Shaders/ClusterGen.hlsl", nullptr, nullptr, "main", "cs_6_0", compileFlags, 0, &shader, &error);

}

void RenderSys::CreateIndirectStaff()
{
	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
	
	D3D12_COMMAND_SIGNATURE_DESC commandSignDesc = {};
	commandSignDesc.pArgumentDescs = argumentDescs;
	commandSignDesc.NumArgumentDescs = _countof(argumentDescs);
	commandSignDesc.ByteStride = 12;
	mDevice->CreateCommandSignature(&commandSignDesc, nullptr, IID_PPV_ARGS(&mCommandSignature));

	mCommmandBufferSize = sizeof(IndirectCommand);
	ThrowIfFailed(mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(mCommmandBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mCommandBufferUpload)));
}

void RenderSys::Initialize(HWND& hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	mWidth = rc.right - rc.left; mHeight = rc.bottom - rc.top;
	mCamera = std::make_unique<Camera>(DirectX::XMFLOAT3(1.5, 1.5, 0), DirectX::XMFLOAT3(0, 0, 0));
	mVSync = true;
	mTearingSupported = CheckTearingSupport();

#ifdef _DEBUG
	D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugInterface));
	mDebugInterface->EnableDebugLayer();
#endif

	auto adapter = GetAdapter();
	CreateDevice(adapter);
	mCQ = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	mRTVDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mSRVDesricptorHandleOffset = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateDepthStencil();
	CreateRootSignatue();
	CreatePSOs();

	CreateSwapChain(hWnd, mWidth, mHeight, g_numFrames);
	mRTVDescHeap.Swap(CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, g_numFrames + g_numFrames * 2)); // GBuffer textures count * g_numFrames + g_numFrames

	UpdateRenderTargetViews();
	for (int i = 0; i < g_numFrames; ++i)
	{
		mCommandAllocator[i] = CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
	mGCL = CreateCommandList(mCommandAllocator[mCurrentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

	mFence = CreateFence();
	mFenceEvent = CreateEventHandle();

	mViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(mWidth), static_cast<float>(mHeight));
	mScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight));
	
	CreateDeferredCBDescriptionHead();
	//Deferred Plane
	Vertex box[4]; box[0].position = { -1, 1, 0 }; box[1].position = { 1, 1, 0 }; box[2].position = { -1, -1, 0 }; box[3].position = { 1, -1, 0 };
	UINT indices[] = { 0, 1, 2, 2, 1, 3 };
	mDefferedPassScreen = std::unique_ptr<Entity>(CreateEntity(box, 4, indices, 6));
	
	DirectX::XMUINT2 screen = { mWidth, mHeight };
	InitilizeComputeShaderResources();
	CreateIndirectStaff();
}

void RenderSys::GPass()
{
	mGCL->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		mDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
	auto backBuffer = mBackBuffer[mCurrentBackBufferIndex];

	mGCL->SetPipelineState(mPSOs["GPass"].Get());
	mGCL->SetGraphicsRootSignature(mRSs["GPass"].Get());

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mDSDescHeap->GetCPUDescriptorHandleForHeapStart());
	mGCL->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0, 0, 0, nullptr);
	mGCL->RSSetViewports(1, &mViewport);
	mGCL->RSSetScissorRects(1, &mScissorRect);

	for (UINT iGTexture = 0; iGTexture < mGTextureCount; ++iGTexture)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE gHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			mRTVDescHeap->GetCPUDescriptorHandleForHeapStart(), g_numFrames + iGTexture, mRTVDescriptorSize);
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mGTexture[iGTexture].Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		mGCL->ResourceBarrier(1, &barrier);
		mGCL->ClearRenderTargetView(gHandle, clearColor, 0, nullptr);
	}

	auto gHandleHead = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRTVDescHeap->GetCPUDescriptorHandleForHeapStart(), g_numFrames, mRTVDescriptorSize);
	mGCL->OMSetRenderTargets(mGTextureCount, &gHandleHead, TRUE, &dsvHandle);

	auto CBHeadHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetGPUDescriptorHandleForHeapStart());
	auto cameraHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CBHeadHandle, 4 + mCurrentBackBufferIndex, mSRVDesricptorHandleOffset);
	mGCL->SetGraphicsRootDescriptorTable(0, cameraHandle);
	mGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto it = mObjects.begin(); it != mObjects.end(); ++it)
	{
		auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetGPUDescriptorHandleForHeapStart());
		UINT handleOffset = mCurrentBackBufferIndex * 2 + (it - mObjects.begin());
		handle.Offset(handleOffset, mSRVDesricptorHandleOffset);
		mFrameResources->mModelCB->SetResources(DirectX::XMMatrixTranspose((*it)->GetModelMatrix()), handleOffset);
		mGCL->SetGraphicsRootDescriptorTable(1, handle);
		(*it)->SetVertexIndexBufferView(mGCL);
		(*it)->DrawIndexed(mGCL);
	}
	for (UINT iGTexture = 0; iGTexture < mGTextureCount; ++iGTexture)
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(mGTexture[iGTexture].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		mGCL->ResourceBarrier(1, &barrier);
	}
	mGCL->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		mDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

}

void RenderSys::DeferredShading()
{
	auto backBuffer = mBackBuffer[mCurrentBackBufferIndex];
	FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
	mGCL->SetPipelineState(mPSOs["deferred"].Get());
	mGCL->SetGraphicsRootSignature(mRSs["deferred"].Get());

	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRTVDescHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrentBackBufferIndex, mRTVDescriptorSize);
	mGCL->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);
	mGCL->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	auto CBHead = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetGPUDescriptorHandleForHeapStart());
	mGCL->SetGraphicsRootDescriptorTable(0, CD3DX12_GPU_DESCRIPTOR_HANDLE(CBHead, 9, mSRVDesricptorHandleOffset));
	mGCL->SetGraphicsRootDescriptorTable(1, CD3DX12_GPU_DESCRIPTOR_HANDLE(CBHead, 10, mSRVDesricptorHandleOffset));
	mGCL->SetGraphicsRootDescriptorTable(2, CD3DX12_GPU_DESCRIPTOR_HANDLE(CBHead, 13, mSRVDesricptorHandleOffset));
	mGCL->SetGraphicsRootDescriptorTable(3, CD3DX12_GPU_DESCRIPTOR_HANDLE(CBHead, 4 + mCurrentBackBufferIndex, mSRVDesricptorHandleOffset));
	mGCL->SetGraphicsRootDescriptorTable(4, CD3DX12_GPU_DESCRIPTOR_HANDLE(CBHead, 6 + mCurrentBackBufferIndex, mSRVDesricptorHandleOffset));
	
	mGCL->SetGraphicsRootDescriptorTable(5, CD3DX12_GPU_DESCRIPTOR_HANDLE(CBHead, 14, mSRVDesricptorHandleOffset));
	mGCL->SetGraphicsRootDescriptorTable(6, CD3DX12_GPU_DESCRIPTOR_HANDLE(CBHead, 15, mSRVDesricptorHandleOffset));


	mFrameData.mCameraPos = mCamera->GetEyePosition();
	mFrameResources->mFrameCB->SetResources(mFrameData, mCurrentBackBufferIndex);

	mDefferedPassScreen->SetVertexIndexBufferView(mGCL);
	mDefferedPassScreen->DrawIndexed(mGCL);
}

void RenderSys::ComputeClusters()
{
	mGenData.tileSize = { static_cast<float>(mWidth) / mClusterCountX, static_cast<float>(mHeight) / mClusterCountY };
	mGenData.inverseProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, mProjection));
	mGenData.screenSize = DirectX::XMUINT2(mWidth, mHeight);
	mGenData.zNear = mZNear;
	mGenData.zFar = mZFar;
	mGenData.dispatchSize = DirectX::XMUINT3(mClusterCountX, mClusterCountY, mClusterCountZ);
	mGenData.numSlices = mClusterCountZ;
	mGenData.viewMx = DirectX::XMMatrixIdentity();
	mClusterCSData->SetResources(mGenData, 0);

	mComputeCA->Reset();
	mComputeGCL->Reset(mComputeCA.Get(), mPSOs["clusterGen"].Get());

	FLOAT clearColor[4] = { 0,0,0,0 };
	ID3D12DescriptorHeap* ppHeaps[] = { mUAVDescHeapGPU.Get() };
	mComputeGCL->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	mComputeGCL->SetPipelineState(mPSOs["clusterGen"].Get());
	mComputeGCL->SetComputeRootSignature(mRSs["clusterGen"].Get());

	auto hGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(mUAVDescHeapGPU->GetGPUDescriptorHandleForHeapStart());
	auto hCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mUAVDescHeapCPU->GetCPUDescriptorHandleForHeapStart());
	//CSBuffer
	mComputeGCL->SetComputeRootDescriptorTable(0, hGPU);
	hGPU.Offset(2, mSRVDesricptorHandleOffset);
	hCPU.Offset(2, mSRVDesricptorHandleOffset);
	//outMinMaxPoints
	mComputeGCL->ClearUnorderedAccessViewFloat(hGPU, hCPU, mOutMinMaxPointClusters.Get(), clearColor, 0, NULL);
	mComputeGCL->SetComputeRootDescriptorTable(2, hGPU);
	
	mComputeGCL->Dispatch(mClusterCountX, mClusterCountY, mClusterCountZ);
	ThrowIfFailed(mComputeGCL->Close());

	ID3D12CommandList* const comandList = { mComputeGCL.Get() };
	mComputeCQ->ExecuteCommandLists(1, &comandList);
	mComputeFenceValue = Signal(mComputeCQ, mComputeFence, mComputeFenceValue);
	WaitForFenceValue(mComputeFence, mComputeFenceValue, mComputeFenceEvent);
}

void RenderSys::BuildLightClusterList()
{
	UINT clearColor[4] = { 0,0,0,0 };
	{
		mComputeCA->Reset();
		mComputeGCL->Reset(mComputeCA.Get(), mPSOs["BuildActiveClusters"].Get());
		ID3D12DescriptorHeap* ppHeaps[] = { mUAVDescHeapGPU.Get() };
		mComputeGCL->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		mComputeGCL->SetPipelineState(mPSOs["BuildActiveClusters"].Get());
		mComputeGCL->SetComputeRootSignature(mRSs["clusterGen"].Get());

		CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(mUAVDescHeapGPU->GetGPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mUAVDescHeapCPU->GetCPUDescriptorHandleForHeapStart());
		mGenData.dispatchSize = DirectX::XMUINT3(mClusterCountX, mClusterCountY, mClusterCountZ);
		mGenData.numSlices = mClusterCountZ;
		mGenData.tileSize = {static_cast<float>(mWidth) / mClusterCountX / mWidth, 
								static_cast<float>(mHeight) / mClusterCountY / mHeight };
		mClusterCSData->SetResources(mGenData, 0);
		mComputeGCL->SetComputeRootDescriptorTable(0, hGPU);
		hGPU.Offset(3, mSRVDesricptorHandleOffset);
		hCPU.Offset(3, mSRVDesricptorHandleOffset);
		//mComputeGCL->ClearUnorderedAccessViewFloat(hGPU, hCPU, mOutMinMaxPointClusters.Get(), clearColor, 0, NULL);
		mComputeGCL->ClearUnorderedAccessViewUint(hGPU, hCPU, mActiveClusters.Get(), clearColor, 0, NULL);
		mComputeGCL->SetComputeRootDescriptorTable(3, hGPU);
		hGPU.Offset(5, mSRVDesricptorHandleOffset);
		hCPU.Offset(5, mSRVDesricptorHandleOffset);
		mComputeGCL->SetComputeRootDescriptorTable(8, hGPU);

		mComputeGCL->Dispatch(mWidth, mHeight, 1);
		ThrowIfFailed(mComputeGCL->Close());

		ID3D12CommandList* const comandList = { mComputeGCL.Get() };
		mComputeCQ->ExecuteCommandLists(1, &comandList);
		mComputeFenceValue = Signal(mComputeCQ, mComputeFence, mComputeFenceValue);
		WaitForFenceValue(mComputeFence, mComputeFenceValue, mComputeFenceEvent);
	}

	{
		mComputeCA->Reset();
		mComputeGCL->Reset(mComputeCA.Get(), mPSOs["BuildCompactCluster"].Get());
		ID3D12DescriptorHeap* ppHeaps[] = { mUAVDescHeapGPU.Get() };
		mComputeGCL->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		mComputeGCL->SetComputeRootSignature(mRSs["clusterGen"].Get());

		CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(mUAVDescHeapGPU->GetGPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mUAVDescHeapCPU->GetCPUDescriptorHandleForHeapStart());
		
		//mGenData.dispatchSize = DirectX::XMUINT3(mWidth, mHeight, 1);
		mGenData.numSlices = mClusterCountZ;
		mClusterCSData->SetResources(mGenData, 0);
		mComputeGCL->SetComputeRootDescriptorTable(0, hGPU); //CB
		hGPU.Offset(3, mSRVDesricptorHandleOffset);
		hCPU.Offset(3, mSRVDesricptorHandleOffset);
		mComputeGCL->SetComputeRootDescriptorTable(3, hGPU); //Active
		hGPU.Offset(1, mSRVDesricptorHandleOffset);
		hCPU.Offset(1, mSRVDesricptorHandleOffset);
		mComputeGCL->SetComputeRootDescriptorTable(4, hGPU); //Compact
		mComputeGCL->ClearUnorderedAccessViewUint(hGPU, hCPU, mCompactClusters.Get(), clearColor, 0, NULL);
		hGPU.Offset(1, mSRVDesricptorHandleOffset);
		hCPU.Offset(1, mSRVDesricptorHandleOffset);
		
		mComputeGCL->SetComputeRootDescriptorTable(5, hGPU);
		mComputeGCL->ClearUnorderedAccessViewUint(hGPU, hCPU, mRWActiveClusters.Get(), clearColor, 0, NULL);
		
		mComputeGCL->Dispatch(mClusterCountX, mClusterCountY, mClusterCountZ);//(mWidth, mHeight, 1);
		ThrowIfFailed(mComputeGCL->Close());

		ID3D12CommandList* const comandList = { mComputeGCL.Get() };
		mComputeCQ->ExecuteCommandLists(1, &comandList);
		mComputeFenceValue = Signal(mComputeCQ, mComputeFence, mComputeFenceValue);
		WaitForFenceValue(mComputeFence, mComputeFenceValue, mComputeFenceEvent);
	}
	{
		mComputeCA->Reset();
		mComputeGCL->Reset(mComputeCA.Get(), mPSOs["BuildLightCluster"].Get());
		ID3D12DescriptorHeap* ppHeaps[] = { mUAVDescHeapGPU.Get() };
		mComputeGCL->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		mComputeGCL->SetComputeRootSignature(mRSs["clusterGen"].Get());

		mGenData.viewMx = DirectX::XMMatrixTranspose(mCamera->GetViewMatrix() * mProjection);
		mClusterCSData->SetResources(mGenData, 0);

		CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(mUAVDescHeapGPU->GetGPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mUAVDescHeapCPU->GetCPUDescriptorHandleForHeapStart());

		mComputeGCL->SetComputeRootDescriptorTable(0, hGPU);
		hGPU.Offset(1, mSRVDesricptorHandleOffset);
		mComputeGCL->SetComputeRootDescriptorTable(1, hGPU);
		hGPU.Offset(1, mSRVDesricptorHandleOffset);
		mComputeGCL->SetComputeRootDescriptorTable(2, hGPU);
		hGPU.Offset(2, mSRVDesricptorHandleOffset);
		mComputeGCL->SetComputeRootDescriptorTable(4, hGPU);
		hGPU.Offset(2, mSRVDesricptorHandleOffset);
		mComputeGCL->SetComputeRootDescriptorTable(6, hGPU);
		hCPU.Offset(6, mSRVDesricptorHandleOffset);
		mComputeGCL->ClearUnorderedAccessViewUint(hGPU, hCPU, mClusterNumLightPairs.Get(), clearColor, 0, NULL);
		hGPU.Offset(1, mSRVDesricptorHandleOffset);
		hCPU.Offset(1, mSRVDesricptorHandleOffset);
		mComputeGCL->SetComputeRootDescriptorTable(7, hGPU);
		mComputeGCL->ClearUnorderedAccessViewUint(hGPU, hCPU, mClustersLightList.Get(), clearColor, 0, NULL);
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mRWActiveClusters.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
		mComputeGCL->ResourceBarrier(1, &barrier);
		mComputeGCL->ExecuteIndirect(mCommandSignature.Get(), 1, mRWActiveClusters.Get(), 0, nullptr, 0);
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(mRWActiveClusters.Get(),
			D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		ThrowIfFailed(mComputeGCL->Close());

		ID3D12CommandList* const comandList = { mComputeGCL.Get() };
		mComputeCQ->ExecuteCommandLists(1, &comandList);
		mComputeFenceValue = Signal(mComputeCQ, mComputeFence, mComputeFenceValue);
		WaitForFenceValue(mComputeFence, mComputeFenceValue, mComputeFenceEvent);
	}
}

void RenderSys::Render()
{
	ComputeClusters();
	BuildLightClusterList();
	//Update();
	auto commandAllocator = mCommandAllocator[mCurrentBackBufferIndex];
	auto backBuffer = mBackBuffer[mCurrentBackBufferIndex];

	commandAllocator->Reset();
	mGCL->Reset(commandAllocator.Get(), nullptr);
	FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mGCL->ResourceBarrier(1, &barrier);

	{
		ID3D12DescriptorHeap* ppHeaps[] = { mCBDescHeap.Get() };
		mGCL->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		mCameraCB.mVPMx[0] = DirectX::XMMatrixTranspose(mCamera->GetViewMatrix() * mProjection);
		mCameraCB.mVPMx[1] = DirectX::XMMatrixInverse(nullptr, mCameraCB.mVPMx[0]);
		mFrameResources->mCameraCB->SetResources(mCameraCB, mCurrentBackBufferIndex);

		GPass();
		DeferredShading();
	}

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), 
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mGCL->ResourceBarrier(1, &barrier);

	ThrowIfFailed(mGCL->Close());
	ID3D12CommandList* const commandLists[] = { mGCL.Get() };
	mCQ->ExecuteCommandLists(1, commandLists);
	UINT syncInterval = mVSync ? 1 : 0;
	UINT presentFlags = mTearingSupported && !mVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
	ThrowIfFailed(mSwapChain->Present(syncInterval, presentFlags));
	mFrameFenceValues[mCurrentBackBufferIndex] = Signal(mCQ, mFence, mFenceValue);
	mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
	WaitForFenceValue(mFence, mFrameFenceValues[mCurrentBackBufferIndex], mFenceEvent);

}

ComPtr<IDXGIAdapter3> RenderSys::GetAdapter()
{
	ComPtr<IDXGIAdapter1> pAdapter1;
	ComPtr<IDXGIAdapter3> pAdapter3;

	ComPtr<IDXGIFactory5> pFactory;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&pFactory));

	SIZE_T maxDedicatedMemory = 0;
	for (UINT i = 0; pFactory->EnumAdapters1(i, &pAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC1 dxgiDesc;
		pAdapter1->GetDesc1(&dxgiDesc);
		
		if ((dxgiDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && 
			SUCCEEDED(D3D12CreateDevice(pAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
			dxgiDesc.DedicatedVideoMemory > maxDedicatedMemory)
		{
			maxDedicatedMemory = dxgiDesc.DedicatedVideoMemory;
			ThrowIfFailed(pAdapter1.As(&pAdapter3));
		}
	}

	return pAdapter3;
}

void RenderSys::CreateDevice(ComPtr<IDXGIAdapter3> adapter)
{
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice)), "Can't create device!");
#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(mDevice.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
	}
#endif
}

ComPtr<ID3D12CommandQueue> RenderSys::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)), "Cann't create command queue!\n");

	return d3d12CommandQueue;
}

bool RenderSys::CheckTearingSupport()
{
	BOOL allowTearing = FALSE;

	ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	{
		ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5)))
		{
			if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
			{
				allowTearing = FALSE;
			}
		}
	}

	return allowTearing == TRUE;
}

void RenderSys::CreateSwapChain(HWND hWnd, uint32_t width, uint32_t height, uint32_t bufferCount)
{
	ComPtr<IDXGIFactory4> dxgiFactory4;

	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(mCQ.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1),
		"Cann't createa SwapChain!\n");


	ThrowIfFailed(swapChain1.As(&mSwapChain));
}

ComPtr<ID3D12DescriptorHeap> RenderSys::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flag)
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	desc.Flags = _flag;

	ThrowIfFailed(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
}

ComPtr<ID3D12CommandAllocator> RenderSys::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ThrowIfFailed(mDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)), "Cannt create command allocator!\n");
	return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> RenderSys::CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ThrowIfFailed(mDevice->CreateCommandList(0, type, commandAllocator.Get(), mPSOs["shadows"].Get(), IID_PPV_ARGS(&commandList)),
		"Cannt create command List!\n");
	ThrowIfFailed(commandList->Close(), "Cannt close command List!\n");
	return commandList;
}

ComPtr<ID3D12Fence> RenderSys::CreateFence()
{
	ComPtr<ID3D12Fence> fence;
	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	return fence;
}

HANDLE RenderSys::CreateEventHandle()
{
	HANDLE fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fence_event && "Failed to create fence event");
	return fence_event;
}

void RenderSys::UpdateRenderTargetViews()
{
	UINT rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVDescHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < g_numFrames; ++i)
	{
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		mDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		mBackBuffer[i] = backBuffer;
		rtvHandle.Offset(rtvDescriptorSize);
	}
	mGTexture.reserve(2 * g_numFrames);
	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	
	for (UINT i = 0; i <  g_numFrames; ++i)
	{
		mGTexture.push_back(nullptr);
		D3D12_RESOURCE_DESC resourceDesc;
		resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, mWidth, mHeight,
			1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		optimizedClearValue.DepthStencil.Depth = 1.0f;
		optimizedClearValue.DepthStencil.Stencil = 0;

		mDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, &optimizedClearValue, IID_PPV_ARGS(&mGTexture.back()));
		mDevice->CreateRenderTargetView(mGTexture.back().Get(), nullptr, rtvHandle);
		rtvHandle.Offset(rtvDescriptorSize);


		mGTexture.push_back(nullptr);
		resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32G32B32A32_FLOAT, mWidth, mHeight,
			1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

		optimizedClearValue.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		optimizedClearValue.DepthStencil.Depth = 1.0f;
		optimizedClearValue.DepthStencil.Stencil = 0;

		mDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, &optimizedClearValue, IID_PPV_ARGS(&mGTexture.back()));
		mDevice->CreateRenderTargetView(mGTexture.back().Get(), nullptr, rtvHandle);
		rtvHandle.Offset(rtvDescriptorSize);
	}
}

uint64_t RenderSys::Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue)
{
	uint64_t fenceValueForSignal = ++fenceValue;
	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));

	return fenceValueForSignal;
}

void RenderSys::WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent,
	std::chrono::milliseconds duration)
{
	if (fence->GetCompletedValue() < fenceValue)
	{
		ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
		WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
	}
}

void RenderSys::Flush()
{
	uint64_t fenceValueForSignal = Signal(mCQ, mFence, mFenceValue);
	WaitForFenceValue(mFence, fenceValueForSignal, mFenceEvent);
}

void RenderSys::CreateRootSignatue()
{
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[4];
		CD3DX12_ROOT_PARAMETER1 rootParameters[5];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);

		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);   //Camera
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);//Model
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL); //ShadowMap
		rootParameters[3].InitAsConstants(2, 1, 0, D3D12_SHADER_VISIBILITY_ALL);			   //Const
		rootParameters[4].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL); //Frame
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		const CD3DX12_STATIC_SAMPLER_DESC samplerState(0, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &samplerState, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRSs["shadows"])));
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
		CD3DX12_ROOT_PARAMETER1 rootParameters[3];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[2].InitAsConstants(2, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRSs["zTest"])));
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[7];
		CD3DX12_ROOT_PARAMETER1 rootParameters[7];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
		ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);   
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[4].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[5].InitAsDescriptorTable(1, &ranges[5], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[6].InitAsDescriptorTable(1, &ranges[6], D3D12_SHADER_VISIBILITY_PIXEL);
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		const CD3DX12_STATIC_SAMPLER_DESC samplerState(0, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &samplerState, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRSs["deferred"])));
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRSs["GPass"])));
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1]);
		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RSDesc(2, rootParameters, 0, nullptr, flags);

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;
		HRESULT hRes = D3DX12SerializeVersionedRootSignature(&RSDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
		ThrowWithMessage(hRes, error.Get());
		ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRSs["clusterDraw"])));
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[9];
		CD3DX12_ROOT_PARAMETER1 rootParametrs[9];

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
		ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
		ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
		ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
		ranges[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 6, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

		ranges[8].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

		rootParametrs[0].InitAsDescriptorTable(1, &ranges[0]);
		rootParametrs[1].InitAsDescriptorTable(1, &ranges[1]);

		rootParametrs[2].InitAsDescriptorTable(1, &ranges[2]);
		rootParametrs[3].InitAsDescriptorTable(1, &ranges[3]);
		rootParametrs[4].InitAsDescriptorTable(1, &ranges[4]);
		rootParametrs[5].InitAsDescriptorTable(1, &ranges[5]);
		rootParametrs[6].InitAsDescriptorTable(1, &ranges[6]);
		rootParametrs[7].InitAsDescriptorTable(1, &ranges[7]);
		rootParametrs[8].InitAsDescriptorTable(1, &ranges[8]);
		CD3DX12_STATIC_SAMPLER_DESC samplerState(0, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RSDesc(_countof(rootParametrs), rootParametrs, 1, &samplerState);

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;
		HRESULT hRes = D3DX12SerializeVersionedRootSignature(&RSDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
		ThrowWithMessage(hRes, error.Get());
		ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRSs["clusterGen"])));
	}
}

void RenderSys::CreatePSOs()
{

	WCHAR szFilePath[MAX_PATH];
	GetModuleFileNameW(NULL, szFilePath, MAX_PATH);
	std::wstring exePath(szFilePath);
	exePath = exePath.substr(0, exePath.find_last_of(L"/\\"));
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	D3D12_INPUT_ELEMENT_DESC ia_desc[] = { 
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3DReadFileToBlob((exePath + std::wstring(L"/ShadowsVS.cso")).c_str(), &vertexShader), "Cannot load ShadowsVS!");
		ThrowIfFailed(D3DReadFileToBlob((exePath + std::wstring(L"/ShadowsPS.cso")).c_str(), &pixelShader), "Cannot load ShadowsPS!");
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { ia_desc, _countof(ia_desc) };
		psoDesc.pRootSignature = mRSs["shadows"].Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["shadows"])), "Cannot create PSO!");
	}
	{
		ComPtr<ID3DBlob> zTestShader;
		ThrowIfFailed(D3DReadFileToBlob((exePath + std::wstring(L"/ZTest.cso")).c_str(), &zTestShader), "Cannot load ZTest shader!");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoZDesc = {};
		psoZDesc.InputLayout = { ia_desc, _countof(ia_desc) };
		psoZDesc.pRootSignature = mRSs["zTest"].Get();
		psoZDesc.VS = CD3DX12_SHADER_BYTECODE(zTestShader.Get());
		//psoZDesc.PS = 
		psoZDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoZDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoZDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoZDesc.SampleMask = UINT_MAX;
		psoZDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoZDesc.NumRenderTargets = 0;
		psoZDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		psoZDesc.SampleDesc.Count = 1;
		psoZDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoZDesc, IID_PPV_ARGS(&mPSOs["zTest"])), "Cannot create zTest PSO!");
	}
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3DReadFileToBlob((exePath + std::wstring(L"/GPassVS.cso")).c_str(), &vertexShader), "Cannot load GPassVS!");
		ThrowIfFailed(D3DReadFileToBlob((exePath + std::wstring(L"/GPassPS.cso")).c_str(), &pixelShader), "Cannot load GPassPS");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { ia_desc, _countof(ia_desc) };
		psoDesc.pRootSignature = mRSs["GPass"].Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 2;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["GPass"])), "Cannot create GPass PSO!");
	}
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3DReadFileToBlob((exePath + std::wstring(L"/DeferredVS.cso")).c_str(), &vertexShader), "Cannot load DeferredVS!");
		ThrowIfFailed(D3DReadFileToBlob((exePath + std::wstring(L"/DeferredPS.cso")).c_str(), &pixelShader), "Cannot load DeferredPS!");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { ia_desc, _countof(ia_desc) };
		psoDesc.pRootSignature = mRSs["deferred"].Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		//psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["deferred"])), "Cannot create Deferred PSO!");
	}
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> error;
		//L"DrawClustersVS.cso"
		ThrowIfFailed(D3DReadFileToBlob((exePath + std::wstring(L"/DrawClustersVS.cso")).c_str(), &vertexShader), "Cannot load DrawClustersVS!");
		ThrowIfFailed(D3DReadFileToBlob((exePath + std::wstring(L"/DrawClustersPS.cso")).c_str(), &pixelShader), "Cannot load DrawClustersPS!");

		D3D12_INPUT_ELEMENT_DESC ia_desc_cluster[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{"INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }

		};
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { ia_desc_cluster, _countof(ia_desc_cluster) };
		psoDesc.pRootSignature = mRSs["clusterDraw"].Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		//psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["clusterDraw"])), "Cannot create clusterDraw PSO!");
	}
	{
		Microsoft::WRL::ComPtr<ID3DBlob> shader;
		ThrowIfFailed(D3DReadFileToBlob((exePath.c_str() + std::wstring(L"/ClusterGen.cso")).c_str(), &shader), "Cannot load clusterGen shader!");

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.CS = CD3DX12_SHADER_BYTECODE(shader.Get());
		psoDesc.pRootSignature = mRSs["clusterGen"].Get();
		psoDesc.NodeMask = 0;
		ThrowIfFailed(mDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["clusterGen"])), "Cannot create clusterGen PSO!");
	}
	{
		Microsoft::WRL::ComPtr<ID3DBlob> shader;
		ThrowIfFailed(D3DReadFileToBlob((exePath.c_str() + std::wstring(L"/BuildActiveClusters.cso")).c_str(), &shader), "Cannot load BuildActiveClusters shader!");

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.CS = CD3DX12_SHADER_BYTECODE(shader.Get());
		psoDesc.pRootSignature = mRSs["clusterGen"].Get();
		psoDesc.NodeMask = 0;
		ThrowIfFailed(mDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["BuildActiveClusters"])), "Cannot create BuildActiveClusters PSO!");
	}
	{
		Microsoft::WRL::ComPtr<ID3DBlob> shader;
		ThrowIfFailed(D3DReadFileToBlob((exePath.c_str() + std::wstring(L"/BuildCompactCluster.cso")).c_str(), &shader), "Cannot load BuildCompactCluster shader!");

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.CS = CD3DX12_SHADER_BYTECODE(shader.Get());
		psoDesc.pRootSignature = mRSs["clusterGen"].Get();
		psoDesc.NodeMask = 0;
		ThrowIfFailed(mDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["BuildCompactCluster"])), "Cannot create BuildCompactCluster PSO!");
	}
	{
		Microsoft::WRL::ComPtr<ID3DBlob> shader;
		ThrowIfFailed(D3DReadFileToBlob((exePath.c_str() + std::wstring(L"/BuildLightCluster.cso")).c_str(), &shader), "Cannot load BuildLightCluster shader!");

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.CS = CD3DX12_SHADER_BYTECODE(shader.Get());
		psoDesc.pRootSignature = mRSs["clusterGen"].Get();
		psoDesc.NodeMask = 0;
		ThrowIfFailed(mDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["BuildLightCluster"])), "Cannot create BuildLightCluster PSO!");
	}
}

RenderSys::~RenderSys()
{
	for (auto it = mObjects.begin(); it != mObjects.end(); ++it)
	{
		delete (*it);
		(*it) = nullptr;
	}
	mObjects.clear();
	CloseHandle(mFenceEvent);
}

void RenderSys::Update()
{
	static uint64_t frameCounter = 0;
	static double elapsedSeconds = 0.0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	frameCounter++;
	auto t1 = clock.now();
	auto deltaTime = t1 - t0;
	t0 = t1;
	elapsedSeconds += deltaTime.count() * 1e-9;

	float angle = DirectX::XM_2PI * static_cast<float>(elapsedSeconds) / 6;
	static const float R = 2.;
	static const float H = 1.5;
	DirectX::XMFLOAT3 camPos = { sinf(angle) * R, H, cos(angle) * R };
	mCamera->SetEyePosition(camPos);
	if (elapsedSeconds >= 6)
	{
		elapsedSeconds = 0;
	}
}

Entity* RenderSys::CreateEntity(Vertex* _pVtx, UINT _uVertexCount, UINT* _pIndecies, UINT _uIndexCount)
{
	Entity* res = new Entity();
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vb_view;
	{
		const UINT uVBSize = static_cast<UINT>(sizeof(Vertex) * _uVertexCount);

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uVBSize);
		ThrowIfFailed(mDevice->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pVertexBuffer)), "Cannot create vertex Buffer");

		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(pVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, _pVtx, uVBSize);
		pVertexBuffer->Unmap(0, nullptr);

		vb_view.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
		vb_view.SizeInBytes = uVBSize;
		vb_view.StrideInBytes = sizeof(Vertex); 
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> pIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW ib_view;
	{
		const UINT uIBSize = static_cast<UINT>(sizeof(UINT) * _uIndexCount);
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uIBSize);
		ThrowIfFailed(mDevice->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pIndexBuffer)), "Cannot create index Buffer");

		UINT8* pIndexDataBegin;
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(pIndexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
		memcpy(pIndexDataBegin, _pIndecies, uIBSize);
		pIndexBuffer->Unmap(0, nullptr);

		ib_view.BufferLocation = pIndexBuffer->GetGPUVirtualAddress();
		ib_view.SizeInBytes = uIBSize;
		ib_view.Format = DXGI_FORMAT_R32_UINT;
	}


	res->Initialize(_pVtx, _uVertexCount, pVertexBuffer, vb_view, _pIndecies, _uIndexCount, pIndexBuffer, ib_view);
	
	return res;
}

void RenderSys::DrawObject(Vertex* _pVtx, const UINT& _uVertexCount, UINT* _pIndices, const UINT& _uIndicesCount)
{
	mObjects.push_back(CreateEntity(_pVtx, _uVertexCount, _pIndices, _uIndicesCount));
}

void RenderSys::MoveForward()
{
	mCamera->MoveForward();
}

void RenderSys::MoveBackward()
{
	mCamera->MoveBackward();
}

void RenderSys::MoveLeft()
{
	mCamera->MoveLeft();
}

void RenderSys::MoveRight()
{
	mCamera->MoveRight();
}

void RenderSys::RotateCamera(int _deltaX, int _deltaY)
{
	if (_deltaX || _deltaY) mCamera->RotateCamera(_deltaX, _deltaY);
}

void RenderSys::CreateFrameResources()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mCBDescHeap->GetCPUDescriptorHandleForHeapStart());
	UINT objectCount = 2;

	mFrameResources = std::make_unique<FrameResource>(mDevice.Get(), g_numFrames, objectCount, handle);
}

RenderSys::RenderSys()
{
	mWidth = 0;
	mHeight = 0;
}

void RenderSys::DrawSceneToShdowMap()
{
	mGCL->RSSetViewports(1, &mShadowMap->GetViewport());
	mGCL->RSSetScissorRects(1, &mShadowMap->GetScissorRect());
	mGCL->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->GetResource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));

	mGCL->SetPipelineState(mPSOs["zTest"].Get());
	mGCL->SetGraphicsRootSignature(mRSs["zTest"].Get());

	mGCL->ClearDepthStencilView(mShadowMap->Dsv(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	mGCL->OMSetRenderTargets(0, nullptr, false, &mShadowMap->Dsv());

	mGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&mLights.mLights[0].lightPos);
	DirectX::XMVECTOR lookAt = DirectX::XMVectorSet(0, 0, 0, 0);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
	DirectX::XMMATRIX viewMx = DirectX::XMMatrixLookAtLH(eyePos, lookAt, up);

	mCameraCB.mVPMx[1] = DirectX::XMMatrixTranspose(viewMx * mProjection);
	mFrameResources->mCameraCB->SetResources(mCameraCB, mCurrentBackBufferIndex);
	mGCL->SetGraphicsRootDescriptorTable(0, CD3DX12_GPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetGPUDescriptorHandleForHeapStart(), 
						4 + mCurrentBackBufferIndex, mSRVDesricptorHandleOffset));

	//pGCL->SetGraphicsRoot32BitConstant(2, 1, 0);
	for (auto it = mObjects.begin(); it != mObjects.end(); ++it)
	{
		UINT handleOffset = it - mObjects.begin() + mCurrentBackBufferIndex * mObjects.size();
		auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetGPUDescriptorHandleForHeapStart(), handleOffset, mSRVDesricptorHandleOffset);
		mFrameResources->mModelCB->SetResources(DirectX::XMMatrixTranspose((*it)->GetModelMatrix()), handleOffset);
		mGCL->SetGraphicsRootDescriptorTable(1, handle);
		(*it)->SetVertexIndexBufferView(mGCL);
		(*it)->DrawIndexed(mGCL);
	}

	mGCL->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->GetResource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

}

void RenderSys::CreateConstantBuffers()
{
	float aspectRatio = (float)mWidth / mHeight;
	mZNear = 0.1f;
	mZFar = 100;
	mProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspectRatio, mZNear, mZFar);
	
	CameraCBData cameraData;
	cameraData.mVPMx[0] = DirectX::XMMatrixTranspose(mCamera->GetViewMatrix() * mProjection);
	cameraData.mVPMx[1] = DirectX::XMMatrixInverse(nullptr, cameraData.mVPMx[0]);
	float lightPosStep = 1;

	for (UINT iLight = 0; iLight < 10; iLight += 2)
	{
		mLights.mLights[iLight].color_ambient = { 0.1, 0.1, 0.1, 0.1 };
		mLights.mLights[iLight].lightPos = { 1 + lightPosStep * iLight, 1, 1 + lightPosStep * iLight };
		mLights.mLights[iLight].light_color = { 1, 1, 1, 1 };
		mLights.mLights[iLight].range = 5.;
		DirectX::XMMATRIX light_view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mLights.mLights[iLight].lightPos), DirectX::XMVectorSet(0, 0, 0, 0),
			DirectX::XMVectorSet(0, 1, 0, 0));
		cameraData.mVPMx[iLight + 2] = DirectX::XMMatrixTranspose(light_view * mProjection);

		mLights.mLights[iLight+1].color_ambient = { 0.1, 0.1, 0.1, 0.1 };
		mLights.mLights[iLight+1].lightPos = { 1 + lightPosStep * iLight, 1, -1 + lightPosStep * iLight };
		mLights.mLights[iLight+1].light_color = { 1, 1, 1, 1 };
		mLights.mLights[iLight+1].range = 5.;
		light_view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mLights.mLights[iLight+1].lightPos), DirectX::XMVectorSet(0, 0, 0, 0),
			DirectX::XMVectorSet(0, 1, 0, 0));
		cameraData.mVPMx[iLight + 3] = DirectX::XMMatrixTranspose(light_view * mProjection);
		mFrameData.mLL[iLight] = mLights.mLights[iLight];
		mFrameData.mLL[iLight+1] = mLights.mLights[iLight+1];
	}
	mFrameData.mCameraPos = mCamera->GetEyePosition();
	mFrameData.mDispatchSize = DirectX::XMUINT3(mClusterCountX, mClusterCountY, mClusterCountZ);
	mFrameData.mNearFar = { mZNear, mZFar };
	mFrameData.mNumSlices = mClusterCountZ;
	mFrameData.mTileSize = { static_cast<float>(mWidth) / mClusterCountX / mWidth, static_cast<float>(mHeight) / mClusterCountY / mHeight };
	mFrameResources->mFrameCB->SetResources(mFrameData, 0);
	mFrameResources->mFrameCB->SetResources(mFrameData, 1);

	mFrameResources->mCameraCB->SetResources(cameraData, 0);
	mFrameResources->mCameraCB->SetResources(cameraData, 1);

}

void RenderSys::CreateDepthStencil()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mDSDescHeap)));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, mWidth, mHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&mDepthStencilBuffer)
	);

	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &depthStencilDesc, mDSDescHeap->GetCPUDescriptorHandleForHeapStart());
}

DirectX::XMVECTOR calcTriangleNormal(const Vertex& _vt1, const Vertex& _vt2, const Vertex& _vt3)
{
	DirectX::XMVECTOR u, v, pt;
	pt = XMLoadFloat3(&_vt1.position);
	u = XMLoadFloat3(&_vt2.position);
	u = DirectX::XMVectorSubtract(u, pt);
	v = XMLoadFloat3(&_vt3.position);
	v = DirectX::XMVectorSubtract(v, pt);
	return DirectX::XMVector3Normalize(DirectX::XMVector3Cross(u, v));
}

DirectX::XMVECTOR calcWeightedTriangleNormal(const Vertex& _vt1, const Vertex& _vt2, const Vertex& _vt3)
{
	DirectX::XMVECTOR u, v, pt;
	pt = XMLoadFloat3(&_vt1.position);
	u = XMLoadFloat3(&_vt2.position);
	u = DirectX::XMVectorSubtract(u, pt);
	v = XMLoadFloat3(&_vt3.position);
	v = DirectX::XMVectorSubtract(v, pt);
	return DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVector3Cross(u, v)), 
		fabs(DirectX::XMVector3AngleBetweenVectors(u, v).m128_f32[0] / DirectX::XM_PIDIV2));
}

void calcWeightedNormals(Vertex* _vertices, UINT _vtxCount, const UINT* _indicies, UINT _idxCount)
{
	for (UINT iVtx = 0; iVtx < _vtxCount; ++iVtx)
	{
		_vertices[iVtx].normal = { 0,0,0 };
	}
	if (_idxCount % 3 != 0) return;

	UINT* linkCounter = new UINT[_vtxCount];
	memset(linkCounter, 0, sizeof(UINT) * _vtxCount);

	Vertex* vx1, * vx2, * vx3;
	DirectX::XMVECTOR vNormal, vBuf;
	for (UINT iIdx = 0; iIdx < _idxCount; iIdx += 3)
	{
		vx1 = &_vertices[_indicies[iIdx]];
		vx2 = &_vertices[_indicies[iIdx + 1]];
		vx3 = &_vertices[_indicies[iIdx + 2]];
		vNormal = calcWeightedTriangleNormal(*vx1, *vx2, *vx3);
		vBuf = XMLoadFloat3(&vx1->normal);
		XMStoreFloat3(&vx1->normal, DirectX::XMVectorAdd(vNormal, vBuf));
		vBuf = XMLoadFloat3(&vx2->normal);
		XMStoreFloat3(&vx2->normal, DirectX::XMVectorAdd(vNormal, vBuf));
		vBuf = XMLoadFloat3(&vx3->normal);
		XMStoreFloat3(&vx3->normal, DirectX::XMVectorAdd(vNormal, vBuf));

		++linkCounter[_indicies[iIdx]];
		++linkCounter[_indicies[iIdx + 1]];
		++linkCounter[_indicies[iIdx + 2]];
	}
	for (UINT iVtx = 0; iVtx < _vtxCount; ++iVtx)
	{
		vNormal = XMLoadFloat3(&_vertices[iVtx].normal);
		vBuf = DirectX::XMVectorSet(linkCounter[iVtx], linkCounter[iVtx], linkCounter[iVtx], 1);
		XMStoreFloat3(&_vertices[iVtx].normal, DirectX::XMVector3Normalize(DirectX::XMVectorDivide(vNormal, vBuf)));
	}
	delete[] linkCounter;
}


void RenderSys::CreateClusterEntity()
{

	Vertex cube[8];
	cube[0].position = { 0.5, 0.5, 0.5 };
	cube[1].position = { 0.5, 0.5, -0.5 };
	cube[2].position = { -0.5, 0.5, -0.5 };
	cube[3].position = { -0.5, 0.5, 0.5 };

	cube[4].position = { 0.5,  -0.5, 0.5 };
	cube[5].position = { 0.5,  -0.5, -0.5 };
	cube[6].position = { -0.5, -0.5, -0.5 };
	cube[7].position = { -0.5, -0.5, 0.5 };
	UINT indices[32] = {
		0, 4, 1, 5, 2, 6, 3, 7,
		2, 3, 0, 3, 1, 0, 1, 2,
		4, 7, 5, 4, 6, 5, 6, 7, 
		6, 7, 7, 4, 5, 4, 6, 5
	};
	mClusterEntity = std::unique_ptr<Entity>(CreateEntity(cube, 8, indices, 32));
}

void RenderSys::InitilizeComputeShaderResources()
{
	CreateClusterEntity();
	{
		mUAVDescHeapGPU = this->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 10, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		mUAVDescHeapCPU = this->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 10, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		mGenData.tileSize = { static_cast<float>(mWidth) / mClusterCountX, static_cast<float>(mHeight) / mClusterCountY};
		mGenData.inverseProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, mProjection));
		mGenData.screenSize = DirectX::XMUINT2(mWidth, mHeight);
		mGenData.zNear = mZNear;
		mGenData.zFar = mZFar;
		mGenData.dispatchSize = DirectX::XMUINT3(mClusterCountX, mClusterCountY, mClusterCountZ);
		mGenData.numSlices = mClusterCountZ;
		mGenData.viewMx = DirectX::XMMatrixIdentity();

		mComputeCA = CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		mComputeCQ = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);

		ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE,
			mComputeCA.Get(), mPSOs["clusterGen"].Get(), IID_PPV_ARGS(&mComputeGCL)), "Cann't create command list!\n");
		ThrowIfFailed(mComputeGCL->Close(), "Cannt close compute command List!\n");
		ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mComputeFence)), "Cann't create compute fence!\n");
	}
	//Initilizing UAVs
	{
		auto descPtrGPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mUAVDescHeapGPU->GetCPUDescriptorHandleForHeapStart());
		auto descPtrCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mUAVDescHeapCPU->GetCPUDescriptorHandleForHeapStart());
		
		//ClusterGen CB
		mClusterCSData = std::make_unique<ConstantBuffer<ClusterGenData>>();
		mClusterCSData->Initialize(mDevice, descPtrGPU, 1);
		mClusterCSData->SetResources(mGenData, 0);
		descPtrGPU.Offset(1, mSRVDesricptorHandleOffset);
		
		//ClusterLight
		mClusterCSLights = std::make_unique<ConstantBuffer<FrameLights>>();
		mClusterCSLights->Initialize(mDevice, descPtrGPU, 1);
		mClusterCSLights->SetResources(mLights, 0);
		descPtrGPU.Offset(1, mSRVDesricptorHandleOffset);
		descPtrCPU.Offset(2, mSRVDesricptorHandleOffset);
		//MinMaxPoints Texture
		D3D12_RESOURCE_DESC uavDesc = CD3DX12_RESOURCE_DESC::Tex1D(DXGI_FORMAT_R32G32B32A32_FLOAT,
			mClusterCountX * mClusterCountY * mClusterCountZ * 2, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavViewDesc = {};
		uavViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		uavViewDesc.Texture1D.MipSlice = 0;
		uavViewDesc.Texture2D.MipSlice = 0;
		uavViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;

		mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&mOutMinMaxPointClusters));
		mDevice->CreateUnorderedAccessView(mOutMinMaxPointClusters.Get(), nullptr, &uavViewDesc, descPtrGPU);
		mDevice->CreateUnorderedAccessView(mOutMinMaxPointClusters.Get(), nullptr, &uavViewDesc, descPtrCPU);
		descPtrCPU.Offset(1, mSRVDesricptorHandleOffset);
		descPtrGPU.Offset(1, mSRVDesricptorHandleOffset);
		
		//ActiveClusters
		uavViewDesc.Format = DXGI_FORMAT_R32_UINT;
		uavDesc.Format = DXGI_FORMAT_R32_UINT;
		uavDesc.Width = mClusterCountX * mClusterCountY * mClusterCountZ;
		mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&mActiveClusters));
		mDevice->CreateUnorderedAccessView(mActiveClusters.Get(), nullptr, &uavViewDesc, descPtrGPU);
		mDevice->CreateUnorderedAccessView(mActiveClusters.Get(), nullptr, &uavViewDesc, descPtrCPU);
		descPtrCPU.Offset(1, mSRVDesricptorHandleOffset);
		descPtrGPU.Offset(1, mSRVDesricptorHandleOffset);
		
		//Compact ActiveClusters
		uavDesc.Width = mClusterCountX * mClusterCountY * mClusterCountZ;
		mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&mCompactClusters));
		mDevice->CreateUnorderedAccessView(mCompactClusters.Get(), nullptr, &uavViewDesc, descPtrGPU);
		mDevice->CreateUnorderedAccessView(mCompactClusters.Get(), nullptr, &uavViewDesc, descPtrCPU);
		descPtrCPU.Offset(1, mSRVDesricptorHandleOffset);
		descPtrGPU.Offset(1, mSRVDesricptorHandleOffset);

		//RWCounter
		auto rwUavDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT) * 3, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS); // Width = 1(elements_count) * 4 (sizeof(DXGI_FORMAT_R32_UINT))
		rwUavDesc.Format = DXGI_FORMAT_UNKNOWN;
		mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, 
			&rwUavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&mRWActiveClusters));
		D3D12_UNORDERED_ACCESS_VIEW_DESC rwViewDesc = {};
		rwViewDesc.Format = DXGI_FORMAT_R32_UINT;
		rwViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		rwViewDesc.Buffer.FirstElement = 0;
		rwViewDesc.Buffer.NumElements = 3;
		rwViewDesc.Buffer.StructureByteStride = 0;
        rwViewDesc.Buffer.CounterOffsetInBytes = 0;
		rwViewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		mDevice->CreateUnorderedAccessView(mRWActiveClusters.Get(), nullptr, &rwViewDesc, descPtrGPU);
		descPtrGPU.Offset(1, mSRVDesricptorHandleOffset);
		mDevice->CreateUnorderedAccessView(mRWActiveClusters.Get(), nullptr, &rwViewDesc, descPtrCPU);
		descPtrCPU.Offset(1, mSRVDesricptorHandleOffset);

		//ClusterNumLightPairs
		auto uav2dDesc = static_cast<D3D12_RESOURCE_DESC>(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_UINT, 4096, 4096,
			1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS));
		uavDesc.Width = 4096;
		uavDesc.Height = 4096;
		uavDesc.Format = DXGI_FORMAT_R32_UINT;
		uavViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavViewDesc.Buffer.CounterOffsetInBytes = 0;
		mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &uav2dDesc, 
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&mClusterNumLightPairs));
		mDevice->CreateUnorderedAccessView(mClusterNumLightPairs.Get(), nullptr, &uavViewDesc, descPtrGPU);
		descPtrGPU.Offset(1, mSRVDesricptorHandleOffset);
		mDevice->CreateUnorderedAccessView(mClusterNumLightPairs.Get(), nullptr, &uavViewDesc, descPtrCPU);
		descPtrCPU.Offset(1, mSRVDesricptorHandleOffset);
		
		//ClusterLights List
		mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &uav2dDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&mClustersLightList));
		mDevice->CreateUnorderedAccessView(mClustersLightList.Get(), nullptr, &uavViewDesc, descPtrGPU);
		descPtrGPU.Offset(1, mSRVDesricptorHandleOffset);
		mDevice->CreateUnorderedAccessView(mClustersLightList.Get(), nullptr, &uavViewDesc, descPtrCPU);
		descPtrCPU.Offset(1, mSRVDesricptorHandleOffset);

		auto zTextureDesc = D3D12_SHADER_RESOURCE_VIEW_DESC();
		zTextureDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		zTextureDesc.Format = DXGI_FORMAT_R32_FLOAT;
		zTextureDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		zTextureDesc.Texture2D.MostDetailedMip = 0;
		zTextureDesc.Texture2D.MipLevels = 1;
		zTextureDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		zTextureDesc.Texture2D.PlaneSlice = 0;
		mDevice->CreateShaderResourceView(mDepthStencilBuffer.Get(), &zTextureDesc, descPtrGPU);
		mDevice->CreateShaderResourceView(mDepthStencilBuffer.Get(), &zTextureDesc, descPtrCPU);

	}
	{
		auto hClusterNumLightPairs = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCBDescHeap->GetCPUDescriptorHandleForHeapStart(), 14, mSRVDesricptorHandleOffset);
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavViewDesc = {};
		uavViewDesc.Texture2D.MipSlice = 0;
		uavViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavViewDesc.Buffer.CounterOffsetInBytes = 0;
		uavViewDesc.Format = DXGI_FORMAT_R32_UINT;
		mDevice->CreateUnorderedAccessView(mClusterNumLightPairs.Get(), nullptr, &uavViewDesc, hClusterNumLightPairs);

		auto hClusterLightList = hClusterNumLightPairs;
		hClusterLightList.Offset(1, mSRVDesricptorHandleOffset);
		mDevice->CreateUnorderedAccessView(mClustersLightList.Get(), nullptr, &uavViewDesc, hClusterLightList);
	}
	ComputeClusters();
}