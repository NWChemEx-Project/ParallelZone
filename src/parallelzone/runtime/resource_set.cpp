#include "detail_/resource_set_pimpl.hpp"

namespace parallelzone::runtime {

// -----------------------------------------------------------------------------
// -- Ctors, assignment, and dtor
// -----------------------------------------------------------------------------

ResourceSet::ResourceSet() noexcept = default;

ResourceSet::ResourceSet(pimpl_pointer pimpl) noexcept :
  m_pimpl_(std::move(pimpl)) {}

ResourceSet::ResourceSet(const ResourceSet& other) :
  m_pimpl_(other.m_pimpl_ ? other.m_pimpl_->clone() : nullptr) {}

ResourceSet::ResourceSet(ResourceSet&& other) noexcept = default;

ResourceSet& ResourceSet::operator=(const ResourceSet& rhs) {
    if(this != &rhs) { ResourceSet(rhs).swap(*this); }
    return *this;
}

ResourceSet& ResourceSet::operator=(ResourceSet&& rhs) noexcept = default;

ResourceSet::~ResourceSet() noexcept = default;

// -----------------------------------------------------------------------------
// -- Getters
// -----------------------------------------------------------------------------

ResourceSet::size_type ResourceSet::mpi_rank() const {
    assert_pimpl_();
    return m_pimpl_->m_rank;
}

bool ResourceSet::is_mine() const { throw std::runtime_error("NYI"); }

bool ResourceSet::has_ram() const noexcept { return has_pimpl_(); }

ResourceSet::const_ram_reference ResourceSet::ram() const {
    if(has_ram()) return m_pimpl_->m_ram;
    throw std::out_of_range("ResourceSet has no RAM");
}

ResourceSet::logger_reference ResourceSet::progress_logger() {
    assert_pimpl_();
    return m_pimpl_->progress_logger();
}

ResourceSet::logger_reference ResourceSet::debug_logger() {
    assert_pimpl_();
    return m_pimpl_->debug_logger();
}

// -----------------------------------------------------------------------------
// -- Setters
// -----------------------------------------------------------------------------

void ResourceSet::set_progress_logger(logger_type&& l) {
    assert_pimpl_();
    m_pimpl_->m_progress_logger_pointer =
      std::make_unique<logger_type>(std::move(l));
}

void ResourceSet::set_debug_logger(logger_type&& l) {
    assert_pimpl_();
    m_pimpl_->m_debug_logger_pointer =
      std::make_unique<logger_type>(std::move(l));
}

// -----------------------------------------------------------------------------
// -- Utility methods
// -----------------------------------------------------------------------------

bool ResourceSet::empty() const noexcept {
    return !static_cast<bool>(m_pimpl_);
}

void ResourceSet::swap(ResourceSet& other) noexcept {
    m_pimpl_.swap(other.m_pimpl_);
}

bool ResourceSet::operator==(const ResourceSet& rhs) const {
    // Check if one is empty and other isn't
    if(empty() != rhs.empty()) return false;

    // Both are empty, or both are non-empty, if empty return
    if(empty()) return true;

    // TODO: Implement once we can compare RuntimeView instances
    throw std::runtime_error("NYI");
}

bool ResourceSet::operator!=(const ResourceSet& rhs) const {
    return !(*this == rhs);
}

bool ResourceSet::has_pimpl_() const noexcept {
    return static_cast<bool>(m_pimpl_);
}

void ResourceSet::assert_pimpl_() const {
    if(has_pimpl_()) return;
    throw std::runtime_error(
      "ResourceSet has no PIMPL. Was it default constructed or moved from?");
}

} // namespace parallelzone::runtime
