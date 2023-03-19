#include "stdafx.h"
#include "Device.h"
#include "SwapChain.h"

CSwapChain CSwapChain::m_instance;

void CSwapChain::Init(UINT width, UINT height, UINT dxgiFactoryFlags)
{
	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = numBackBuffers;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	TIF(GetDevice()->GetD3D12Device()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    ComPtr<IDXGIFactory4> factory;
    TIF(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = numBackBuffers;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    TIF(factory->CreateSwapChainForHwnd(
        GetDevice()->GetCommandQueue().Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // This sample does not support fullscreen transitions.
    TIF(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    TIF(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();


    // Create frame resources.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	// Create a RTV for each frame.
	for (UINT n = 0; n < numBackBuffers; n++)
	{
		ComPtr<ID3D12Resource> backBuffer;
		TIF(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffer)));
		GetDevice()->GetD3D12Device()->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		m_backBuffers[n].SetD3D12Resource(backBuffer);
		rtvHandle.Offset(1, GetDevice()->GetRtvDescriptorSize());
	}
}

void CSwapChain::OnDestroy()
{
}

CResource& CSwapChain::GetBackBuffer()
{
	return m_backBuffers[m_frameIndex];
}

CD3DX12_CPU_DESCRIPTOR_HANDLE CSwapChain::GetBackBufferView()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, GetDevice()->GetRtvDescriptorSize());
	return rtvHandle;
}

void CSwapChain::Present()
{

    // Present the frame.
    TIF(m_swapChain->Present(1, 0));

    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.
    GetDevice()->FlushGpu();
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

CSwapChain* GetSwapChain()
{
	return CSwapChain::GetInstance();
}
