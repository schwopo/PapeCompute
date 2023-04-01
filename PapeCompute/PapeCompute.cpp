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

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        TIF(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        TIF(GetDevice()->GetD3D12Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

        // Create compute root signature


        ComPtr<ID3DBlob> computeSignature;
        D3D12_ROOT_SIGNATURE_DESC computeSignatureDesc = {};
        std::vector<D3D12_ROOT_PARAMETER> computeParams;

        D3D12_ROOT_PARAMETER uav = {};
        uav.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        std::vector<D3D12_DESCRIPTOR_RANGE> uavRanges;
        D3D12_DESCRIPTOR_RANGE uavRange = {};
        uavRange.BaseShaderRegister = 0;
        uavRange.RegisterSpace = 0;
        uavRange.NumDescriptors = 1;
        uavRange.OffsetInDescriptorsFromTableStart = 0;
        uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        uavRanges.push_back(uavRange);
        uav.DescriptorTable.NumDescriptorRanges = uavRanges.size();
        uav.DescriptorTable.pDescriptorRanges = uavRanges.data();
        computeParams.emplace_back(std::move(uav));

        computeSignatureDesc.NumParameters = computeParams.size();
        computeSignatureDesc.pParameters = computeParams.data();
        computeSignatureDesc.NumStaticSamplers = 1;
        computeSignatureDesc.pStaticSamplers = &sampler;

        TIF(D3D12SerializeRootSignature(&computeSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &computeSignature, &error));
        TIF(GetDevice()->GetD3D12Device()->CreateRootSignature(0, computeSignature->GetBufferPointer(), computeSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatureCompute)));

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
			TIF(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob));
			TIF(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob));
			TIF(D3DCompileFromFile(GetAssetFullPath(L"compute.hlsl").c_str(), nullptr, nullptr, "main", "cs_5_0", compileFlags, 0, &computeShader, &errorBlob));
        }
        catch (HrException hr)
        {
			if (errorBlob)
			{
				MessageBoxA(NULL, (LPCSTR) errorBlob->GetBufferPointer(), "Shader Compiler Error!", MB_OK | MB_TASKMODAL | MB_ICONWARNING);
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
        TIF(GetDevice()->GetD3D12Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoComputeDesc = {};
        psoComputeDesc.CS.BytecodeLength = computeShader->GetBufferSize();
        psoComputeDesc.CS.pShaderBytecode = computeShader->GetBufferPointer();
        psoComputeDesc.pRootSignature = m_rootSignatureCompute.Get();
        TIF(GetDevice()->GetD3D12Device()->CreateComputePipelineState(&psoComputeDesc, IID_PPV_ARGS(&m_pipelineStateCompute)));
    }

    // Create the command list.
    TIF(GetDevice()->GetD3D12Device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GetDevice()->GetCommandAllocator().Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

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

	m_texture = GetDevice()->Create2DTexture(m_textureWidth, m_textureHeight);
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.GetD3D12Resource().Get(), 0, 1);
	CResource uploadTexture = GetDevice()->CreateBuffer(uploadBufferSize, EHeapType::Upload); // Needs to stay in scope until we flush
    {

        std::vector<UINT8> texture = GenerateTextureData();

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = m_textureWidth * TexturePixelSize;
        textureData.SlicePitch = textureData.RowPitch * m_textureHeight;

        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
        UpdateSubresources(m_commandList.Get(), m_texture.GetD3D12Resource().Get(), uploadTexture.GetD3D12Resource().Get(), 0, 0, 1, &textureData);
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.GetD3D12Resource().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = m_texture.GetD3D12Resource()->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        m_textureSrvHeapIndex = GetDevice()->GetDescriptorHeap().Insert(m_texture, srvDesc);

        // Create UAV as well so we can write from compute shader
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = m_texture.GetD3D12Resource()->GetDesc().Format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0; // I guess
        uavDesc.Texture2D.PlaneSlice = 0; // I guess
        m_textureUavHeapIndex = GetDevice()->GetDescriptorHeap().Insert(m_texture, uavDesc);
    }
    
    // Close the command list and execute it to begin the initial GPU setup.
    TIF(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    GetDevice()->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.

        // what if we don't


        //WaitForPreviousFrame();
        GetDevice()->FlushGpu();
    }
    m_presentScenePass.Init();
    m_presentScenePass.SetTexture(&m_texture);
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
}

// Render the scene.
void CPapeCompute::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { 
        m_presentScenePass.GetCommandList().Get(),
        m_commandList.Get() 
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

void CPapeCompute::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    TIF(GetDevice()->GetCommandAllocator()->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.

    TIF(m_commandList->Reset(GetDevice()->GetCommandAllocator().Get(), m_pipelineStateCompute.Get()));
    ID3D12DescriptorHeap* ppHeapsCompute[] = { GetDevice()->GetDescriptorHeap().GetHeap()};
    m_commandList->SetDescriptorHeaps(_countof(ppHeapsCompute), ppHeapsCompute);
    m_commandList->SetComputeRootSignature(m_rootSignatureCompute.Get());
    m_commandList->SetComputeRootDescriptorTable(0, GetDevice()->GetDescriptorHeap().GetGpuHandle(m_textureUavHeapIndex));
    m_texture.Transition(EResourceState::PixelShader, EResourceState::UnorderedAccess, m_commandList.Get());
    m_commandList->Dispatch(m_dispatchWidth, m_dispatchHeight, 1);
    m_texture.Transition(EResourceState::UnorderedAccess, EResourceState::PixelShader, m_commandList.Get());
    TIF(m_commandList->Close());

    // Set necessary state.
    m_presentScenePass.Evaluate();



}
