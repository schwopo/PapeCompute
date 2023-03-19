#pragma once

#include "Resource.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class CDevice
{
    friend CDevice* GetDevice();
public:
    CDevice();

    void OnInit();
    void OnDestroy();

    ComPtr<ID3D12Device> GetD3D12Device() { return m_device; }
    ComPtr<ID3D12CommandAllocator> GetCommandAllocator() { return m_commandAllocator; }
    ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_commandQueue; };

    UINT GetRtvDescriptorSize() { return m_rtvDescriptorSize; }
    UINT GetUavDescriptorSize() { return m_uavDescriptorSize; }

    CResource CreateBuffer(UINT size, EHeapType heap = EHeapType::Default);
    CResource Create2DTexture(UINT width, UINT height, EHeapType heap = EHeapType::Default, EResourceFlags flags = EResourceFlags::None, UINT mipLevels = 1);
    ComPtr<ID3D12GraphicsCommandList> CreateGraphicsCommandList();

    void FlushGpu();

private:
    void GetHardwareAdapter( _In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);

    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;

    // Synchronization objects.
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    UINT m_rtvDescriptorSize;
    UINT m_uavDescriptorSize;
    static CDevice sm_instance;
};
