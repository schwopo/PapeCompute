#include "stdafx.h"
#include "ComputeRenderPass.h"
#include "Device.h"
#include "SwapChain.h"
#include "RenderResources.h"

void CComputeRenderPass::Init()
{
    m_commandList = GetDevice()->CreateGraphicsCommandList();
    TIF(m_commandList->Close());
    m_descriptorHeap.Init(1);

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
    computeSignatureDesc.pStaticSamplers = &s_renderResources.mipSampler;

    
    ComPtr<ID3DBlob> error;
    TIF(D3D12SerializeRootSignature(&computeSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &computeSignature, &error));
    TIF(GetDevice()->GetD3D12Device()->CreateRootSignature(0, computeSignature->GetBufferPointer(), computeSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));


// Create the pipeline state, which includes compiling and loading shaders.
{
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
        TIF(D3DCompileFromFile(L"compute.hlsl", nullptr, nullptr, "main", "cs_5_0", compileFlags, 0, &computeShader, &errorBlob));
    }
    catch (HrException hr)
    {
        if (errorBlob)
        {
            MessageBoxA(NULL, (LPCSTR)errorBlob->GetBufferPointer(), "Shader Compiler Error!", MB_OK | MB_TASKMODAL | MB_ICONWARNING);
            std::exit(1);
        }
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoComputeDesc = {};
    psoComputeDesc.CS.BytecodeLength = computeShader->GetBufferSize();
    psoComputeDesc.CS.pShaderBytecode = computeShader->GetBufferPointer();
    psoComputeDesc.pRootSignature = m_rootSignature.Get();
    TIF(GetDevice()->GetD3D12Device()->CreateComputePipelineState(&psoComputeDesc, IID_PPV_ARGS(&m_pso)));
    m_commandList->SetName(L"ComputeRenderPass");
}


}

void CComputeRenderPass::Evaluate()
{
    int dispatchWidth = s_renderResources.viewport.Width;
    int dispatchHeight = s_renderResources.viewport.Height;

    TIF(m_commandList->Reset(GetDevice()->GetCommandAllocator().Get(), m_pso.Get()));
    ID3D12DescriptorHeap* ppHeapsCompute[] = { m_descriptorHeap.GetHeap()};
    m_commandList->SetDescriptorHeaps(_countof(ppHeapsCompute), ppHeapsCompute);
    m_commandList->SetComputeRootSignature(m_rootSignature.Get());
    m_commandList->SetComputeRootDescriptorTable(0, m_descriptorHeap.GetGpuHandle());
    m_pTargetTexture->Transition(EResourceState::PixelShader, EResourceState::UnorderedAccess, m_commandList.Get());
    m_commandList->Dispatch(dispatchWidth, dispatchHeight, 1);
    m_pTargetTexture->Transition(EResourceState::UnorderedAccess, EResourceState::PixelShader, m_commandList.Get());
    TIF(m_commandList->Close());
}

void CComputeRenderPass::SetTargetTexture(CResource* pTargetTexture)
{
    assert(pTargetTexture);

    m_pTargetTexture = pTargetTexture;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = m_pTargetTexture->GetD3D12Resource()->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;
    m_descriptorHeap.Update(0, *m_pTargetTexture, uavDesc);
}

ComPtr<ID3D12GraphicsCommandList> CComputeRenderPass::GetCommandList()
{
    return m_commandList;
}
