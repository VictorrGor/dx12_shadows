#include "Entity.h"

Entity::Entity()
{
	uVtxCount = 0;
	uIndexCount = 0;
	mModel = DirectX::XMMatrixIdentity();
}


void Entity::Initialize(Vertex* _pVtx, UINT _uVtxCount,
	Microsoft::WRL::ComPtr<ID3D12Resource> _pVertexBuffer, D3D12_VERTEX_BUFFER_VIEW& _pVBView,
	UINT* _pIndices, UINT _uIndexCount,
	Microsoft::WRL::ComPtr<ID3D12Resource> _pIndexBuffer, D3D12_INDEX_BUFFER_VIEW& _pIBView)
{
	uVtxCount = _uVtxCount;
	pVertexBuffer = _pVertexBuffer;
	mVertexBufferView = std::move(_pVBView);

	uIndexCount = _uIndexCount;
	pIndexBuffer = _pIndexBuffer;
	mIndexBufferView = std::move(_pIBView);
}

Entity::~Entity()
{
}

void Entity::SetVertexIndexBufferView(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _pGCL)
{
	_pGCL->IASetVertexBuffers(0, 1, &mVertexBufferView);
	_pGCL->IASetIndexBuffer(&mIndexBufferView);
}

void Entity::DrawIndexed(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _pGCL)
{
	_pGCL->DrawIndexedInstanced(uIndexCount, 1, 0, 0, 0);
	//_pGCL->DrawInstanced(uVtxCount, 1, 0, 0);
}

const DirectX::XMMATRIX& Entity::GetModelMatrix() const
{
	return mModel;
}

void Entity::SetModelMatrix(const DirectX::XMMATRIX& _model)
{
	mModel = _model;
}
