//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "PapeCompute.h"
#include "RenderResources.h"
#include "Device.h"
#include "SwapChain.h"

CPapeCompute::CPapeCompute(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_textureWidth(width),
    m_textureHeight(height),
    m_dispatchWidth(width),
    m_dispatchHeight(height)
{
    s_renderResources.scissorRect = m_scissorRect;
    s_renderResources.viewport = m_viewport;
}

void CPapeCompute::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void CPapeCompute::LoadPipeline()
{
    GetDevice()->OnInit();

    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    CSwapChain::GetInstance()->Init(m_width, m_height, dxgiFactoryFlags);
    s_renderResources.Init();
}

// Load the sample assets.
void CPapeCompute::LoadAssets()
{
	m_texture = GetDevice()->Create2DTexture(m_textureWidth, m_textureHeight);
    m_texture.GetD3D12Resource()->SetName(L"RenderTexture");

    TIF(GetDevice()->GetD3D12Device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GetDevice()->GetCommandAllocator().Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
    m_commandList->SetName(L"UploadCommandList");
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.GetD3D12Resource().Get(), 0, 1);
	CResource uploadTexture = GetDevice()->CreateBuffer(uploadBufferSize, EHeapType::Upload); // Needs to stay in scope until we flush
	std::vector<UINT8> texture = GenerateTextureData();

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &texture[0];
	textureData.RowPitch = m_textureWidth * TexturePixelSize;
	textureData.SlicePitch = textureData.RowPitch * m_textureHeight;

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(m_commandList.Get(), m_texture.GetD3D12Resource().Get(), uploadTexture.GetD3D12Resource().Get(), 0, 0, 1, &textureData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    
    // Close the command list and execute it to begin the initial GPU setup.
    TIF(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    GetDevice()->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	GetDevice()->FlushGpu();

    m_presentScenePass.Init();
    m_presentScenePass.SetTexture(&m_texture);
    m_computeScenePass.Init();
    m_computeScenePass.SetTargetTexture(&m_texture);
}

// Generate a simple black and white checkerboard texture.
std::vector<UINT8> CPapeCompute::GenerateTextureData()
{
    const UINT rowPitch = m_textureWidth * TexturePixelSize;
    const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = m_textureWidth >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * m_textureHeight;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += TexturePixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0x11 * j;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}

// Update frame-based values.
void CPapeCompute::OnUpdate()
{
    static float y = 0;
    y += 0.01;
    CComputeRenderPass::SRenderParams renderParams;
    renderParams.eye = XMFLOAT4(0.0, y, 0.0, 0.0);
    renderParams.target = XMFLOAT4(0.0, 0.0, 1.0, 0.0);
    m_computeScenePass.SetRenderParams(renderParams);
}

// Render the scene.
void CPapeCompute::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    TIF(GetDevice()->GetCommandAllocator()->Reset());
    m_computeScenePass.Evaluate();
    m_presentScenePass.Evaluate();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { 
        m_computeScenePass.GetCommandList().Get(),
        m_presentScenePass.GetCommandList().Get()
    };

    GetDevice()->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    GetSwapChain()->Present();
}

void CPapeCompute::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    //WaitForPreviousFrame();
    GetDevice()->FlushGpu();
}
