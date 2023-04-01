#include "stdafx.h"
#include "Device.h"
#include "DescriptorBumpAllocator.h"

void CDescriptorBumpAllocator::Init(int size)
{
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = size;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        TIF(GetDevice()->GetD3D12Device()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_heap)));
}

int CDescriptorBumpAllocator::Insert(const CResource& resource, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart(), m_head, GetDevice()->GetUavDescriptorSize());
	GetDevice()->GetD3D12Device()->CreateShaderResourceView(resource.GetD3D12Resource().Get(), &srvDesc, handle);
    return m_head++;
}

int CDescriptorBumpAllocator::Insert(const CResource& resource, D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart(), m_head, GetDevice()->GetUavDescriptorSize());
	GetDevice()->GetD3D12Device()->CreateUnorderedAccessView(resource.GetD3D12Resource().Get(), nullptr, &uavDesc, handle);
    return m_head++;
}

int CDescriptorBumpAllocator::Insert(D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart(), m_head, GetDevice()->GetUavDescriptorSize());
	GetDevice()->GetD3D12Device()->CreateConstantBufferView(&cbvDesc, handle);
    return m_head++;
}

void CDescriptorBumpAllocator::Update(int idx, const CResource& resource, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart(), idx, GetDevice()->GetUavDescriptorSize());
	GetDevice()->GetD3D12Device()->CreateShaderResourceView(resource.GetD3D12Resource().Get(), &srvDesc, handle);
}

void CDescriptorBumpAllocator::Update(int idx, const CResource& resource, D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart(), idx, GetDevice()->GetUavDescriptorSize());
	GetDevice()->GetD3D12Device()->CreateUnorderedAccessView(resource.GetD3D12Resource().Get(), nullptr, &uavDesc, handle);
}

void CDescriptorBumpAllocator::Update(int idx, D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart(), idx, GetDevice()->GetUavDescriptorSize());
	GetDevice()->GetD3D12Device()->CreateConstantBufferView(&cbvDesc, handle);
}

D3D12_GPU_DESCRIPTOR_HANDLE CDescriptorBumpAllocator::GetGpuHandle(int n)
{
	assert(n < m_head || n == 0);
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_heap->GetGPUDescriptorHandleForHeapStart(), n, GetDevice()->GetUavDescriptorSize());
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE CDescriptorBumpAllocator::GetCpuHandle(int n)
{
	assert(n < m_head || n == 0);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart(), n, GetDevice()->GetUavDescriptorSize());
	return handle;
}
