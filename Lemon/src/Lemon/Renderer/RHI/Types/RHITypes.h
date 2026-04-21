#pragma once

#include <Lemon/Core.h>

namespace Lemon::RHI
{
    // === FORMATS ===  (maps to DXGI_FORMAT / VkFormat)
    enum class Format : u32
    {
        Unknown,
        RGBA8_UNORM,
        RGBA16_FLOAT,
        RGBA32_FLOAT,
        D32_FLOAT,
        D24_UNORM_S8_UINT,
    };

    enum class ElementType
    {
        Float, Float2, Float3, Float4,
        Int, Int2, Int3, Int4,
        Uint, Uint2, Uint3, Uint4,
        Byte4, Byte4N, Ubyte4, Ubyte4N,
        Ushort, Ushort2, Ushort4,
        Short2, Short2N,
        Short4, Short4N,
        Unknown
    };

    // === Shader stages ===
    enum class ShaderStage : u32
    {
        Vertex = BIT(0),
        Pixel = BIT(1),
        Compute = BIT(2),
        All = ~0u,
    };

    constexpr ShaderStage operator|(ShaderStage a, ShaderStage b)
    {
        return static_cast<ShaderStage>(
            static_cast<u32>(a) | static_cast<u32>(b));
    }

    // --- Topology --- (maps to D3D12_PRIMITIVE_TOPOLOGY_TYPE / VkPrimitiveTopology)
    enum class PrimitiveTopology : u32
    {
        TriangleList,
        TriangleStrip,
        LineList,
        PointList,
    };

    // --- Cull mode --- (maps to D3D12_CULL_MODE / VkCullModeFlagBits)
    enum class CullMode : u32 { None, Front, Back };

    // --- Fill mode --- (maps to D3D12_FILL_MODE / VkPolygonMode)
    enum class FillMode : u32 { Solid, Wireframe };

    // --- Input rate --- (maps to D3D12_INPUT_CLASSIFICATION / VkVertexInputRate)
    enum class InputRate : u32 { PerVertex, PerInstance };

    // =================================================================================
    // Vertex input layout
    // In DX12 → D3D12_INPUT_ELEMENT_DESC
    // In Vulkan → VkVertexInputAttributeDescription + VkVertexInputBindingDescription
    // =================================================================================
    struct VertexAttribute
    {
        std::string semanticName; // DX12 uses this; Vulkan uses only location
        u32 semanticIndex = 0;
        ElementType format = ElementType::Unknown;
        u32 binding = 0;
        u32 offset = 0;
        InputRate inputRate = InputRate::PerVertex;
        u32 location = 0; // Vulkan: explicit; DX12: ignored (uses semantic)
    };

    // =================================================================================
    // Root parameters / Push constants / Descriptor bindings
    //
    // DX12: CD3DX12_ROOT_PARAMETER (root constants, CBV, SRV, UAV, table)
    // Vulkan: VkPushConstantRange + VkDescriptorSetLayoutBinding
    // =================================================================================
    enum class RootParamType : u32
    {
        Constants, // DX12: root constants  | Vulkan: push constants
        ConstantBufferView, // DX12: root CBV        | Vulkan: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        ShaderResourceView, // DX12: root SRV        | Vulkan: VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        UnorderedAccessView, // DX12: root UAV        | Vulkan: VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    };

    struct RootParameter
    {
        RootParamType type = RootParamType::Constants;
        u32 count = 1; // num 32-bit values (Constants) or descriptors
        u32 shaderReg = 0; // DX12: register(bN) | Vulkan: binding point
        u32 space = 0; // DX12: space N       | Vulkan: set N
        ShaderStage visibility = ShaderStage::All;
    };

    // =================================================================================
    // Rasterizer state
    // DX12: D3D12_RASTERIZER_DESC | Vulkan: VkPipelineRasterizationStateCreateInfo
    // =================================================================================
    struct RasterizerState
    {
        CullMode cullMode = CullMode::Back;
        FillMode fillMode = FillMode::Solid;
        bool depthClipEnable = true;
        bool frontAsCCW = false; // DX12 default is clock-wise; Vulkan default is counter-clockwise
        float depthBias = 0.0f;
    };

    // =================================================================================
    // Depth/stencil state
    // DX12: D3D12_DEPTH_STENCIL_DESC | Vulkan: VkPipelineDepthStencilStateCreateInfo
    // =================================================================================
    struct DepthStencilState
    {
        bool depthEnable = true;
        bool depthWrite = true;
        bool stencilEnable = false;
        //Extend with compare op, stencil ops, etc. as needed
    };

    // =================================================================================
    // Blend state
    // DX12: D3D12_BLEND_DESC | Vulkan: VkPipelineColorBlendStateCreateInfo
    // =================================================================================
    struct BlendState
    {
        bool blendEnable = false;
        // Extend with src/dst blend factors, blend op, etc.
    };

    // =================================================================================
    // MSAA
    // DX12: SampleDesc | Vulkan: VkPipelineMultisampleStateCreateInfo
    // =================================================================================
    struct SampleState {
        u32 count   = 1;
        u32 quality = 0;
    };







}
