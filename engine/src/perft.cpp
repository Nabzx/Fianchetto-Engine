#include "board.hpp"
#include "movegen.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: fianchetto_perft <depth> [fen]" << std::endl;
        return 1;
    }

    int depth = std::stoi(argv[1]);
    std::string fen = (argc > 2) ? argv[2] : "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    fianchetto::Board board(fen);
    uint64_t nodes = fianchetto::movegen::perft(board, depth);
    
    std::cout << "Perft(" << depth << ") = " << nodes << std::endl;
    return 0;
}

