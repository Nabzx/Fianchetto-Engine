#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "board.hpp"
#include "movegen.hpp"

TEST_CASE("Perft depth 1", "[perft]") {
    fianchetto::Board board;
    uint64_t nodes = fianchetto::movegen::perft(board, 1);
    REQUIRE(nodes == 20);
}

TEST_CASE("Perft depth 2", "[perft]") {
    fianchetto::Board board;
    uint64_t nodes = fianchetto::movegen::perft(board, 2);
    REQUIRE(nodes == 400);
}

TEST_CASE("Perft depth 3", "[perft]") {
    fianchetto::Board board;
    uint64_t nodes = fianchetto::movegen::perft(board, 3);
    REQUIRE(nodes == 8902);
}

