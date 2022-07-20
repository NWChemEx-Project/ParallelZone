#pragma once
#include <parallelzone/runtime/runtime_view.hpp>

namespace parallelzone::runtime::detail_ {

struct RuntimeViewPIMPL {
    /// Type of the class this PIMPL implements
    using parent_type = RuntimeView;

    /// Ultimately a typedef of RuntimeView::size_type
    using size_type = parent_type::size_type;

    /// Ultimately a typedef of RuntimeView::resource_set_type
    using resource_set_type = parent_type::resource_set_type;

    /// Type of the conatiner holding resource_set_type objects
    using resource_set_container = std::map<size_type, resource_set_type>;

    /// Ultimately a typedef of RuntimeView::madness_world_reference
    using madness_world_reference = parent_type::madness_world_reference;

    /// Ultimately a typedef of RuntimeView::argc_type
    using argc_type = parent_type::argc_type;

    /// Ultimately a typedef of RuntimeView::argv_type
    using argv_type = parent_type::argv_type;

    RuntimeViewPIMPL(bool did_i_start_madness, madness_world_reference world);

    /// Tears down MADNESS, when all references are gone (and if we started it)
    ~RuntimeViewPIMPL() noexcept;

    /// Did this PIMPL start MADNESS?
    bool m_did_i_start_madness;

    /// Reference to the madness world this instance wraps
    madness_world_reference m_world;

    /** @brief The ResourceSets known to this RuntimeView
     *
     *  In practice most MPI ranks are only going to care about their
     *  ResourceSet, so we store them in a map from MPI rank to ResourceSet to
     *  avoid having to create a ResourceSet for each rank, just to be able to
     *  return m_resource_sets[r] for rank r.
     */
    resource_set_container m_resource_sets;
};

inline RuntimeViewPIMPL::RuntimeViewPIMPL(bool did_i_start_madness,
                                          madness_world_reference world) :
  m_did_i_start_madness(did_i_start_madness),
  m_world(world),
  m_resource_sets() {}

inline RuntimeViewPIMPL::~RuntimeViewPIMPL() noexcept {
    if(!m_did_i_start_madness) return;
    madness::finalize();
}

} // namespace parallelzone::runtime::detail_
