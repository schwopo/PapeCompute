#pragma once

enum class EHeapType
{
    Default,
    Upload
};

enum EResourceFlags : UINT
{
    None = 0,
    AllowUnorderedAccess = (1 << 0)
};

class CResource
{
public:

    ComPtr<ID3D12Resource> GetD3D12Resource() { return m_resource; }
    void SetD3D12Resource(ComPtr<ID3D12Resource> resource) { m_resource = resource; }

    static D3D12_HEAP_TYPE GetD3D12HeapType(EHeapType heap);
    static D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(EResourceFlags flags);


private:
    ComPtr<ID3D12Resource> m_resource;
};

