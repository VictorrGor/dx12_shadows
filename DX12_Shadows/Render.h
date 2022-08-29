#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <chrono>
#include <d3dcompiler.h>
#include <unordered_map>
#include <vector>

#include "Helpers.h"
#include "Entity.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "ShadowMap.h"
#include "FrameResource.h"
#include "ClusterResources.h"
const UINT g_numFrames = 2;
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
	UINT mWidth;
	UINT mHeight;
	//Piplene objects
	CD3DX12_VIEWPORT mViewport;
	CD3DX12_RECT mScissorRect;
	ComPtr<ID3D12Device8> mDevice;
	
	ComPtr<ID3D12CommandQueue> mCQ;
	ComPtr<ID3D12GraphicsCommandList> mGCL;
	ComPtr<ID3D12CommandAllocator> mCommandAllocator[g_numFrames];
	
	ComPtr<IDXGISwapChain4> mSwapChain;
	ComPtr<ID3D12Resource> mBackBuffer[g_numFrames];
	
	std::unordered_map<std::string, ComPtr<ID3D12RootSignature>> mRSs;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	//Compute
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mUAVDescHeapGPU;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mUAVDescHeapCPU;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mComputeCA;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mComputeGCL;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mComputeCQ;
	Microsoft::WRL::ComPtr<ID3D12Fence> mComputeFence;
	const UINT mClusterCountX = 16;
	const UINT mClusterCountY = 9;
	const UINT mClusterCountZ = 10;
	std::unique_ptr<Entity> mClusterEntity;

	uint64_t mComputeFenceValue = 0;
	HANDLE mComputeFenceEvent;

	Microsoft::WRL::ComPtr<ID3D12Resource> mOutMinMaxPointClusters;
	//Microsoft::WRL::ComPtr<ID3D12Resource> mOutMaxPointClusters;
	Microsoft::WRL::ComPtr<ID3D12Resource> mAllClustersPoint;

	std::unique_ptr<ConstantBuffer<ClusterGenData>> mClusterGenBuf;
	std::unique_ptr<ConstantBuffer<DirectX::XMMATRIX>> mClusterDrawBuf;

	ClusterGenData mGenData;
	//
	ComPtr<ID3D12DescriptorHeap> mCBDescHeap;
	ComPtr<ID3D12DescriptorHeap> mRTVDescHeap;
	ComPtr<ID3D12DescriptorHeap> mDSDescHeap;
	UINT mRTVDescriptorSize;
	UINT mCurrentBackBufferIndex;

	ComPtr<ID3D12Resource> mDepthStencilBuffer;

	//Synchronization objects
	ComPtr<ID3D12Fence> mFence;
	uint64_t mFenceValue = 0;
	uint64_t mFrameFenceValues[g_numFrames];
	HANDLE mFenceEvent;
#ifdef _DEBUG
	ComPtr<ID3D12Debug> mDebugInterface;
#endif
	std::unique_ptr<ShadowMap> mShadowMap;
	std::unique_ptr<FrameResource> mFrameResources;
	DirectX::XMMATRIX mProjection;
	float mZNear = 0;
	float mZFar = 0;

	bool mVSync;
	bool mTearingSupported;
	UINT mSRVDesricptorHandleOffset;

	std::vector<Entity*> mObjects;
	std::unique_ptr<Camera> mCamera;

	//G-Buffer
	const UINT mGTextureCount = 2;
	std::vector<ComPtr<ID3D12Resource>> mGTexture;//0 : normal.x + normal.y + diffuseColor u(256 << 256 << 256) + alfa
												  //1 : position.x + position.y + position.z + alfa
	//

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
	void CreateFrameResources(); 
	void CreateForwardCBDesriptorHeap();
	void CreateDeferredCBDescriptionHead();
	void CreateClusterShader();
	void InitilizeComputeShaderResources();
	void ComputeClusters();
	void CreateClusterEntity();

	void CreateLights();

	uint64_t Signal(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue);
	void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, 
		std::chrono::milliseconds duration = std::chrono::milliseconds::max());

	Entity* CreateEntity(Vertex* _pVtx, UINT _uVertexCount, UINT* _pIndecies, UINT _uIndexCount);
	void Update();
	void DrawSceneToShdowMap();
	void GPass();
	void DeferredShading();

	Light mLights[10];
	CameraCBData mCameraCB;
	std::unique_ptr<Entity> mDefferedPassScreen;
public:
	RenderSys();
	~RenderSys();
	void Initialize(HWND& hWnd);
	void Render();
	void Flush();
	void DrawObject(Vertex* _pVtx, const UINT& _uVertexCount, UINT* _pIndices, const UINT& _uIndicesCount);

	void MoveForward();
	void MoveBackward();
	void MoveLeft();
	void MoveRight();
	void RotateCamera(int _deltaX, int _deltaY);
};