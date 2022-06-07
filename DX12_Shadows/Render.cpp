#include "Render.h"

void RenderSys::Initialize(HWND& hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	width = rc.right - rc.left; height = rc.bottom - rc.top;
	mCamera = std::make_unique<Camera>(DirectX::XMFLOAT3( 1.5, 1.5 , 0 ), DirectX::XMFLOAT3(0, 0, 0));
	bVSync = true;
	bTearingSupported = CheckTearingSupport();

#ifdef _DEBUG
	D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugInterface));
	pDebugInterface->EnableDebugLayer();
#endif

	auto adapter = GetAdapter();
	CreateDevice(adapter);
	pCQ = CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	uRTVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mSRVDesricptorHandleOffset = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateDepthStencil();
	CreateRootSignatue();
	CreatePSOs();

	CreateSwapChain(hWnd, width, height, g_numFrames);
	pRTVDescHeap.Swap(CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, g_numFrames));

	UpdateRenderTargetViews();
	for (int i = 0; i < g_numFrames; ++i)
	{
		pCommandAllocator[i] = CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
	pGCL = CreateCommandList(pCommandAllocator[uCurrentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

	pFence = CreateFence();
	hFenceEvent = CreateEventHandle();

	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

	pCBDescHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 10, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	CreateFrameResources();
	CreateConstantBuffers();
	mShadowMap = std::make_unique<ShadowMap>(pDevice, width, height);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(pCBDescHeap->GetCPUDescriptorHandleForHeapStart());
	hSRV.Offset(6, pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDSV = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDSDescHeap->GetCPUDescriptorHandleForHeapStart());
	hDSV.Offset(1, pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU_SRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(pCBDescHeap->GetGPUDescriptorHandleForHeapStart());
	hGPU_SRV.Offset(6, pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	mShadowMap->SetDescriptors(hSRV, hGPU_SRV, hDSV);
}

void RenderSys::Render()
{
	Update();
	auto commandAllocator = pCommandAllocator[uCurrentBackBufferIndex];
	auto backBuffer = pBackBuffer[uCurrentBackBufferIndex];

	commandAllocator->Reset();
	pGCL->Reset(commandAllocator.Get(), mPSOs["shadows"].Get());
	//Set state
	{

		ID3D12DescriptorHeap* ppHeaps[] = { pCBDescHeap.Get() };
		pGCL->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	
		DrawSceneToShdowMap();
		pGCL->SetPipelineState(mPSOs["shadows"].Get());
		pGCL->SetGraphicsRootSignature(mRSs["shadows"].Get());
		pGCL->SetGraphicsRootDescriptorTable(2, mShadowMap->Srv());

		pGCL->RSSetViewports(1, &m_viewport); 
		pGCL->RSSetScissorRects(1, &m_scissorRect);
	}

	// Draw
	{
		CD3DX12_RESOURCE_BARRIER  barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		//mFrameResources->mModelCB->SetViewMatrix(mCamera->GetViewMatrix());
		pGCL->ResourceBarrier(1, &barrier);
		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(pRTVDescHeap->GetCPUDescriptorHandleForHeapStart(),
			uCurrentBackBufferIndex, uRTVDescriptorSize);

		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mDSDescHeap->GetCPUDescriptorHandleForHeapStart()); 
		pGCL->OMSetRenderTargets(1, &rtv, FALSE, &dsvHandle);
		pGCL->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		pGCL->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		pGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto CBHeadHandle = pCBDescHeap->GetGPUDescriptorHandleForHeapStart();
		mFrameResources->mFrameCB->SetCameraPosition(mCamera->GetEyePosition(), uCurrentBackBufferIndex);
		auto frameHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CBHeadHandle, 4, mSRVDesricptorHandleOffset);
		pGCL->SetGraphicsRootDescriptorTable(1, mFrameResources->mFrameCB->GetGPUDescriptorHandle(frameHandle, 1, uCurrentBackBufferIndex, mSRVDesricptorHandleOffset));

		for (auto it = vObjects.begin(); it != vObjects.end(); ++it)
		{
			auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(pCBDescHeap->GetGPUDescriptorHandleForHeapStart());
			UINT handleOffset = uCurrentBackBufferIndex * 2 + (it - vObjects.begin());
			handle.Offset(handleOffset, mSRVDesricptorHandleOffset);
			mFrameResources->mModelCB->SetResources((*it)->GetModelMatrix(), mCamera->GetViewMatrix(), mProjection, handleOffset);
			pGCL->SetGraphicsRootDescriptorTable(0, handle);
			(*it)->SetVertexIndexBufferView(pGCL);
			(*it)->DrawIndexed(pGCL);
		}
	}
	// Present
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		pGCL->ResourceBarrier(1, &barrier);
		ThrowIfFailed(pGCL->Close());

		ID3D12CommandList* const commandLists[] = {
			pGCL.Get()
		};
		pCQ->ExecuteCommandLists(_countof(commandLists), commandLists);
		UINT syncInterval = bVSync ? 1 : 0;
		UINT presentFlags = bTearingSupported && !bVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		ThrowIfFailed(pSwapChain->Present(syncInterval, presentFlags));

		uFrameFenceValues[uCurrentBackBufferIndex] = Signal(pCQ, pFence, uFenceValue);
		uCurrentBackBufferIndex = pSwapChain->GetCurrentBackBufferIndex();

		WaitForFenceValue(pFence, uFrameFenceValues[uCurrentBackBufferIndex], hFenceEvent);
	}
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
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice)), "Can't create device!");
#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(pDevice.As(&pInfoQueue)))
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

	ThrowIfFailed(pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)), "Cann't create command queue!\n");

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
	ThrowIfFailed(dxgiFactory4->CreateSwapChainForHwnd(pCQ.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1),
		"Cann't createa SwapChain!\n");


	ThrowIfFailed(swapChain1.As(&pSwapChain));
}

ComPtr<ID3D12DescriptorHeap> RenderSys::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flag)
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	desc.Flags = _flag;

	ThrowIfFailed(pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
}

ComPtr<ID3D12CommandAllocator> RenderSys::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ThrowIfFailed(pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)), "Cannt create command allocator!\n");
	return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> RenderSys::CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ThrowIfFailed(pDevice->CreateCommandList(0, type, commandAllocator.Get(), mPSOs["shadows"].Get(), IID_PPV_ARGS(&commandList)),
		"Cannt create command List!\n");
	ThrowIfFailed(commandList->Close(), "Cannt close command List!\n");
	return commandList;
}

ComPtr<ID3D12Fence> RenderSys::CreateFence()
{
	ComPtr<ID3D12Fence> fence;
	ThrowIfFailed(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
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
	UINT rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(pRTVDescHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < g_numFrames; ++i)
	{
		ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		pDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

		pBackBuffer[i] = backBuffer;
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
	uint64_t fenceValueForSignal = Signal(pCQ, pFence, uFenceValue);
	WaitForFenceValue(pFence, fenceValueForSignal, hFenceEvent);
}

void RenderSys::CreateRootSignatue()
{
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
		CD3DX12_ROOT_PARAMETER1 rootParameters[3];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
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
		ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRSs["shadows"])));
	}
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
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
		ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRSs["zTest"])));
	}
}

void RenderSys::CreatePSOs()
{

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
		ThrowIfFailed(D3DCompileFromFile(L"Shaders/ShadowsVS.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr), "Cann't compile VS!\n");
		ThrowIfFailed(D3DCompileFromFile(L"Shaders/ShadowsPS.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr), "Cann't compile PS!\n");

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
		ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["shadows"])), "Cannot create PSO!");
	}
	{
		ComPtr<ID3DBlob> zTestShader;
		ThrowIfFailed(D3DCompileFromFile(L"Shaders/ZTest.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &zTestShader, nullptr), "Cann't compile ZTest shader!\n");

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
		ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&psoZDesc, IID_PPV_ARGS(&mPSOs["zTest"])), "Cannot create PSO!");
	}
}

RenderSys::~RenderSys()
{
	for (auto it = vObjects.begin(); it != vObjects.end(); ++it)
	{
		delete (*it);
		(*it) = nullptr;
	}
	vObjects.clear();
	CloseHandle(hFenceEvent);
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
		ThrowIfFailed(pDevice->CreateCommittedResource(&heapProperties,
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
		ThrowIfFailed(pDevice->CreateCommittedResource(&heapProperties,
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
	vObjects.push_back(CreateEntity(_pVtx, _uVertexCount, _pIndices, _uIndicesCount));
}

void RenderSys::CreateFrameResources()
{
	UINT handleOffset = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(pCBDescHeap->GetCPUDescriptorHandleForHeapStart());
	UINT objectCount = 2;

	mFrameResources = std::make_unique<FrameResource>(pDevice.Get(), g_numFrames, objectCount, handle);
}

RenderSys::RenderSys()
{
	width = 0;
	height = 0;
}

void RenderSys::DrawSceneToShdowMap()
{
	pGCL->RSSetViewports(1, &mShadowMap->GetViewport());
	pGCL->RSSetScissorRects(1, &mShadowMap->GetScissorRect());
	pGCL->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->GetResource(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));

	pGCL->SetPipelineState(mPSOs["zTest"].Get());
	pGCL->SetGraphicsRootSignature(mRSs["zTest"].Get());

	pGCL->ClearDepthStencilView(mShadowMap->Dsv(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	pGCL->OMSetRenderTargets(0, nullptr, false, &mShadowMap->Dsv());

	pGCL->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&mLights[0].lightPos);
	DirectX::XMVECTOR lookAt = DirectX::XMVectorSet(0, 0, 0, 0);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
	DirectX::XMMATRIX viewMx = DirectX::XMMatrixLookAtLH(eyePos, lookAt, up);

	//UINT reverseIndex = uCurrentBackBufferIndex ? 0 : 1;
	for (auto it = vObjects.begin(); it != vObjects.end(); ++it)
	{
		UINT handleOffset = it - vObjects.begin() + uCurrentBackBufferIndex * vObjects.size();
		auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(pCBDescHeap->GetGPUDescriptorHandleForHeapStart(), handleOffset, mSRVDesricptorHandleOffset);
		pGCL->SetGraphicsRootDescriptorTable(0, handle);
		mFrameResources->mModelCB->SetResources((*it)->GetModelMatrix(), viewMx, mShadowMap->GetProjection(), handleOffset);
		(*it)->SetVertexIndexBufferView(pGCL);
		(*it)->DrawIndexed(pGCL);
	}

	pGCL->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->GetResource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

}

///@TODO Вынести матрицу проекции в класс
void RenderSys::CreateConstantBuffers()
{
	//ModelCB
	float aspectRatio = (float)width / height;
	mProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspectRatio, 0.1f, 100.f);
	//mFrameResources->mModelCB->SetResources(DirectX::XMMatrixIdentity(), mCamera->GetViewMatrix(), projection);
	//FrameCB	
	mLights[0].color_ambient = {0.1, 0.1, 0.1, 0.1};
	mLights[0].lightPos = { 1, 1, 0 };
	mLights[0].light_color = {1, 1, 1, 1};
	mLights[0].range = 3.;
	mLights[0].light_view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&mLights[0].lightPos), DirectX::XMVectorSet(0, 0, 0, 0),
		DirectX::XMVectorSet(0, 1, 0, 0));
	mLights[0].light_view = DirectX::XMMatrixTranspose(mLights[0].light_view);
	mLights[0].light_projection = DirectX::XMMatrixTranspose(mProjection);
	mFrameResources->mFrameCB->SetResources(mLights[0], mCamera->GetEyePosition(), 0);
	mFrameResources->mFrameCB->SetResources(mLights[0], mCamera->GetEyePosition(), 1);
}

void RenderSys::CreateDepthStencil()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mDSDescHeap)));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencilBuffer)
	);

	pDevice->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilDesc, mDSDescHeap->GetCPUDescriptorHandleForHeapStart());
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