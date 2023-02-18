#include "stdafx.h"
#include "Resource.h"
#include "Device.h"

D3D12_HEAP_TYPE GetD3D12HeapType(EHeapType heap)
{
    return heap == EHeapType::Upload ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
}

D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(EResourceFlags flags)
{
	return flags | EResourceFlags::AllowUnorderedAccess ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
}

CResource CResource::CreateBuffer(UINT size, EHeapType heap)
{
	CResource resource;

	// Upload heap requires state generic read
	D3D12_RESOURCE_STATES resourceStates = heap == EHeapType::Upload ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON;

	TIF(GetDevice()->GetD3D12Device()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(GetD3D12HeapType(heap)),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		resourceStates,
		nullptr,
		IID_PPV_ARGS(&resource.m_resource)));
    return resource;
}

CResource CResource::Create2DTexture(UINT width, UINT height, EHeapType heap, EResourceFlags flags, UINT mipLevels)
{
	CResource resource;

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = mipLevels;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Flags = GetD3D12ResourceFlags(flags);
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;


	TIF(GetDevice()->GetD3D12Device()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(GetD3D12HeapType(heap)),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&resource.m_resource)));

    return resource;
}
