#pragma once

#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <memory>
#include "ConstantBuffer.h"
#include "Helpers.h"


struct ClusterGenData
{
	DirectX::XMFLOAT2 tileSize;
	DirectX::XMMATRIX inverseProjection;
	DirectX::XMUINT2 screenSize;
	float zNear;
	float zFar;
	DirectX::XMUINT3 dispatchSize;
	UINT numSlices;

	DirectX::XMMATRIX viewMx;
};
//class ClustersResources
//{
//
//
//
//
//	void CreatePSO(ID3D12Device8* _device);
//	void CreateResources(ID3D12Device8* _device);
//	void CreateUAVDescriptorHeap(ID3D12Device8* _device);
//public:
//	ClustersResources(ID3D12Device8* _device, float _zNear, float _zFar, 
//			DirectX::XMMATRIX& _projection, DirectX::XMUINT2& _screenSize);
//	void ComputeClusters(ID3D12Device8* _device);
//};