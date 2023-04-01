#include "Resource.h"

#pragma once

struct SRenderResources
{
	D3D12_STATIC_SAMPLER_DESC mipSampler;
    CD3DX12_VIEWPORT viewport;
    CD3DX12_RECT scissorRect;

    void Init();
};

extern SRenderResources s_renderResources;
