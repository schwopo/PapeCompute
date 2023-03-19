#include "d3d12.h"
#include "Resource.h"

#pragma once
class CFullScreenTexturePass
{
public:
	void Init();
	void SetTexture(CResource* pTexture);
	void Evaluate();
	void Execute();

private:
	ComPtr<ID3D12PipelineState> m_pso;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	CResource m_vertexBuffer;
	CResource* m_pTexture;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

};

