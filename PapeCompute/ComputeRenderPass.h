#include "d3d12.h"
#include "Resource.h"
#include "DescriptorBumpAllocator.h"

#pragma once

class CComputeRenderPass
{
public:
	void Init();
	void SetTargetTexture(CResource* pTargetTexture);
	void Evaluate();
	ComPtr<ID3D12GraphicsCommandList> GetCommandList();

private:
	ComPtr<ID3D12PipelineState> m_pso;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	CDescriptorBumpAllocator m_descriptorHeap;
	CResource* m_pTargetTexture;
};

