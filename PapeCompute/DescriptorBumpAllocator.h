#include "Resource.h"

#pragma once
class CDescriptorBumpAllocator
{
public:
    void Init(int size);

    int Insert(const CResource& resource, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc);
    int Insert(const CResource& resource, D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc);
    int Insert(D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc);

    void Update(int idx, const CResource& resource, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc);
    void Update(int idx, const CResource& resource, D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc);
    void Update(int idx, D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc);

    int GetHead() { return m_head; };
    ID3D12DescriptorHeap* GetHeap() { return m_heap.Get(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(int n = 0);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(int n = 0);

private:
    ComPtr<ID3D12DescriptorHeap> m_heap;
    int m_head = 1; // use 0 as not present
};

