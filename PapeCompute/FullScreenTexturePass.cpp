#include "stdafx.h"
#include "Device.h"
#include "RenderResources.h"
#include "FullScreenTexturePass.h"

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
};

void CFullScreenTexturePass::Init()
{
    m_commandList = GetDevice()->CreateGraphicsCommandList();

    // Create the root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(GetDevice()->GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &s_renderResources.mipSampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        TIF(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        TIF(GetDevice()->GetD3D12Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ComPtr<ID3DBlob> computeShader;
        ComPtr<ID3DBlob> errorBlob;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        try
        {
            TIF(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob));
            TIF(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob));
        }
        catch (HrException hr)
        {
            if (errorBlob)
            {
                MessageBoxA(NULL, (LPCSTR)errorBlob->GetBufferPointer(), "Shader Compiler Error!", MB_OK | MB_TASKMODAL | MB_ICONWARNING);
                std::exit(1);
            }
        }


        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        TIF(GetDevice()->GetD3D12Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
    }

    // Create the vertex buffer.
    {
        Vertex quadVertices[] =
        {
            { {  1.0f,  1.0f, 1.0f }, { 1.0f, 1.0f } },
            { {  1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f } },
            { { -1.0f,  1.0f, 1.0f }, { 0.0f, 1.0f } },
            { { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f } },
        };

        //const UINT vertexBufferSize = sizeof(triangleVertices);
        const UINT vertexBufferSize = sizeof(quadVertices);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        m_vertexBuffer = GetDevice()->CreateBuffer(vertexBufferSize, EHeapType::Upload);

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        TIF(m_vertexBuffer.GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, quadVertices, sizeof(quadVertices));
        m_vertexBuffer.GetD3D12Resource()->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer.GetD3D12Resource()->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }
}

void CFullScreenTexturePass::SetTexture(CResource* pTexture)
{
    m_pTexture = pTexture;
}

void CFullScreenTexturePass::Evaluate()
{
    //TIF(m_commandList->Reset(GetDevice()->GetCommandAllocator().Get(), nullptr));
    //m_commandList->SetPipelineState(m_pso.Get());
    //m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    //m_pTexture->Transition(EResourceState::UnorderedAccess, EResourceState::PixelShader, m_commandList.Get());

    //// Set necessary state.

    //ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    //m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    //m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

    //m_commandList->RSSetViewports(1, &s_renderResources.viewport);
    //m_commandList->RSSetScissorRects(1, &s_renderResources.m_scissorRect);

    //// Indicate that the back buffer will be used as a render target.
    //m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    //m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    //// Record commands.
    //const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    //m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    //m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    //m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    //m_commandList->DrawInstanced(4, 1, 0, 0);

    //// Indicate that the back buffer will now be used to present.
    //m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    //TIF(m_commandList->Close());
}
