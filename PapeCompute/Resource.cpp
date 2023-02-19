#include "stdafx.h"
#include "Resource.h"
#include "Device.h"

void CResource::Transition(EResourceState beforeState, EResourceState afterState, ID3D12GraphicsCommandList* commandList)
{
    D3D12_RESOURCE_STATES d3dBeforeState = (D3D12_RESOURCE_STATES)beforeState;
    D3D12_RESOURCE_STATES d3dAfterState = (D3D12_RESOURCE_STATES)afterState;
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(), d3dBeforeState, d3dAfterState));
}

D3D12_HEAP_TYPE CResource::GetD3D12HeapType(EHeapType heap)
{
    return heap == EHeapType::Upload ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
}

D3D12_RESOURCE_FLAGS CResource::GetD3D12ResourceFlags(EResourceFlags flags)
{
	return (flags | EResourceFlags::AllowUnorderedAccess) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
}

