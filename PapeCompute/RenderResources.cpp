#include "stdafx.h"
#include "RenderResources.h"

SRenderResources s_renderResources;

void SRenderResources::Init()
{
        mipSampler = {};
        mipSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        mipSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        mipSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        mipSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        mipSampler.MipLODBias = 0;
        mipSampler.MaxAnisotropy = 0;
        mipSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        mipSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        mipSampler.MinLOD = 0.0f;
        mipSampler.MaxLOD = D3D12_FLOAT32_MAX;
        mipSampler.ShaderRegister = 0;
        mipSampler.RegisterSpace = 0;
        mipSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}
