#include "detail_/gpu_wrapper_pimpl.hpp"
#include <parallelzone/gpu_wrapper.hpp>

namespace parallelzone {

GPU::GPU() : m_pimpl_(std::make_unique<pimpl_type>()) {}

std::pair<std::size_t, std::size_t> GPU::get_mem_info() {
    return m_pimpl_->get_mem_info();
}

} // namespace parallelzone
