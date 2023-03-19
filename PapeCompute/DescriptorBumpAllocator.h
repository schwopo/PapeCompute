#include "Resource.h"

#pragma once
class CDescriptorBumpAllocator
{
public:
    void Init(int size);

    int Insert(const CResource& resource, D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc);
    int Insert(const CResource& resource, D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDesc);
    int Insert(D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc);

    int GetHead() { return m_head; };
    ID3D12DescriptorHeap* GetHeap() { return m_heap.Get(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(int n);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(int n);

private:
    ComPtr<ID3D12DescriptorHeap> m_heap;
    int m_head = 1; // use 0 as not present
};

