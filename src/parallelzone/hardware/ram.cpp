#include <parallelzone/hardware/ram.hpp>
#include <stdexcept>

namespace parallelzone::hardware {
namespace detail_ {

MPI_Datatype get_mpi_datatype(std::type_index idx) {
    static std::map<std::type_index, MPI_Datatype> map{
      {std::type_index(typeid(double)), MPI_DOUBLE},
      {std::type_index(typeid(float)), MPI_FLOAT}};

    return map.at(idx);
}

MPI_Op get_mpi_op(ReductionOp op) {
    static std::map<ReductionOp, MPI_Op> map{{ReductionOp::Min, MPI_MIN},
                                             {ReductionOp::Max, MPI_MAX},
                                             {ReductionOp::Sum, MPI_SUM}};

    return map.at(op);
}

struct RAMPIMPL {
    using parent_type = RAM;

    /// Ultimately a typedef of RAM::size_type
    using size_type = parent_type::size_type;

    /// Ultimately a typedef of RAM::pimpl_pointer
    using pimpl_pointer = parent_type::pimpl_pointer;

    explicit RAMPIMPL(size_type size) : m_size(size) {}

    pimpl_pointer clone() const { return std::make_unique<RAMPIMPL>(*this); }

    /// Total size of the RAM
    size_type m_size = 0;

    // -----------------------------------------------------------------------------
    // -- MPI all-to-one operations
    // -----------------------------------------------------------------------------

    template<typename InputType>
    std::optional<InputType> gather_impl(InputType send_data) {
        InputType recv_data;
        MPI_Gather(
          &send_data, 1, get_mpi_datatype(std::type_index(typeid(InputType))),
          &recv_data, 1, get_mpi_datatype(std::type_index(typeid(InputType))),
          0, comm_);
        return (this->rank == root) ? recv_data : std::nullopt;
    }

    template<typename InputType, typename FtorType>
    std::optional<InputType> reduce_impl(InputType send_data, FtorType&& fxn) {
        InputType recv_data;
        MPI_Reduce(&send_data, &recv_data, 1,
                   get_mpi_datatype(std::type_index(typeid(InputType))),
                   get_mpi_op(fxn), 0, comm_);
        return (this->rank == root) ? recv_data : std::nullopt;
    }
};

} // namespace detail_

// -----------------------------------------------------------------------------
// -- Ctors, Assignment, Dtor
// -----------------------------------------------------------------------------

RAM::RAM() noexcept = default;

RAM::RAM(size_type total_size) :
  RAM(std::make_unique<pimpl_type>(total_size)) {}

RAM::RAM(pimpl_pointer pimpl) noexcept : m_pimpl_(std::move(pimpl)) {}

RAM::RAM(const RAM& other) :
  RAM(other.has_pimpl_() ? other.m_pimpl_->clone() : nullptr) {}

RAM::RAM(RAM&& other) noexcept = default;

RAM& RAM::operator=(const RAM& rhs) {
    if(this != &rhs) RAM(rhs).swap(*this);
    return *this;
}

RAM& RAM::operator=(RAM&& rhs) noexcept = default;

RAM::~RAM() noexcept = default;

// -----------------------------------------------------------------------------
// -- Getters
// -----------------------------------------------------------------------------

RAM::size_type RAM::total_space() const noexcept {
    return !empty() ? m_pimpl_->m_size : 0;
}

// -----------------------------------------------------------------------------
// -- MPI all-to-one operations
// -----------------------------------------------------------------------------
template<typename InputType>
std::optional<InputType> RAM::gather(InputType input) const {
    return m_pimpl_->gather_impl(input);
}

template<typename InputType, typename FtorType>
std::optional<InputType> RAM::reduce(InputType input, FtorType&& fxn) const {
    return m_pimpl_->reduce_impl(input, std::forward<FtorType>(fxn));
}

// -----------------------------------------------------------------------------
// -- Utility methods
// -----------------------------------------------------------------------------

bool RAM::empty() const noexcept {
    if(!has_pimpl_()) return true;
    return m_pimpl_->m_size == 0;
}

void RAM::swap(RAM& other) noexcept { m_pimpl_.swap(other.m_pimpl_); }

bool RAM::operator==(const RAM& rhs) const noexcept {
    // Rule out one empty and one non-empty
    if(empty() != rhs.empty()) return false;

    // If both are empty return early
    if(empty()) return true;

    return m_pimpl_->m_size == rhs.m_pimpl_->m_size;
}

// -----------------------------------------------------------------------------
// -- Private methods
// -----------------------------------------------------------------------------

bool RAM::has_pimpl_() const noexcept { return static_cast<bool>(m_pimpl_); }

} // namespace parallelzone::hardware
