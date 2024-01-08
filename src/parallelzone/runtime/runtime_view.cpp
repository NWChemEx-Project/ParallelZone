/*
 * Copyright 2022 NWChemEx-Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "detail_/resource_set_pimpl.hpp"
#include "detail_/runtime_view_pimpl.hpp"
#include <mpi.h>
#include <parallelzone/logging/logger_factory.hpp>

// N.B. AFAIK the only way a RuntimeView can have no PIMPL is if an exception is
//      thrown in the ctor, the user catches the exception, and uses the
//      RuntimeView anyways. We assume that users won't do this and thus do not
//      assert that the PIMPL is non-null.

namespace parallelzone::runtime {

namespace {

// Basically a ternary statement dispatching on whether we need to initialize
// MPI or not
auto start_mpi(int argc, char** argv, const MPI_Comm& comm) {
    int mpi_initialized;
    MPI_Initialized(&(mpi_initialized));
    if(!mpi_initialized) MPI_Init(&argc, &argv);
    mpi_helpers::CommPP commpp(comm);

    auto log         = LoggerFactory::default_global_logger(commpp.me());
    using pimpl_type = detail_::RuntimeViewPIMPL;
    return std::make_shared<pimpl_type>(!mpi_initialized, commpp,
                                        std::move(log));
}

// start mpi with a callback initialization function
template <typename T>
auto start_mpi_callback(int argc, char** argv, const MPI_Comm& comm,
                        T (*callback_init_1)(int, char**, bool), 
			T (*callback_init_2)(int, char**, const MPI_Comm&, bool)){
    int mpi_initialized;
    MPI_Initialized(&(mpi_initialized));
    if(!mpi_initialized) {
      if(comm == MPI_COMM_WORLD) callback_init_1(argc, argv, true);
      else callback_init_2(argc, argv, comm, true);
    }
    mpi_helpers::CommPP commpp(comm);
    auto log         = LoggerFactory::default_global_logger(commpp.me());
    using pimpl_type = detail_::RuntimeViewPIMPL;
    return std::make_shared<pimpl_type>(!mpi_initialized, commpp,
                                        std::move(log));
}

} // namespace

// -----------------------------------------------------------------------------
// -- Ctors, Assignment, Dtor
// -----------------------------------------------------------------------------

RuntimeView::RuntimeView() : RuntimeView(0, nullptr) {}

RuntimeView::RuntimeView(argc_type argc, argv_type argv) :
  RuntimeView(argc, argv, MPI_COMM_WORLD) {}

RuntimeView::RuntimeView(mpi_comm_type comm) : RuntimeView(0, nullptr, comm) {}

RuntimeView::RuntimeView(int argc, char** argv, mpi_comm_type comm) :
  RuntimeView(start_mpi(argc, argv, comm)) {}

RuntimeView::RuntimeView(pimpl_pointer pimpl) noexcept :
  m_pimpl_(std::move(pimpl)) {}

RuntimeView::RuntimeView(const RuntimeView& other) noexcept = default;

RuntimeView& RuntimeView::operator=(const RuntimeView& rhs) noexcept = default;

RuntimeView::RuntimeView(RuntimeView&& other) noexcept = default;

RuntimeView& RuntimeView::operator=(RuntimeView&& rhs) noexcept = default;

RuntimeView::~RuntimeView() noexcept = default;

// -----------------------------------------------------------------------------
// -- Getters
// -----------------------------------------------------------------------------

RuntimeView::mpi_comm_type RuntimeView::mpi_comm() const noexcept {
    if(null()) return MPI_COMM_NULL;
    return m_pimpl_->m_comm.comm();
}

RuntimeView::size_type RuntimeView::size() const noexcept {
    return !null() ? m_pimpl_->m_comm.size() : 0;
}

bool RuntimeView::null() const noexcept { return !static_cast<bool>(m_pimpl_); }

bool RuntimeView::did_i_start_mpi() const noexcept {
    return !null() ? m_pimpl_->m_did_i_start_mpi : false;
}

RuntimeView::const_resource_set_reference RuntimeView::at(size_type i) const {
    bounds_check_(i);
    return m_pimpl_->at(i);
}

bool RuntimeView::has_me() const noexcept {
    if(null()) return false;
    return m_pimpl_->m_comm.me() != MPI_PROC_NULL;
}

RuntimeView::const_resource_set_reference RuntimeView::my_resource_set() const {
    return pimpl_().at(m_pimpl_->m_comm.me());
}

RuntimeView::size_type RuntimeView::count(const_ram_reference ram) const {
    size_type count = 0;
    for(size_type i = 0; i < size(); ++i) {
        if(at(i).ram() == ram) ++count;
    }
    return count;
}

RuntimeView::logger_reference RuntimeView::logger() const {
    return *pimpl_().m_plogger;
}

// -----------------------------------------------------------------------------
// -- Utility methods
// -----------------------------------------------------------------------------

void RuntimeView::swap(RuntimeView& other) noexcept {
    m_pimpl_.swap(other.m_pimpl_);
}

bool RuntimeView::operator==(const RuntimeView& rhs) const {
    if(null() != rhs.null()) return false;
    if(null()) return true;
    return *m_pimpl_ == *rhs.m_pimpl_;
}

// -----------------------------------------------------------------------------
// -- Private methods
// -----------------------------------------------------------------------------

mpi_helpers::CommPP RuntimeView::comm_() const {
    return mpi_helpers::CommPP(mpi_comm());
}

void RuntimeView::not_null_() const {
    if(!null()) return;
    throw std::runtime_error("RuntimeView is null. Was it default initialized "
                             "or moved from?");
}

void RuntimeView::bounds_check_(size_type i) const {
    if(i < size()) return;
    throw std::out_of_range(std::to_string(i) + " is not in the range [0, " +
                            std::to_string(size()) + ").");
}

RuntimeView::pimpl_reference RuntimeView::pimpl_() {
    not_null_();
    return *m_pimpl_;
}

RuntimeView::const_pimpl_reference RuntimeView::pimpl_() const {
    not_null_();
    return *m_pimpl_;
}

} // namespace parallelzone::runtime
