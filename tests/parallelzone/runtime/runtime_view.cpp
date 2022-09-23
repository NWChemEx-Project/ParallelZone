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

#include "../test_parallelzone.hpp"
#include <iostream>
#include <sstream>

using namespace parallelzone::runtime;

/* Testing notes
 *
 * Here we have similar caveats to the RuntimeViewPIMPL unit tests, namely we
 * need to avoid restarting and/or shutting down MADNESS (and MPI under it). By
 * time we get into this test the main function will have initialized MADNESS by
 * calling the RuntimeView(argc_type, argv_type) ctor.
 *
 * Without being able to partition a RuntimeView there's really only two states
 * we can prepare a RuntimeView in: a state where we started MADNESS and a state
 * where we did not.
 *
 */

TEST_CASE("RuntimeView") {
    RuntimeView defaulted;
    RuntimeView argc_argv = testing::PZEnvironment::comm_world();

    SECTION("CTors") {
        SECTION("Default") {
            REQUIRE(defaulted.size() > 0);

            // TODO: Implement operator==
            // for(auto i = 0; i < argc_argv.size(); ++i)
            //    REQUIRE(defaulted.at(i) == argc_argv.at(i));
            REQUIRE_FALSE(defaulted.did_i_start_madness());
        }
        SECTION("argc and argv") {
            // The initial state of argc_argv is system dependent
            REQUIRE(argc_argv.did_i_start_madness());
        }
        SECTION("mpi comm") {
            RuntimeView mpi_comm(argc_argv.mpi_comm());
            REQUIRE(mpi_comm.size() == argc_argv.size());

            // TODO: Implement operator==
            // for(auto i = 0; i < argc_argv.size(); ++i)
            //    REQUIRE(mpi_comm.at(i) == argc_argv.at(i));
            REQUIRE_FALSE(mpi_comm.did_i_start_madness());
        }
        SECTION("madness world") {
            RuntimeView mad_world(argc_argv.madness_world());
            REQUIRE(mad_world.size() == argc_argv.size());

            // TODO: Implement operator==
            // for(auto i = 0; i < argc_argv.size(); ++i)
            //    REQUIRE(mad_world.at(i) == argc_argv.at(i));
            REQUIRE_FALSE(mad_world.did_i_start_madness());
        }
        SECTION("primary") {
            RuntimeView primary(0, nullptr, argc_argv.mpi_comm());
            REQUIRE(primary.size() == argc_argv.size());

            // TODO: Implement operator==
            // for(auto i = 0; i < argc_argv.size(); ++i)
            //    REQUIRE(primary.at(i) == argc_argv.at(i));
            REQUIRE_FALSE(primary.did_i_start_madness());
        }
        SECTION("copy") {
            RuntimeView defaulted_copy(defaulted);
            RuntimeView argc_argv_copy(argc_argv);

            // TODO: uncomment when operator== works
            // REQUIRE(defaulted_copy == defaulted);
            // REQUIRE(argc_argv_copy == argc_argv);
        }
        SECTION("copy assignment") {
            RuntimeView defaulted_copy;
            auto pdefaulted_copy = &(defaulted_copy = defaulted);

            RuntimeView argc_argv_copy;
            auto pargc_argv_copy = &(argc_argv_copy = argc_argv);

            REQUIRE(pdefaulted_copy == &defaulted_copy);
            REQUIRE(pargc_argv_copy == &argc_argv_copy);

            // TODO: uncomment when operator== works
            // REQUIRE(defaulted_copy == defaulted);
            // REQUIRE(argc_argv_copy == argc_argv);
        }
        SECTION("move") {
            RuntimeView defaulted_copy(defaulted);
            RuntimeView defaulted_move(std::move(defaulted));

            RuntimeView argc_argv_copy(argc_argv);
            RuntimeView argc_argv_move(std::move(argc_argv));

            // TODO: uncomment when operator== works
            // REQUIRE(defaulted_copy == defaulted_move);
            // REQUIRE(argc_argv_copy == argc_argv_move);
        }
        SECTION("move assignment") {
            RuntimeView defaulted_copy(defaulted);
            RuntimeView defaulted_move;
            auto pdefaulted_move = &(defaulted_move = std::move(defaulted));

            RuntimeView argc_argv_copy(argc_argv);
            RuntimeView argc_argv_move;
            auto pargc_argv_move = &(argc_argv_move = std::move(argc_argv));

            REQUIRE(pdefaulted_move == &defaulted_move);
            REQUIRE(pargc_argv_move == &argc_argv_move);

            // TODO: uncomment when operator== works
            // REQUIRE(defaulted_copy == defaulted_move);
            // REQUIRE(argc_argv_copy == argc_argv_move);
        }
    }

    SECTION("mpi_comm") {
        int result;
        auto comm = defaulted.mpi_comm();
        MPI_Comm_compare(comm, MPI_COMM_WORLD, &result);
        REQUIRE(result == MPI_IDENT);

        comm = argc_argv.mpi_comm();
        MPI_Comm_compare(comm, MPI_COMM_WORLD, &result);
        REQUIRE(result == MPI_IDENT);
    }

    SECTION("madness_world") {
        // REQUIRE(&d.madness_world() == &argc_argv.madness_world());
    }

    SECTION("size()") {
        REQUIRE(defaulted.size() > 0);
        REQUIRE(argc_argv.size() > 0);
    }

    SECTION("did_i_start_madness") {
        REQUIRE_FALSE(defaulted.did_i_start_madness());
        REQUIRE(argc_argv.did_i_start_madness());
    }

    SECTION("at()") {
        auto n_resource_sets = defaulted.size();
        // TODO: implement operator==
        // for(auto i = 0; i < n_resource_sets; ++i) {
        //    REQUIRE(defaulted.at(i) == argc_argv.at(i));
        //}
        REQUIRE_THROWS_AS(defaulted.at(n_resource_sets), std::out_of_range);
    }

    SECTION("at() const") {
        auto n_resource_sets   = defaulted.size();
        const auto& cdefaulted = defaulted;
        const auto& cargc_argv = argc_argv;
        // TODO: implement operator==
        // for(auto i = 0; i < n_resource_sets; ++i) {
        //    REQUIRE(cdefaulted.at(i) == cargc_argv.at(i));
        //}
        // REQUIRE_THROWS_AS(cdefaulted.at(n_resource_sets), std::out_of_range);
    }

    SECTION("has_me()") { // REQUIRE(defaulted.has_me());
    }

    SECTION("my_resource_set") {
        // The instance returned by this method is tested in detail in the
        // ResourceSet test
        // REQUIRE_THROWS_AS(defaulted.my_resource_set(), std::runtime_error);
    }

    SECTION("count(RAM)") {
        RuntimeView::ram_type ram;
        // REQUIRE(defaulted.count(ram));
    }

    SECTION("equal_range") {
        RuntimeView::ram_type ram;
        using const_range = RuntimeView::const_range;
        // REQUIRE(defaulted.equal_range(ram) == const_range{0, 0});
    }

    SECTION("gather") {
        using data_type = std::vector<std::string>;
        data_type local_data(3, "Hello");
        auto rv = defaulted.gather(local_data);
        std::vector<data_type> corr(defaulted.size(), local_data);
        REQUIRE(rv == corr);
    }

    SECTION("reduce") {
        REQUIRE_THROWS_AS(defaulted.reduce(1.23, 2.34), std::runtime_error);
    }

    SECTION("swap") {
        RuntimeView defaulted_copy(defaulted);
        RuntimeView argc_argv_copy(argc_argv);

        defaulted.swap(argc_argv);

        // TODO: Uncomment when operator== works
        // REQUIRE(defaulted == argc_argv_copy);
        // REQUIRE(argc_argv == defaulted_copy);
    }

    SECTION("progress_logger") {
        // Redirect STDOUT to string
        std::stringstream str;
        auto cout_rdbuf = std::cout.rdbuf(str.rdbuf());

        // Print something to progress logger
        argc_argv.progress_logger().stream()
          << "Hello from " << argc_argv.madness_world().rank() << std::flush;

        // Reset STDOUT
        std::cout.rdbuf(cout_rdbuf);

        // Check output was only on root rank
        if(argc_argv.madness_world().rank() == 0) {
            REQUIRE(str.str() == "Hello from 0");
        } else {
            REQUIRE(str.str() == "");
        }
    }

    SECTION("debug_logger") {
        // Redirect STDERR to string
        std::stringstream str;
        auto cerr_rdbuf = std::cerr.rdbuf(str.rdbuf());

        // Print something to debug logger
        argc_argv.debug_logger().stream()
          << "Hello from " << argc_argv.madness_world().rank() << std::flush;

        // Reset STDERR
        std::cerr.rdbuf(cerr_rdbuf);

        // Check output was only on root rank
        if(argc_argv.madness_world().rank() == 0) {
            REQUIRE(str.str() == "Hello from 0");
        } else {
            REQUIRE(str.str() == "");
        }
    }

    SECTION("operator==/operator!=") {
        REQUIRE_THROWS_AS(defaulted == argc_argv, std::runtime_error);
        REQUIRE_THROWS_AS(defaulted != argc_argv, std::runtime_error);
    }
}
