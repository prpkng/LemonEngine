#pragma once

#include <d3dx12.h>
#include "Helpers.h"

namespace Lemon::DX {

class DXDescriptorHeap {
  public:
    DXDescriptorHeap(const ComPtr<ID3D12Device>&       device,
                     D3D12_DESCRIPTOR_HEAP_TYPE type,
                     uint32_t                   capacity,
                     bool                       shaderVisible)
        : capacity_(capacity), currentIndex_(0) {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = type;
        desc.NumDescriptors = capacity;
        desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        CHECK(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap_)), "Failed to create descriptor heap");

        incrementSize_ = device->GetDescriptorHandleIncrementSize(type);
        cpuStart_ = heap_->GetCPUDescriptorHandleForHeapStart();
        if (shaderVisible)
          gpuStart_ = heap_->GetGPUDescriptorHandleForHeapStart();
    }

    struct Allocation {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu;
        D3D12_GPU_DESCRIPTOR_HANDLE gpu;
        uint32_t                    index;
    };

    // Grab the next free slot — never freed in this simple version
    [[nodiscard]] Allocation Allocate() {
        assert(currentIndex_ < capacity_ && "Descriptor heap exhausted");
        const uint32_t i = currentIndex_++;
        return {{cpuStart_.ptr + i * incrementSize_}, {gpuStart_.ptr + i * incrementSize_}, i};
    }

    [[nodiscard]] ID3D12DescriptorHeap* GetHeap() const { return heap_.Get(); }

  private:
    ComPtr<ID3D12DescriptorHeap> heap_;
    D3D12_CPU_DESCRIPTOR_HANDLE  cpuStart_ = {};
    D3D12_GPU_DESCRIPTOR_HANDLE  gpuStart_ = {};
    uint32_t                     incrementSize_ = 0;
    uint32_t                     capacity_ = 0;
    uint32_t                     currentIndex_ = 0;
};

} // namespace Lemon::DX