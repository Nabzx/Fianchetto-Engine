#pragma once

#include "types.hpp"
#include "board.hpp"
#include <vector>

namespace fianchetto {
namespace movegen {

// Attack generation (for check detection)
Bitboard pawn_attacks(Square sq, Color color);
Bitboard knight_attacks(Square sq);
Bitboard bishop_attacks(Square sq, Bitboard occupied);
Bitboard rook_attacks(Square sq, Bitboard occupied);
Bitboard queen_attacks(Square sq, Bitboard occupied);
Bitboard king_attacks(Square sq);

// Move generation
std::vector<Move> generate_moves(const Board& board);
std::vector<Move> generate_legal_moves(const Board& board);

// Perft
uint64_t perft(Board& board, int depth);

} // namespace movegen
} // namespace fianchetto

