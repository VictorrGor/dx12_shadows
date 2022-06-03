#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "Vertex.h"
#include <utility>


class Entity final
{
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	UINT uVtxCount;
	Microsoft::WRL::ComPtr<ID3D12Resource> pIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
	UINT uIndexCount;
	DirectX::XMMATRIX mModel;
public:
	Entity();
	void Initialize(Vertex* _pVtx, UINT _uVtxCount,
		Microsoft::WRL::ComPtr<ID3D12Resource> _pVertexBuffer, D3D12_VERTEX_BUFFER_VIEW& _pVBView,
		UINT* _pIndices, UINT _uIndexCount, 
		Microsoft::WRL::ComPtr<ID3D12Resource> _pIndexBuffer, D3D12_INDEX_BUFFER_VIEW& _pIBView);
	~Entity();
	void SetVertexIndexBufferView(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _pGCL);
	void DrawIndexed(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _pGCL);
	const DirectX::XMMATRIX& GetModelMatrix() const;
	void SetModelMatrix(const DirectX::XMMATRIX& _model);
};