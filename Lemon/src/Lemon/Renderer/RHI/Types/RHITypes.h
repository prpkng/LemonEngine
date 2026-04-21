#pragma once

#include <Lemon/Core.h>

namespace Lemon::RHI
{
    // === FORMATS ===  (maps to DXGI_FORMAT / VkFormat)


    /// \brief Surface type Formats, used for textures, surfaces, etc.
    ///
    /// \code maps to DXGI_FORMAT / VkFormat \endcode
    enum class Format : u32
    {
        Unknown,
        RGBA8_UNORM,
        RGBA16_FLOAT,
        RGBA32_FLOAT,
        D32_FLOAT,
        D24_UNORM_S8_UINT,
    };

    /// \brief Input element types. Represents the layout type that can be passed to a vertex layout
    ///
    /// \code also maps to DXGI_FORMAT / VkFormat \endcode
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

    /// \brief Vertex Input rate
    ///
    /// \code maps to D3D12_INPUT_CLASSIFICATION / VkVertexInputRate \endcode
    enum class InputRate : u32 { PerVertex, PerInstance };

#pragma region === SHADER PIPELINE RELATED ===

    /// \brief Shader file stages
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

    /// Geometry topology type
    ///
    /// \code maps to D3D12_PRIMITIVE_TOPOLOGY_TYPE / VkPrimitiveTopology \endcode
    enum class PrimitiveTopology : u32
    {
        /// \brief Specifies a list of triangles with defined vertices, each 3 vertices form a single triangle
        TriangleList,
        /// \brief Specifies a <a href="https://docs.safe.com/fme/html/FME-Form-Documentation/FME-Form/!FME_Geometry/IFMETriangleStrip.htm">
        /// Triangle Strip</a>. The first 3 vertices define the first triangle.
        /// Subsequent vertices form new triangles with the last 2 defined vertices
        ///
        TriangleStrip,
        /// \brief Specifies a <a href="https://docs.safe.com/fme/html/FME-Form-Documentation/FME-Form/!FME_Geometry/IFMETriangleFan.htm">
        /// Triangle Fan</a> sharing a center vertex. The first 3 vertices define the first triangle.
        /// Subsequent vertices form new triangles sharing the center and the last triangle.
        /// This replaces loop-generated polygon indices (although not frequently used)
        TriangleFan,

        /// \brief Specifies a list of lines. Lines do not share vertices, each 2 new vertices form a new line
        LineList,
        /// \brief Specifies a set of lines. Each new vertex connects with the last one
        LineStrip,
        /// \brief Specifies a set of points
        PointList,
    };

    /// \brief Face culling mode
    ///
    /// \code maps to D3D12_CULL_MODE / VkCullModeFlagBits \endcode
    enum class CullMode : u32 { None, Front, Back };

    /// \brief Geometry filling mode (solid / wireframe)
    ///
    /// \code maps to D3D12_FILL_MODE / VkPolygonMode \endcode
    enum class FillMode : u32 { Solid, Wireframe };

    /// \brief Vertex attribute description
    ///
    /// \code maps to D3D12_INPUT_ELEMENT_DESC / VkVertexInputAttributeDescription + VkVertexInputBindingDescription \endcode
    struct VertexAttribute
    {
        std::string semanticName; /// DX12 uses this; Vulkan uses only location
        u32 semanticIndex = 0;
        ElementType format = ElementType::Unknown;
        u32 binding = 0;
        u32 offset = 0;
        InputRate inputRate = InputRate::PerVertex;
        u32 location = 0; /// Vulkan: explicit; DX12: ignored (uses semantic)
    };

    /// \brief Root parameter type
    ///
    /// \sa RootParameter
    enum class RootParamType : u32
    {
        Constants, /// DX12: root constants  | Vulkan: push constants
        ConstantBufferView, /// DX12: root CBV        | Vulkan: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        ShaderResourceView, /// DX12: root SRV        | Vulkan: VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        UnorderedAccessView, /// DX12: root UAV        | Vulkan: VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
    };


    /// \brief Root parameters / Uniform / Push constants / Descriptor bindings
    ///
    /// \code maps to CD3DX12_ROOT_PARAMETER / VkPushConstantRange + VkDescriptorSetLayoutBinding \endcode
    struct RootParameter
    {
        RootParamType type = RootParamType::Constants;
        u32 count = 1; /// num 32-bit values (Constants) or descriptors
        u32 shaderReg = 0; /// DX12: register(bN) | Vulkan: binding point
        u32 space = 0; /// DX12: space N       | Vulkan: set N
        ShaderStage visibility = ShaderStage::All;
    };

    /// \brief Rasterizer state
    ///
    /// \code maps to D3D12_RASTERIZER_DESC / VkPipelineRasterizationStateCreateInfo \endcode
    struct RasterizerState
    {
        CullMode cullMode = CullMode::Back;
        FillMode fillMode = FillMode::Solid;
        bool depthClipEnable = true;
        bool frontAsCCW = false; /// DX12 default is clock-wise; Vulkan default is counter-clockwise
        float depthBias = 0.0f;
    };

    /// \brief Depth/stencil state
    ///
    /// \code maps to D3D12_DEPTH_STENCIL_DESC / VkPipelineDepthStencilStateCreateInfo \endcode
    struct DepthStencilState
    {
        bool depthEnable = true;
        bool depthWrite = true;
        bool stencilEnable = false;
        //Extend with compare op, stencil ops, etc. as needed
    };

    /// \brief Blend state
    ///
    /// \code maps to D3D12_BLEND_DESC / VkPipelineColorBlendStateCreateInfo \endcode
    struct BlendState
    {
        bool blendEnable = false;
        // Extend with src/dst blend factors, blend op, etc.
    };

    /// \brief MSAA
    ///
    /// \code maps to SampleDesc / VkPipelineMultisampleStateCreateInfo \endcode
    struct SampleState {
        u32 count   = 1;
        u32 quality = 0;
    };

#pragma endregion === SHADER PIPELINE RELATED ===

#pragma region === BUFFER RELATED ===

    /// \brief Describes how the buffer memory should be handled by the GPU
    enum class MemoryUsage
    {
        GPU_ONLY, /// maps to D3D12_HEAP_TYPE_DEFAULT
        CPU_TO_GPU, /// maps to D3D12_HEAP_TYPE_UPLOAD
        GPU_TO_CPU /// maps to D3D12_HEAP_TYPE_READBACK
    };

    /// \brief Describes how the buffer should be used by the GPU
    enum class BufferUsage
    {
        Vertex,
        Index,
        Uniform,
        Storage,
        Indirect,
        Staging
    };

    /// \brief A description of a single vertex element entry
    struct VertexElement
    {
        std::string name; /// Entry name (must match the shader definition)
        ElementType type; /// Element type (must match the data)
        u32 offset;  /// Pointer offset
    };

    /// \brief Describes how the input data is laid on the supplied data array
    struct VertexLayout
    {
        std::vector<VertexElement> elements;
        u32 stride;
        InputRate inputRate = InputRate::PerVertex;
    };

#pragma endregion === BUFFER RELATED ===


}
