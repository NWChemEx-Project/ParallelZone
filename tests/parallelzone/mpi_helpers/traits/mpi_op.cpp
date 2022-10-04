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

#include <catch2/catch.hpp>
#include <parallelzone/mpi_helpers/traits/mpi_op.hpp>

using namespace parallelzone::mpi_helpers;

using test_types = std::tuple<float, double>;

TEMPLATE_LIST_TEST_CASE("MPIOp", "", test_types) {
    using T = TestType;
    STATIC_REQUIRE(has_mpi_op_v<std::plus<T>>);
    STATIC_REQUIRE(mpi_op_v<std::plus<T>> == MPI_SUM);

    STATIC_REQUIRE(has_mpi_op_v<std::multiplies<T>>);
    STATIC_REQUIRE(mpi_op_v<std::multiplies<T>> == MPI_PROD);

    STATIC_REQUIRE(has_mpi_op_v<std::logical_and<T>>);
    STATIC_REQUIRE(mpi_op_v<std::logical_and<T>> == MPI_LAND);

    STATIC_REQUIRE(has_mpi_op_v<std::bit_and<T>>);
    STATIC_REQUIRE(mpi_op_v<std::bit_and<T>> == MPI_BAND);

    STATIC_REQUIRE(has_mpi_op_v<std::logical_or<T>>);
    STATIC_REQUIRE(mpi_op_v<std::logical_or<T>> == MPI_LOR);

    STATIC_REQUIRE(has_mpi_op_v<std::bit_or<T>>);
    STATIC_REQUIRE(mpi_op_v<std::bit_or<T>> == MPI_BOR);

    STATIC_REQUIRE(has_mpi_op_v<std::bit_xor<T>>);
    STATIC_REQUIRE(mpi_op_v<std::bit_xor<T>> == MPI_BXOR);
}
