#pragma once

#include <cstdint>
#include <string>

namespace fianchetto {

// Piece types
enum class PieceType : uint8_t {
    NONE = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6
};

// Colors
enum class Color : uint8_t {
    WHITE = 0,
    BLACK = 1
};

// Squares (0-63, a1=0, h8=63)
using Square = uint8_t;

// Bitboard type
using Bitboard = uint64_t;

// Move representation (32-bit packed)
// Format: [6 bits: from][6 bits: to][3 bits: piece][3 bits: captured][2 bits: promotion][12 bits: flags]
struct Move {
    uint32_t data;

    Move() : data(0) {}
    Move(uint32_t d) : data(d) {}
    Move(Square from, Square to, PieceType piece, PieceType captured = PieceType::NONE, 
         PieceType promotion = PieceType::NONE, uint16_t flags = 0) {
        data = (from) | (to << 6) | (static_cast<uint32_t>(piece) << 12) |
               (static_cast<uint32_t>(captured) << 15) | (static_cast<uint32_t>(promotion) << 18) |
               (flags << 20);
    }

    Square from() const { return data & 0x3F; }
    Square to() const { return (data >> 6) & 0x3F; }
    PieceType piece() const { return static_cast<PieceType>((data >> 12) & 0x7); }
    PieceType captured() const { return static_cast<PieceType>((data >> 15) & 0x7); }
    PieceType promotion() const { return static_cast<PieceType>((data >> 18) & 0x7); }
    uint16_t flags() const { return (data >> 20) & 0xFFF; }
    bool is_capture() const { return captured() != PieceType::NONE; }
    bool is_promotion() const { return promotion() != PieceType::NONE; }
    bool is_castling() const { return (flags() & 0x1) != 0; }
    bool is_en_passant() const { return (flags() & 0x2) != 0; }

    bool operator==(const Move& other) const { return data == other.data; }
    bool operator!=(const Move& other) const { return data != other.data; }
};

// Move flags
constexpr uint16_t MOVE_FLAG_CASTLE_KINGSIDE = 0x1;
constexpr uint16_t MOVE_FLAG_CASTLE_QUEENSIDE = 0x2;
constexpr uint16_t MOVE_FLAG_EN_PASSANT = 0x4;
constexpr uint16_t MOVE_FLAG_PROMOTION = 0x8;

// Square helpers
inline Square square(int file, int rank) {
    return static_cast<Square>(rank * 8 + file);
}

inline int file_of(Square sq) { return sq % 8; }
inline int rank_of(Square sq) { return sq / 8; }

// String conversion
std::string square_to_string(Square sq);
std::string move_to_string(Move move);
Move string_to_move(const std::string& str);

} // namespace fianchetto

