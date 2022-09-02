#pragma once
#include <parallelzone/runtime/runtime_view.hpp>

namespace parallelzone::runtime::detail_ {

inline std::shared_ptr<Logger> make_default_stdout_logger(madness::World& w) {
    return w.rank() ? std::make_shared<Logger>(make_null_logger()) :
                      std::make_shared<Logger>(make_stdout_logger());
}

inline std::shared_ptr<Logger> make_default_stderr_logger(madness::World& w) {
    return w.rank() ? std::make_shared<Logger>(make_null_logger()) :
                      std::make_shared<Logger>(make_stderr_logger());
}

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

    /// Logger type
    using logger_type = parent_type::logger_type;

    /// Read/write Logger reference
    using logger_reference = parent_type::logger_reference;

    /// Logger instance pointer
    using logger_pointer = std::shared_ptr<logger_type>;

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

    /// Progress Logger
    logger_pointer m_progress_logger_pointer;

    /// Debug Logger
    logger_pointer m_debug_logger_pointer;

    logger_reference progress_logger() {
        if(!m_progress_logger_pointer)
            throw std::runtime_error("No Progress Logger");
        return *m_progress_logger_pointer;
    }

    logger_reference debug_logger() {
        if(!m_debug_logger_pointer) throw std::runtime_error("No Debug Logger");
        return *m_debug_logger_pointer;
    }
};

inline RuntimeViewPIMPL::RuntimeViewPIMPL(bool did_i_start_madness,
                                          madness_world_reference world) :
  m_did_i_start_madness(did_i_start_madness),
  m_world(world),
  m_resource_sets(),
  m_progress_logger_pointer(make_default_stdout_logger(world)),
  m_debug_logger_pointer(make_default_stderr_logger(world)) {}

inline RuntimeViewPIMPL::~RuntimeViewPIMPL() noexcept {
    if(!m_did_i_start_madness) return;
    madness::finalize();
}

} // namespace parallelzone::runtime::detail_
