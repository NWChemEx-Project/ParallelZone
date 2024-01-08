# Copyright 2022 NWChemEx-Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.14)
#TODO: Get from git
set(VERSION 1.0)
project(parallelzone VERSION "${VERSION}" LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    nwx_cmake
    GIT_REPOSITORY https://github.com/NWChemEx/NWXCMake
)
FetchContent_MakeAvailable(nwx_cmake)
list(APPEND CMAKE_MODULE_PATH "${nwx_cmake_SOURCE_DIR}/cmake")

set(
    CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake"
    CACHE STRING "" FORCE
)

include(nwx_versions)
include(get_cmaize)

### Options ###
# These are the default values, used only if the user hasn't already set them.
option(BUILD_TESTING "Should we build the tests?" OFF)
option(BUILD_SHARED_LIBS "Should we build shared libraries?" ON)
option(BUILD_PYBIND11_PYBINDINGS "Should we build pybind11 python bindings?" ON)
option(BUILD_CPPYY_PYBINDINGS "Should the python bindings be Cppyy?" OFF)
option(BUILD_CUDA_BINDINGS "Enable CUDA Bindings" OFF )
option(BUILD_HIP_BINDINGS  "Enable HIP Bindings"  OFF )
option(BUILD_SYCL_BINDINGS "Enable SYCL Bindings" OFF )
option(BUILD_PAPI_BINDINGS "Enable PAPI Bindings" OFF )

if (BUILD_CUDA_BINDING OR BUILD_HIP_BINDINGS OR BUILD_SYCL_BINDING)
  include(build_device)
endif()

# Work out the project paths
set(project_inc_dir "${CMAKE_CURRENT_LIST_DIR}/include/${PROJECT_NAME}")
set(project_src_dir "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT_NAME}")

# Project dependencies
set(project_depends "MPI::MPI_CXX")

find_package(MPI REQUIRED)

include(nwx_cxx_api_docs)
nwx_cxx_api_docs("README.md" "${project_inc_dir}" "${project_src_dir}")

cmaize_find_or_build_dependency(
    spdlog
    URL github.com/gabime/spdlog
    VERSION ${NWX_SPDLOG_VERSION}
    BUILD_TARGET spdlog
    FIND_TARGET spdlog::spdlog
    CMAKE_ARGS SPDLOG_INSTALL=ON
)
list(APPEND project_depends spdlog)

# PAPI bindings are enabled, leading to building PAPI with CUDA or rocm support
if("${BUILD_PAPI_BINDINGS}")
    include(build_papi)
endif ()

cmaize_find_or_build_dependency(
    cereal
    URL github.com/USCiLab/cereal
    VERSION ${NWX_CEREAL_VERSION}
    BUILD_TARGET cereal
    FIND_TARGET cereal
    CMAKE_ARGS JUST_INSTALL_CEREAL=ON
)
list(APPEND project_depends cereal)

cmaize_add_library(
    ${PROJECT_NAME}
    SOURCE_DIR "${project_src_dir}"
    INCLUDE_DIRS "${project_inc_dir}"
    DEPENDS "${project_depends}"
)

# N.B. this is a no-op if BUILD_PYBIND11_PYBINDINGS is not turned on
include(nwx_pybind11)
nwx_add_pybind11_module(
    ${PROJECT_NAME}
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/python"
    DEPENDS "${PROJECT_NAME}"
)

if("${BUILD_TESTING}")
    set(CXX_TEST_DIR "${CMAKE_CURRENT_LIST_DIR}/tests/cxx")
    set(PYTHON_TEST_DIR "${CMAKE_CURRENT_LIST_DIR}/tests/python")

    cmaize_find_or_build_dependency(
        Catch2
        URL github.com/catchorg/Catch2
        BUILD_TARGET Catch2
        FIND_TARGET Catch2::Catch2
        VERSION ${NWX_CATCH2_VERSION}
    )
    cmaize_add_tests(
        test_unit_parallelzone
        SOURCE_DIR "${CXX_TEST_DIR}/unit_tests"
        INCLUDE_DIRS "${project_src_dir}"
	DEPENDS Catch2::Catch2 ${PROJECT_NAME}
    )
    cmaize_add_tests(
        test_integration_parallelzone
        SOURCE_DIR "${CXX_TEST_DIR}/integration_tests"
        INCLUDE_DIRS "${project_src_dir}"
        DEPENDS Catch2::Catch2 ${PROJECT_NAME}
    )
    cmaize_add_tests(
        test_parallelzone_docs
        SOURCE_DIR "${CXX_TEST_DIR}/doc_snippets"
        INCLUDE_DIRS "${project_src_dir}"
        DEPENDS Catch2::Catch2 ${PROJECT_NAME}
    )

    # N.B. these are no-ops if BUILD_PYBIND11_PYBINDINGS is not turned on
    nwx_pybind11_tests(
        py_parallelzone "${PYTHON_TEST_DIR}/unit_tests/test_parallelzone.py"
    )
    nwx_pybind11_tests(
        py_doc_snippets "${PYTHON_TEST_DIR}/doc_snippets/test_doc_snippets.py"
    )

    add_test(
        NAME "test_pz_under_mpi"
        COMMAND "${MPIEXEC_EXECUTABLE}" "${MPIEXEC_NUMPROC_FLAG}" "2"
               "${CMAKE_BINARY_DIR}/test_unit_parallelzone"
    )
    add_test(
        NAME "test_pz_docs_under_mpi"
        COMMAND "${MPIEXEC_EXECUTABLE}" "${MPIEXEC_NUMPROC_FLAG}" "2"
                "${CMAKE_BINARY_DIR}/test_parallelzone_docs"
    )
endif()

if("${BUILD_CPPYY_PYBINDINGS}")
    # Cppyy bindings
    include(nwx_python_mods)
    cppyy_make_python_package(
        MPI
        PACKAGE parallelzone
        NAMESPACES parallelzone
    )
endif()

cmaize_add_package(${PROJECT_NAME} NAMESPACE nwx::)
