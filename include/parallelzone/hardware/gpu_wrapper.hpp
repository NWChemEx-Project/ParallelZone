#pragma once

#include <memory>
#include <utility>

namespace parallelzone {

namespace detail_ {
class GpuPIMPL;
} // namespace detail_

/**
 * @brief Class GPU
 * @details Supports as a convinence wrapper around GPU-vendor APIs
 */
class GPU {
public:
    /// Type of the PIMPL associated with this instance
    using pimpl_type = detail_::GpuPIMPL;

    /**
     * @brief Default constructor
     * Sets up the vendor APIs: CUDA, HIP and SYCL
     */
    GPU();

    /**
     * @brief Get free and available memory for memory resource
     *
     * @return std::pair containg free_size and total_size of memory
     */
    [[nodiscard]] std::pair<std::size_t, std::size_t> get_mem_info();

private:
    using pimpl_pointer = std::unique_ptr<pimpl_type>;
    pimpl_pointer m_pimpl_;
};

} // namespace parallelzone
