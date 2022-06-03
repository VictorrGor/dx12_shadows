#pragma once
#include <d3dx12.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "Helpers.h"

class ShadowMap
{
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	float mNear;
	float mFar;
	UINT mWidth;
	UINT mHeight;
	Microsoft::WRL::ComPtr<ID3D12Resource> mShadowMap;
	Microsoft::WRL::ComPtr<ID3D12Device8> mDevice;
	DXGI_FORMAT mFormat;
	DirectX::XMMATRIX mProjection;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuDsv;
	UINT8* m_pCbvDataBegin;
	void CreateResource()
	{
		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = mWidth;
		texDesc.Height = mHeight;
		texDesc.DepthOrArraySize = 1;
		texDesc.Format = mFormat;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		ThrowIfFailed(mDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&optClear,
			IID_PPV_ARGS(&mShadowMap)));
	}
	void CreateDescriptors()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2D.PlaneSlice = 0;
		mDevice->CreateShaderResourceView(mShadowMap.Get(), &srvDesc, mCpuSrv);


		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		mDevice->CreateDepthStencilView(mShadowMap.Get(), &dsvDesc, mCpuDsv);
	}
public:
	ShadowMap(Microsoft::WRL::ComPtr<ID3D12Device8> _device, UINT _width, UINT _height)
	{
		mDevice = _device;
		mWidth = _width;
		mHeight = _height;
		mFormat = DXGI_FORMAT_R24G8_TYPELESS;
		mNear = 0.1;
		mFar = 10;
		mViewport = { 0.0f, 0.0f, (float)mWidth, (float)mHeight, 0.0f, 1.0f };
		mScissorRect = { 0, 0, (int)mWidth, (int)mHeight };
		mProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, static_cast<float>(mWidth) / mHeight, mNear, mFar);
		CreateResource();
	};
	void SetDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE _cpuSrv,
		CD3DX12_GPU_DESCRIPTOR_HANDLE _gpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE _cpuDsv)
	{
		mCpuSrv = _cpuSrv;
		mGpuSrv = _gpuSrv;
		mCpuDsv = _cpuDsv;

		CreateDescriptors();
	}
	D3D12_VIEWPORT GetViewport() const
	{
		return mViewport;
	};
	D3D12_RECT GetScissorRect() const
	{
		return mScissorRect;
	}
	ID3D12Resource* GetResource()
	{
		return mShadowMap.Get();
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv() const
	{
		return mGpuSrv;
	}
	const DirectX::XMMATRIX& GetProjection() const
	{
		return mProjection;
	};
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv() const
	{
		return mCpuDsv;
	}

};