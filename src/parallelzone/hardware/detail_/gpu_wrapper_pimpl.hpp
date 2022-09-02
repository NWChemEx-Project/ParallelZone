#pragma once

#include <memory>

#ifdef BUILD_CUDA_BINDINGS
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#elif defined(BUILD_HIP_BINDINGS)
#include <hip/hip_runtime.h>
#elif defined(BUILD_SYCL_BINDINGS)
#include <CL/sycl/backend/level_zero.hpp>
#include <level_zero/ze_api.h>
#include <level_zero/zes_api.h>
#include <sycl/sycl.hpp>
#endif

namespace parallelzone::detail_ {

struct GpuPIMPL {
    std::pair<std::size_t, std::size_t> get_mem_info() {
        std::size_t mem_free{};
        std::size_t mem_total{};

#ifdef BUILD_CUDA_BINDINGS
        cudaMemGetInfo(&mem_free, &mem_total);
#elif defined(BUILD_HIP_BINDINGS)
        hipMemGetInfo(&mem_free, &mem_total);
#elif defined(BUILD_SYCL_BINDINGS)
        sycl::device myDev(sycl::gpu_selector{});
        ze_device_handle_t ze_device =
          sycl::get_native<sycl::backend::ext_oneapi_level_zero>(myDev);
        zes_device_handle_t zes_device = (zes_device_handle_t)ze_device;

        uint32_t module_count = 0;
        zesDeviceEnumMemoryModules(zes_device, &module_count, nullptr);
        if(module_count > 0) {
            std::vector<zes_mem_handle_t> module_list(module_count);
            std::vector<zes_mem_state_t> state_list(module_count);

            zesDeviceEnumMemoryModules(zes_device, &module_count,
                                       module_list.data());

            for(uint32_t i = 0; i < module_count; ++i) {
                zesMemoryGetState(module_list[i], &(state_list[i]));
                mem_free += state_list[i].free;
                mem_total += state_list[i].size;
            }
        }
#endif

        return std::make_pair(mem_free, mem_total);
    }
};

} // namespace parallelzone::detail_
