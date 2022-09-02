#include <catch2/catch.hpp>
#include <parallelzone/runtime/resource_set.hpp>

using namespace parallelzone::runtime;

// TODO: Tests for non-defaulted resource sets
TEST_CASE("ResourceSet") {
    ResourceSet defaulted;

    SECTION("Ctors and assignment") {
        SECTION("Default") {}

        SECTION("Copy") {
            ResourceSet defaulted_copy(defaulted);
            REQUIRE(defaulted_copy == defaulted);
        }

        SECTION("Move") {
            ResourceSet defaulted_copy(defaulted);
            ResourceSet defaulted_move(std::move(defaulted));
            REQUIRE(defaulted_copy == defaulted_move);
        }

        SECTION("Copy Assignment") {
            ResourceSet defaulted_copy;
            auto pdefaulted_copy = &(defaulted_copy = defaulted);
            REQUIRE(pdefaulted_copy == &defaulted_copy);
            REQUIRE(defaulted_copy == defaulted);
        }

        SECTION("Move Assignment") {
            ResourceSet defaulted_copy(defaulted);
            ResourceSet defaulted_move;
            auto pdefaulted_move = &(defaulted_move = std::move(defaulted));
            REQUIRE(pdefaulted_move == &defaulted_move);
            REQUIRE(defaulted_copy == defaulted_move);
        }
    }

    SECTION("mpi_rank") {
        REQUIRE_THROWS_AS(defaulted.mpi_rank(), std::runtime_error);
    }

    SECTION("is_mine") {
        REQUIRE_THROWS_AS(defaulted.is_mine(), std::runtime_error);
    }

    SECTION("has_ram") { REQUIRE_FALSE(defaulted.has_ram()); }

    SECTION("ram") { REQUIRE_THROWS_AS(defaulted.ram(), std::out_of_range); }

    SECTION("empty") { REQUIRE(defaulted.empty()); }

    SECTION("swap") {
        // TODO: test when we have ResourceSets with different state
    }

    SECTION("operator==/operator!=") {
        ResourceSet other_defaulted;
        REQUIRE(defaulted == other_defaulted);
        REQUIRE_FALSE(defaulted != other_defaulted);

        // TODO: Test non-empty
    }

    SECTION("progress_logger") {
        // TODO: Set and test progress logger
        REQUIRE_THROWS_AS(defaulted.progress_logger(), std::runtime_error);
    }

    SECTION("debug_logger") {
        // TODO: Set and test debug logger
        REQUIRE_THROWS_AS(defaulted.debug_logger(), std::runtime_error);
    }
}
