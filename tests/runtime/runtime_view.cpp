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
            REQUIRE(defaulted.size() == argc_argv.size());

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
    }

    SECTION("mpi_comm") {
        int result;
        MPI_Comm_compare(defaulted.mpi_comm(), MPI_COMM_WORLD, &result);
        REQUIRE(result == MPI_IDENT);

        MPI_Comm_compare(argc_argv.mpi_comm(), MPI_COMM_WORLD, &result);
        REQUIRE(result == MPI_IDENT);
    }

    SECTION("madness_world") {
        REQUIRE(&defaulted.madness_world() == &argc_argv.madness_world());
    }

    SECTION("size()") { REQUIRE(defaulted.size() == argc_argv.size()); }

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
        REQUIRE_THROWS_AS(cdefaulted.at(n_resource_sets), std::out_of_range);
    }

    SECTION("has_me()") {
        REQUIRE_THROWS_AS(defaulted.has_me(), std::runtime_error);
    }

    SECTION("my_resource_set") {
        REQUIRE_THROWS_AS(defaulted.my_resource_set(), std::runtime_error);
    }

    SECTION("count(RAM)") {
        RuntimeView::ram_type ram;
        REQUIRE_THROWS_AS(defaulted.count(ram), std::runtime_error);
    }

    SECTION("equal_range") {
        RuntimeView::ram_type ram;
        REQUIRE_THROWS_AS(defaulted.equal_range(ram), std::runtime_error);
    }

    SECTION("gather") {
        REQUIRE_THROWS_AS(defaulted.gather(1.23), std::runtime_error);
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
        REQUIRE(str.str() == "Hello from 0");
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
        REQUIRE(str.str() == "Hello from 0");
    }

    SECTION("operator==/operator!=") {
        REQUIRE_THROWS_AS(defaulted == argc_argv, std::runtime_error);
        REQUIRE_THROWS_AS(defaulted != argc_argv, std::runtime_error);
    }
}
