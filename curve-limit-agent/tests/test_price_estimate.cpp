#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "utils.h"

TEST_CASE("hex padding works")
{
    REQUIRE(hex_pad_left("a", 4) == "000a");
}