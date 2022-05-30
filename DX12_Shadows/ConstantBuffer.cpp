#include "ConstantBuffer.h"


void ModelCB::Initialize(Microsoft::WRL::ComPtr<ID3D12Device8> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc)
{
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_buffer)));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = m_buffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = m_constantBufferSize;
    _device->CreateConstantBufferView(&cbvDesc, _cpu_desc);

    CD3DX12_RANGE readRange(0, 0);      
    ThrowIfFailed(m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)), "Cannt create CB!");
  }

ModelCB::ModelCB() : m_constantBufferSize(sizeof(ModelCBData))
{
}

void ModelCB::SetResources(const DirectX::XMMATRIX& _model, const DirectX::XMMATRIX& _view, const DirectX::XMMATRIX& _projection)
{
    m_model = DirectX::XMMatrixTranspose(_model);
    m_view = DirectX::XMMatrixTranspose(_view);
    m_projection = DirectX::XMMatrixTranspose(_projection);

    m_data.m_model = m_model;
	m_data.m_mvp = m_projection * m_view * m_model;
    memcpy(m_pCbvDataBegin, &m_data, m_constantBufferSize);
}

ModelCB::~ModelCB()
{ 
    m_buffer->Unmap(0, nullptr);
}

void ModelCB::SetModelMatrix(const DirectX::XMMATRIX& _model)
{
    m_model = DirectX::XMMatrixTranspose(_model);
    m_data.m_model = m_model;
    m_data.m_mvp = m_projection * m_view * m_model;
    memcpy(m_pCbvDataBegin, &m_data, m_constantBufferSize);
}

void ModelCB::SetViewMatrix(const DirectX::XMMATRIX& _view)
{
    m_view = DirectX::XMMatrixTranspose(_view);
    m_data.m_mvp = m_projection * m_view * m_model;
    memcpy(m_pCbvDataBegin, &m_data, m_constantBufferSize);
}

UINT ModelCB::getBufferSize()
{
    return m_constantBufferSize;
}


void FrameCB::SetResources(const Light& _light, const DirectX::XMFLOAT3& _cameraPos)
{
    m_data.m_light = _light;
    m_data.m_cameraPos = _cameraPos;
    memcpy(m_pCbvDataBegin, &m_data, m_constantBufferSize);
}

void FrameCB::Initialize(Microsoft::WRL::ComPtr<ID3D12Device8> _device, D3D12_CPU_DESCRIPTOR_HANDLE& _cpu_desc)
{
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_buffer)));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = m_buffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = m_constantBufferSize;
    _device->CreateConstantBufferView(&cbvDesc, _cpu_desc);

    ThrowIfFailed(m_buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pCbvDataBegin)), "Cannt create CB!");
}

FrameCB::FrameCB() : m_constantBufferSize(sizeof(FrameCBData))
{
}

void FrameCB::SetCameraPosition(const DirectX::XMFLOAT3& _pos)
{
    m_data.m_cameraPos = _pos;
    memcpy(m_pCbvDataBegin, &m_data, m_constantBufferSize);
}

UINT FrameCB::getBufferSize()
{
    return m_constantBufferSize;
}

void FrameCB::SetLightPos(const DirectX::XMFLOAT3& _light_pos)
{
    m_data.m_light.lightPos = _light_pos;
    memcpy(m_pCbvDataBegin, &m_data, m_constantBufferSize);
}

void FrameCB::SetLight(const Light& _light)
{
    m_data.m_light = _light;
    memcpy(m_pCbvDataBegin, &m_data, m_constantBufferSize);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUVirtualAdress()
{
    return m_buffer->GetGPUVirtualAddress();
}
