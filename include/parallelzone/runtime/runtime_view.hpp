#pragma once

#include <madness/world/MADworld.h>
#include <parallelzone/runtime/resource_set.hpp>

namespace parallelzone::runtime {
namespace detail_ {
class RuntimeViewPIMPL;
}

/** @brief A view of the execution environment.
 *
 *  The runtime (environment) is where a program runs. The RuntimeView class
 *  provides an API for querying properties of the runtime. The "view" part of
 *  the name signifies that a RuntimeView instance does not own the state, but
 *  only aliases it.
 *
 *  Users should be aware that a RuntimeView instance may only be a slice of the
 *  actual runtime (either because other programs are running on the computer
 *  or because another part of the program is using resources).
 *
 *  In MPI terms think of RuntimeView as an intra-communicator paralleling
 *  MPI_COMM_WORLD; however, the underlying MPI_Comm need not be MPI_COMM_WORLD.
 *
 *  TODO: Like the RAM class the MPI methods are stubs. In practice they should
 *        be templated on the data type coming in, and the return type should
 *        be worked out via template meta programming. For now we just do double
 *        in, double out. Calling the ops will raise an exception.
 *
 *  @note RuntimeView uses RAII (resource acquisition is initialization) to
 *        manage MADNESS/MPI. This means that if you do supply a handle to an
 *        already initialized MADNESS/MPI runtime, then RuntimeView will assume
 *        it is using MPI_COMM_WORLD and will initialize MADNESS/MPI if
 *        necesary. In the event that RuntimeView does initialize MADNESS/MPI,
 *        it is also responsible for tearing-down MADNESS/MPI when the last
 *        reference to MADNESS/MPI goes out of scope. Internally, this is
 *        managed with a shared_ptr. So if you want to ensure that MADNESS/MPI
 *        is not finalized while you're doing something, make sure you hold on
 *        to a RuntimeView.
 */
class RuntimeView {
public:
    /// Unsigned integral type used for counting, offsets, and indexing
    using size_type = std::size_t;

    /// Type Runtime uses for handles to MPI communicators
    using mpi_comm_type = MPI_Comm;

    /// Type of a read-only reference to a mpi_comm_type object
    using const_mpi_comm_reference = const mpi_comm_type&;

    /// Type of a MADNESS world (sort of it's MPI_Comm)
    using madness_world_type = madness::World;

    /// Type of a modifable reference to a MADNESS world
    using madness_world_reference = madness_world_type&;

    /// Type used to pass the program's argument count (i.e., "argc")
    using argc_type = int;

    /// Type used to pass the program's arguments (i.e., "argv")
    using argv_type = char**;

    /// Type of a set of resources
    using resource_set_type = ResourceSet;

    /// Type of a read/write reference to a resource_set_type object
    using resource_set_reference = resource_set_type&;

    /// Type of a read-only reference to a resource_set_type object
    using const_resource_set_reference = const resource_set_type&;

    /// Type of a Logger
    using logger_type = Logger;

    /// Type of a read/write reference to a logger_type object
    using logger_reference = logger_type&;

    // TODO: Write an iterator class
    /// Type of an interator over a range of resource_set_type instances
    using const_iterator = int;

    /// Pair of iterators to a read-only range of resource_set_type
    using const_range = std::pair<const_iterator, const_iterator>;

    /// Type of the RAM class, ultimately set by ResourceSet::ram_type
    using ram_type = resource_set_type::ram_type;

    /// Ultimately a typedef of ResourceSet::const_ram_reference
    using const_ram_reference = resource_set_type::const_ram_reference;

    /// Type of the class managing the state of this class
    using pimpl_type = detail_::RuntimeViewPIMPL;

    /// Type of a pointer to the PIMPL
    using pimpl_pointer = std::shared_ptr<pimpl_type>;

    // -------------------------------------------------------------------------
    // -- Ctors, Assignment, Dtor
    // -------------------------------------------------------------------------

    /** @brief Automatically starts MADNESS (and MPI) if not started
     *
     *  This ctor calls the primary ctor passing in `argc = 0`,
     *  `argv = nullptr`, and `comm = MPI_COMM_WORLD`. See the primary ctor's
     *  description for more details.
     *
     */
    RuntimeView();

    /** @brief Initializes the runtime given the arguments to `main`
     *
     *  In C/C++ the `main` function takes two positional arguments, @p argc
     *  and @p argv. This ctor simply forwards @p argc and @p argv to the
     *  primary ctor (and sets `comm = MPI_COMM_WORLD`). See the primary ctor's
     *  description for more details.
     *
     *  @param[in] argc The number of arguments the program was called with.
     *                  In conventional usage, @p argc is >= 1, but a value of 0
     *                  is allowed if @p argv is a nullptr.
     *  @param[in] argv The argument to the program. In conventional usage
     *                  @p argv is minimally the name of the program. @p argv
     *                  may be a nullptr if @p argc is 0.
     */
    RuntimeView(argc_type argc, argv_type argv);

    /** @brief Creates a RuntimeView which aliases the provided MPI runtime.
     *
     *  This ctor is primarily meant for initializing a RuntimeView instance
     *  after MPI has already been started. This ctor dispatches to the primary
     *  ctor by passing `argc = 0` and `argv = nullptr`. See the primary ctor
     *  documentation for more details.
     *
     *  @param[in] comm The MPI_Comm this RuntimeView should alias.
     */
    explicit RuntimeView(const_mpi_comm_reference comm);

    /** @brief Creates a RuntimeView which aliases the provided MADNESS runtime.
     *
     *  This ctor is primarily meant for initializing a RuntimeView instance
     *  after MADNESS (and therefore MPI) have already been started. This ctor
     *  dispatches to the primary ctor by passing `argc = 0`, `argv = nullptr`,
     *  and whatever MPI_Comm corresponds to @p world. See the primary ctor
     *  documentation for more details.
     *
     *  @param[in] world The MADNESS runtime instance we are aliasing.
     *
     */
    explicit RuntimeView(madness_world_reference world);

    /** @brief Primary ctor for creating a new RuntimeView
     *
     *  Most of the other ctors dispatch to this ctor. The ctor first determines
     *  if MADNESS has been initialized. If MADNESS has not been initialized,
     *  MADNESS will be initialized and the resulting RuntimeView instance will
     *  be responsible for tearing down MADNESS when it is no longer in use
     *  (as determined by an internal counter). After ensuring MADNESS
     *  is initialized, this ctor will collect information about the computer
     *  and initialize the internal ResourceSets with said information.
     *
     *  @param[in] argc The number of arguments the program was called with.
     *                  In conventional usage, @p argc is >= 1, but a value of 0
     *                  is allowed if @p argv is a nullptr.
     *  @param[in] argv The argument to the program. In conventional usage
     *                  @p argv is minimally the name of the program. @p argv
     *                  may be a nullptr if @p argc is 0.
     *  @param[in] comm The MPI_Comm this RuntimeView should alias.
     *
     *  // TODO: Document throws
     */
    RuntimeView(argc_type argc, argv_type argv, const_mpi_comm_reference comm);

    /** @brief Make a new RuntimeView which aliases the same resources as *this
     *
     *  RuntimeView instances behave a bit like a shared_ptr. The copy ctor is
     *  deep in the sense that the new instance is not an alias of the original
     *  RuntimeView, but shallow in the sense that the new RuntimeView aliases
     *  the same resources as the original.
     *
     *  @param[in] other The RuntimeView we are copying.
     *
     *  @throw None No throw guarantee.
     */
    RuntimeView(const RuntimeView& other) noexcept;

    /** @brief Makes *this alias the same resources as @p rhs
     *
     *  This method will set the internal state so that it aliases the same
     *  resources as @p rhs. This will decrement the reference count to the
     *  resources *this holds before the operation and increase the reference
     *  count to the resources in @p rhs. If the reference count to the
     *  resources in *this reaches 0 as a result of the aforementioned
     *  decrement, they will be released (possibly finalizing MADNESS/MPI).
     *
     *  @param[in] rhs The RuntimeView whose resources we want *this to alias
     *
     *  @return *this after making *this point to the same resources as @p rhs.
     *
     *  @throw None No throw guarantee.
     */
    RuntimeView& operator=(const RuntimeView&) noexcept;

    /// Move has basically no performance benefit over copy and is deleted to
    /// ensure reference counting happens as intended
    ///@{
    RuntimeView(RuntimeView&&) noexcept = delete;
    RuntimeView& operator=(RuntimeView&&) noexcept = delete;
    ///@}

    /** @brief Releases the present RuntimeView instance
     *
     *  RuntimeViews behave somewhat like a shared_ptr. Copies of a RuntimeView
     *  increment an internal counter. As long as the inernal counter is
     *  non-zero the resources (i.e., MADNESS and MPI) will not be finalized.
     *  When the counter reaches zero (and assuming the original RuntimeView
     *  actually initialized MADNESS/MPI) MADNESS and MPI will be finalized.
     *
     *  @throw None No throw guarantee.
     */
    ~RuntimeView() noexcept;

    // -------------------------------------------------------------------------
    // -- Getters
    // -------------------------------------------------------------------------

    /** @brief Returns the MPI communicator powering *this
     *
     *  ParallelZone assumes that MPI is powering the runtime. This method is
     *  used to retrieve the MPI communicator which is managing the resources
     *  in this RuntimeView.
     *
     *  @return A read-only reference to the MPI communicator in *this.
     *
     *  @throw None No throw guarantee.
     */
    const_mpi_comm_reference mpi_comm() const;

    /** @brief Returns the MADNESS world powering *this
     *
     *  @return A read/write reference to the MADNESS world
     *
     *  @throw None No throw guarantee.
     */
    madness_world_reference madness_world() const;

    /** @brief The number of resource sets in this instance.
     *
     *  @return The number of resource sets in this view.
     *
     *  @throw None No throw guarantee.
     */
    size_type size() const noexcept;

    /** @brief Used to determine if this RuntimeView aliases the resources that
     *         started MADNESS.
     *
     *  If a RuntimeView starts MADNESS it's responsible for making sure MADNESS
     *  is finalized. The finalization happens in the dtor when all references
     *  to the original resources are out of scope. This method can be used to
     *  determine if letting the current RuntimeView go out of scope will
     *  decrement the reference count (if the return is true) or not (if the
     *  return is false).
     *
     *  @return True if the aliased resources started MADNESS and false
     *          otherwise.
     *
     *  @throw None No throw guarantee.
     */
    bool did_i_start_madness() const noexcept;

    /** @brief Returns the @p i-th resource set in a read/write state.
     *
     *  This method is used to retrieve a resource set by offset.
     *
     *  @param[in] i The 0-based offset of the desired resource set. @p i is
     *               expected to be in the range [0, size()).
     *
     *  @return A read/write reference to the requested resource set.
     *
     *  @throw std::out_of_range if @p i is not in the range [0, size()).
     *                           Strong throw guarantee.
     */
    resource_set_reference at(size_type i);

    /** @brief Returns the @p i-th resource set in a read-only state.
     *
     *  This method behaves identical to the non-const version, except that the
     *  resulting resource set is read-only.
     *
     *  @param[in] i The 0-based offset of the desired resource set. @p i is
     *               expected to be in the range [0, size()).
     *
     *  @return A read-only reference to the requested resource set.
     *
     *  @throw std::out_of_range if @p i is not in the range [0, size()).
     *                           Strong throw guarantee.
     */
    const_resource_set_reference at(size_type i) const;

    // TODO: Add noexcept after implementing
    /** @brief Used to determine if the current process is part of *this
     *
     *  Each RuntimeView is meant to be treated as the "full" set of resources
     *  available. In practice, it need not actually be the full set, hence the
     *  quotes. After partitioning a RuntimeView into sub-views, each process
     *  can only directly see the resources in the sub-view it was assigned to.
     *  This method is used to determine if the current process was assigned to
     *  *this.
     *
     *  In MPI terms, if *this is wrapping MPI_COMM_WORLD than the current
     *  process is always part of *this. However, for MPI intra-comms which are
     *  proper subsets of MPI_COMM_WORLD, a particular process need not be in
     *  the intra-comm (and by definition at least one is not). This method
     *  tells you if the current process is in the wrapped MPI_Comm or not.
     *
     *  @return True if the current process is in the communicator and false
     *          otherwise.
     *
     *  @throw None No throw guarantee.
     */
    bool has_me() const;

    /** @brief Finds the resource set for the current process.
     *
     *  @throw std::out_of_range if the current process is not part of *this.
     *                           Strong throw guarantee.
     */
    const_resource_set_reference my_resource_set() const;

    // TODO: Make noexcept
    /** @brief Determines the number of ResourceSets a specific RAM instance
     *         belongs to.
     *
     *  ResourceSet instances need not be disjoint. In turn RAM may be locally
     *  accessible to more than one ResourceSet. This method allows you to
     *  determine how many of the ResourceSets in *this have access to the
     *  specified RAM instance.
     *
     *  @param[in] ram The chunk of RAM we are looking for. @p ram may be local
     *                 or remote relative to my_resource_set().
     *
     *  @throw None No throw guarantee.
     */
    size_type count(const_ram_reference ram) const;

    // TODO: Make no except after implementing
    /** @brief Returns a range of ResourceSets that contain @p ram.
     *
     *  ResourceSet instances need not be disjoint. This method finds the
     *  ResourceSets that contain @p ram and returns a pair of iterators to
     *  this set.
     *
     *  @return A pair whose first element is an iterator pointing to the
     *          first ResourceSet containing @p ram and whose second element is
     *          an iterator pointing to just past the last ResourceSet
     *          containing @p ram. The elements of the return are equal if
     *          @p ram is not in *this.
     *
     *  @throw None No throw guarantee.
     */
    const_range equal_range(const_ram_reference ram) const;

    /**
     * @brief Get progress logger for this RuntimeView
     *
     * This logger is intended to print progress information to a specified
     * output associated with this RuntimeView. Defaults to root-only
     * print.
     *
     * @returns Logger instance intended for progress output associated with
     *          this runtime view.
     */
    logger_reference progress_logger();

    /**
     * @brief Get debug logger for this RuntimeView
     *
     * This logger is intended to print debug information to a specified
     * output associated with this RuntimeView. Defaults to root-only
     * print.
     *
     * @returns Logger instance intended for debug output associated with
     *          this runtime view.
     */
    logger_reference debug_logger();

    /**
     * @brief Set progress logger for this RuntimeView
     *
     * @param[in] l Logger instance to override the current progress logger
     *              for this instance.
     */
    void set_progress_logger(logger_type&& l);

    /**
     * @brief Set debug logger for this RuntimeView
     *
     * @param[in] l Logger instance to override the current debug logger
     *              for this instance.
     */
    void set_debug_logger(logger_type&& l);

    // -------------------------------------------------------------------------
    // -- MPI all-to-all methods
    // -------------------------------------------------------------------------

    /** @brief Performs an all gather on the data.
     *
     *  This operation is assumed to involve RAM only.
     *
     *  @param[in] input The data local to the current ResourceSet.
     *
     *  @return A local copy of the gathered data.
     */
    double gather(double input) const;

    /** @brief Performs a reduction on the data.
     *
     *  @param[in] input The data local to the current ResourceSet.
     *  @param[in] op    The functor being used to reduce the data.
     *
     *  @return A local copy of the result of the reduction.
     *
     */
    double reduce(double input, double op) const;

    // -------------------------------------------------------------------------
    // -- Utility methods
    // -------------------------------------------------------------------------

    /** @brief Swaps the state of *this with @p other.
     *
     *  This method simply swaps the pointers of the PIMPLs. As such all
     *  references, etc. to state in *this will remain valid, except that after
     *  this call the references now refer to state in @p other.
     *
     *  @param[in] other The instance to swap *this's state with.
     *
     *  @throw None No throw guarantee.
     */
    void swap(RuntimeView& other) noexcept;

    // TODO: Add noexcept after implementing
    /** @brief Determines if @p rhs is value equal to *this.
     *
     *  Two RuntimeView instances are value equal if they wrap the same
     *  MPI_Comm and have the same resources.
     *
     *  @param[in] rhs The RuntimeView we are comparing to.
     *
     *  @return True if *this is value equal to @p rhs and false otherwise.
     *
     *  @throw None No Throw guarantee.
     */
    bool operator==(const RuntimeView& rhs) const;

private:
    /** @brief Code factorization for ensuring @p i is in the range [0, size())
     *
     *  @param[in] i The index to bounds check.
     *
     *  @throw std::out_of_range if @p i is not in the range [0, size()).
     */
    void bounds_check_(size_type i) const;

    /// Holds the actual state of the Runtime
    pimpl_pointer m_pimpl_;
};

// TODO: Add noexcept when implemented
/** @brief Determines if two RuntimeView instances are different.
 *  @relates RuntimeView
 *
 *  Two RuntimeView instances are different if they are not value equal. See
 *  operator== for the definition of value equal.
 *
 *  @param[in] lhs The RuntimeView instance on the left of the operator.
 *  @param[in] rhs The RuntimeView instance on the right of the operator.
 *
 *  @return False if @p lhs is value equal to @p rhs and true otherwise.
 *
 *  @throw None No throw guarantee.
 */
inline bool operator!=(const RuntimeView& lhs, const RuntimeView& rhs) {
    return !(lhs == rhs);
}

} // namespace parallelzone::runtime
