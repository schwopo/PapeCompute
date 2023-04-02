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

#pragma once

#include "DXSample.h"
#include "Resource.h"
#include "FullScreenTexturePass.h"
#include "ComputeRenderPass.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class CPapeCompute : public DXSample
{
public:
    CPapeCompute(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    // change these to something more dynamic later
    static const UINT FrameCount = 2;
    const UINT m_textureWidth;
    const UINT m_textureHeight;
    static const UINT TexturePixelSize = 4;    // The number of bytes used to represent a pixel in the texture.

    UINT m_dispatchWidth;
    UINT m_dispatchHeight;

    

    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    // Render Passes
    CComputeRenderPass     m_computeScenePass;
    CFullScreenTexturePass m_presentScenePass;

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12RootSignature> m_rootSignatureCompute;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12PipelineState> m_pipelineStateCompute;
    ComPtr<ID3D12GraphicsCommandList> m_commandList; // also usable for compute
    UINT m_rtvDescriptorSize; // Descriptor offsets, not size of heap
    UINT m_uavDescriptorSize;

    int m_textureSrvHeapIndex = 0;
    int m_textureUavHeapIndex = 0;

    // App resources.
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    CResource m_texture;
    CResource m_vertexBuffer;


    void LoadPipeline();
    void LoadAssets();
    std::vector<UINT8> GenerateTextureData();
};
