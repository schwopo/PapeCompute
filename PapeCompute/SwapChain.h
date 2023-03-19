#include "Resource.h"

#pragma once
class CSwapChain
{
public:
	static CSwapChain* GetInstance() { return &m_instance; }

	void Init(UINT width, UINT height, UINT dxgiFactoryFlags);
	void OnDestroy();

	void Present();


	CResource& GetBackBuffer();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetBackBufferView();
private:
	static constexpr int numBackBuffers = 2;

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<IDXGISwapChain4> m_swapChain;
	CResource m_backBuffers[numBackBuffers];
    UINT m_frameIndex;

	static CSwapChain m_instance;
};

CSwapChain* GetSwapChain();
