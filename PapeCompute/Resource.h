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
    static CResource CreateBuffer(UINT size, EHeapType heap = EHeapType::Default);
    static CResource Create2DTexture(UINT width, UINT height, EHeapType heap = EHeapType::Default, EResourceFlags flags = EResourceFlags::None, UINT mipLevels = 1);

private:
    ComPtr<ID3D12Resource> m_resource;
};

