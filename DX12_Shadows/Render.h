#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <chrono>
#include <d3dcompiler.h>
#include <unordered_map>

#include "Helpers.h"
#include "Entity.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "ShadowMap.h"
const UINT g_numFrames = 3;
using Microsoft::WRL::ComPtr;


#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif


void calcWeightedNormals(Vertex* _vertices, UINT _vtxCount, const UINT* _indicies, UINT _idxCount);

class RenderSys final
{
	UINT width;
	UINT height;
	//Piplene objects
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<ID3D12Device8> pDevice;
	
	ComPtr<ID3D12CommandQueue> pCQ;
	ComPtr<ID3D12GraphicsCommandList> pGCL;
	ComPtr<ID3D12CommandAllocator> pCommandAllocator[g_numFrames];
	
	ComPtr<IDXGISwapChain4> pSwapChain;
	ComPtr<ID3D12Resource> pBackBuffer[g_numFrames];
	
	std::unordered_map<std::string, ComPtr<ID3D12RootSignature>> mRSs;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	ComPtr<ID3D12DescriptorHeap> pCBDescHeap;
	ComPtr<ID3D12DescriptorHeap> pRTVDescHeap;
	ComPtr<ID3D12DescriptorHeap> mDSDescHeap;
	UINT uRTVDescriptorSize;
	UINT uCurrentBackBufferIndex;

	ComPtr<ID3D12Resource> m_depthStencilBuffer;

	//Synchronization objects
	ComPtr<ID3D12Fence> pFence;
	uint64_t uFenceValue = 0;
	uint64_t uFrameFenceValues[g_numFrames];
	HANDLE hFenceEvent;
#ifdef _DEBUG
	ComPtr<ID3D12Debug> pDebugInterface;
#endif
	std::unique_ptr<ShadowMap> mShadowMap;
	ModelCB m_modelCB;
	FrameCB m_frameCB;

	bool bVSync;
	bool bTearingSupported;
	UINT m_desricptorHandleOffset;

	std::vector<Entity*> vObjects;
	std::unique_ptr<Camera> mCamera;


	ComPtr<IDXGIAdapter3> GetAdapter();
	void CreateDevice(ComPtr<IDXGIAdapter3> adapter);
	void CreateSwapChain(HWND hWnd, uint32_t width, uint32_t height, uint32_t bufferCount);
	ComPtr<ID3D12CommandQueue> CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12Fence> CreateFence();
	HANDLE CreateEventHandle();
	bool CheckTearingSupport();
	void UpdateRenderTargetViews();
	void CreateRootSignatue();
	void CreatePSOs();
	void CreateConstantBuffers();
	void CreateDepthStencil();

	uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue);
	void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, 
		std::chrono::milliseconds duration = std::chrono::milliseconds::max());

	Entity* CreateEntity(Vertex* _pVtx, UINT _uVertexCount, UINT* _pIndecies, UINT _uIndexCount);
	void Update();
	void DrawSceneToShdowMap();

	Light mLights[1];
public:
	RenderSys();
	~RenderSys();
	void Initialize(HWND& hWnd);
	void Render();
	void Flush();
	void DrawObject(Vertex* _pVtx, const UINT& _uVertexCount, UINT* _pIndices, const UINT& _uIndicesCount);
};