#include "stdafx.h"
#include "Resource.h"
#include "Device.h"

D3D12_HEAP_TYPE CResource::GetD3D12HeapType(EHeapType heap)
{
    return heap == EHeapType::Upload ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
}

D3D12_RESOURCE_FLAGS CResource::GetD3D12ResourceFlags(EResourceFlags flags)
{
	return flags | EResourceFlags::AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
}

