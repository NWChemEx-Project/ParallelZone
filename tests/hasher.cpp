#include "parallelzone/hasher.hpp"
#include <catch2/catch.hpp>
#include <functional>
#include <iostream>
#include <limits>

using namespace parallelzone;
TEST_CASE("Hashing type traits") {
    STATIC_REQUIRE(std::is_same_v<hash_type, std::string>);
    STATIC_REQUIRE(is_hash_v<hash_type>);
}

TEST_CASE("Hashing with BPHash") {
    struct SA {
        int idx;
        std::vector<int> vec;
        void hash(parallelzone::Hasher& h) const { return h(idx, vec); }
    };
    struct SB {
        int idx;
        std::vector<int> vec;
        void hash(parallelzone::Hasher& h) const { return h(idx, vec); }
    };
    SA sa, sa0;
    SB sb, sb0;
    int i              = 1;
    std::vector<int> v = {1, 2, 3};
    sa.vec             = v;
    sa.idx             = i;
    sb.vec             = v;
    sb.idx             = i;
    auto rv            = std::ref(v);
    auto rs            = std::ref(sa);

    REQUIRE(hash_objects(v) == hash_to_string(make_hash(HashType::Hash128, v)));
    REQUIRE(hash_objects(v) == hash_objects(std::vector<int>{1, 2, 3}));
    REQUIRE_FALSE(hash_objects(rv) == hash_objects(std::vector<int>{3, 2, 1}));
    // BPHash adds typeid to the hash when BPHASH_USE_TYPEID is defined.
    SECTION("BPHASH_USE_TYPEID") {
#ifdef BPHASH_USE_TYPEID
        std::cout << "BPHash uses typeid for hashes. \n";
        REQUIRE_FALSE(hash_objects(rv) == hash_objects(v));
        REQUIRE_FALSE(hash_objects(rs) == hash_objects(i, v));
        REQUIRE_FALSE(hash_objects(sa) == hash_objects(sb));
        REQUIRE_FALSE(hash_objects(sa0) == hash_objects(sb0));
#else
        REQUIRE(hash_objects(rv) == hash_objects(v));
        REQUIRE(hash_objects(rs) == hash_objects(i, v));
        REQUIRE(hash_objects(sa) == hash_objects(sb));
        // Hash of empty objects might still differ even if typeid is not used.
        // REQUIRE(hash_objects(sa0) == hash_objects(sb0));
#endif
    }
    SECTION("Different types, same value") {
        REQUIRE_FALSE(hash_objects(double(1.)) == hash_objects(float(1.)));
        REQUIRE_FALSE(hash_objects(short(1)) == hash_objects(int(1.)));
    }
    SECTION("Epsilon difference") {
        double depsilon = std::numeric_limits<double>::epsilon();
        float fepsilon  = std::numeric_limits<float>::epsilon();
        REQUIRE_FALSE(hash_objects(double(1.)) ==
                      hash_objects(double(1. + depsilon)));
        REQUIRE_FALSE(hash_objects(float(1.)) ==
                      hash_objects(float(1. + fepsilon)));
    }
}

TEST_CASE("Hash std::reference wrapper") {
    int m  = 33;
    auto r = std::ref(m);
    REQUIRE(hash_objects(m) == hash_objects(r));
}

TEST_CASE("Hash std::optional") {
    std::optional<int> novalue;
    std::optional<int> one{1};
    REQUIRE(hash_objects(one) == hash_objects(1));
    REQUIRE(hash_objects(novalue) == "00000000000000000000000000000000");
    REQUIRE_FALSE(hash_objects(one) == hash_objects(novalue));
}

TEST_CASE("Hash std::type_info/std::type_index") {
    class A {};
    class B {};
    std::type_index a(typeid(A));
    std::type_index b(typeid(B));
    std::type_index i(typeid(int));
    const std::type_info& aref(typeid(A));
    const std::type_info& bref(typeid(B));
    const std::type_info& iref(typeid(int));

    REQUIRE(hash_objects(a) == hash_objects(aref));
    REQUIRE_FALSE(hash_objects(a) == hash_objects(b));
    REQUIRE_FALSE(hash_objects(aref) == hash_objects(bref));
    REQUIRE(hash_objects(typeid(int)) == hash_objects(typeid(int).name()));
}
