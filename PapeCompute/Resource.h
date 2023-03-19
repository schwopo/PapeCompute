#pragma once

enum class EHeapType
{
    Default,
    Upload
};

enum EResourceFlags : UINT
{
    None = 0,
    AllowUnorderedAccess = (1 << 0)
};

enum EResourceState : UINT
{
        Common = D3D12_RESOURCE_STATE_COMMON,
        VertexAndConstantBuffer = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        IndexBuffer = D3D12_RESOURCE_STATE_INDEX_BUFFER,
        RenderTarget = D3D12_RESOURCE_STATE_RENDER_TARGET,
        UnorderedAccess = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        DepthWrite = D3D12_RESOURCE_STATE_DEPTH_WRITE,
        DepthRead = D3D12_RESOURCE_STATE_DEPTH_READ,
        NonPixelShader = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        PixelShader = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        CopyDest = D3D12_RESOURCE_STATE_COPY_DEST,
        CopySource = D3D12_RESOURCE_STATE_COPY_SOURCE,
        GenericRead = D3D12_RESOURCE_STATE_GENERIC_READ,
        Present = D3D12_RESOURCE_STATE_PRESENT,

        // Unused states, not included to keep autocomplete cleaner
        //D3D12_RESOURCE_STATE_PREDICATION,
        //D3D12_RESOURCE_STATE_STREAM_OUT,
        //D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
        //D3D12_RESOURCE_STATE_RESOLVE_DEST,
        //D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
        //D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        //D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE,
        //D3D12_RESOURCE_STATE_VIDEO_DECODE_READ,
        //D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE,
        //D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ,
        //D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE,
        //D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ,
        //D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE,
};

class CResource
{
public:

    ComPtr<ID3D12Resource> GetD3D12Resource() const { return m_resource; }
    void SetD3D12Resource(ComPtr<ID3D12Resource> resource) { m_resource = resource; }

    void Transition(EResourceState beforeState, EResourceState afterState, ID3D12GraphicsCommandList* commandList);

    static D3D12_HEAP_TYPE GetD3D12HeapType(EHeapType heap);
    static D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(EResourceFlags flags);


private:
    ComPtr<ID3D12Resource> m_resource;
};

