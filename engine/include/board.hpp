#pragma once

#include "types.hpp"
#include <string>
#include <array>
#include <vector>


namespace fianchetto {

class Board {
public:
    Board();
    Board(const std::string& fen);

    // FEN operations
    void set_fen(const std::string& fen);
    std::string get_fen() const;

    // Piece operations
    PieceType piece_on(Square sq) const;
    Color color_on(Square sq) const;
    void place_piece(Square sq, PieceType piece, Color color);
    void remove_piece(Square sq);

    // Bitboard operations
    Bitboard pieces(PieceType piece, Color color) const;
    Bitboard all_pieces(Color color) const;
    Bitboard all_pieces() const;

    // Game state
    Color side_to_move() const { return stm_; }
    void set_side_to_move(Color c) { stm_ = c; }

    // Castling rights
    bool can_castle_kingside(Color c) const { return castling_[static_cast<int>(c) * 2]; }
    bool can_castle_queenside(Color c) const { return castling_[static_cast<int>(c) * 2 + 1]; }
    void set_castle_kingside(Color c, bool val) { castling_[static_cast<int>(c) * 2] = val; }
    void set_castle_queenside(Color c, bool val) { castling_[static_cast<int>(c) * 2 + 1] = val; }

    // En passant
    Square en_passant_square() const { return ep_square_; }
    void set_en_passant_square(Square sq) { ep_square_ = sq; }

    // Move counters
    int halfmove_clock() const { return halfmove_clock_; }
    int fullmove_number() const { return fullmove_number_; }
    void set_halfmove_clock(int n) { halfmove_clock_ = n; }
    void set_fullmove_number(int n) { fullmove_number_ = n; }

    // Make/unmake moves
    void make_move(Move move);
    void unmake_move(Move move);

    // Check detection
    bool in_check(Color color) const;
    bool is_legal_move(Move move) const;

    // Zobrist hashing
    uint64_t hash() const { return hash_key_; }
    void update_hash();

private:
    // Bitboards: [color][piece_type]
    std::array<std::array<Bitboard, 7>, 2> bitboards_;

    // Piece array for quick lookup
    std::array<PieceType, 64> pieces_;
    std::array<Color, 64> colors_;

    // Game state
    Color stm_;
    std::array<bool, 4> castling_; // [WK, WQ, BK, BQ]
    Square ep_square_;
    int halfmove_clock_;
    int fullmove_number_;

    // Zobrist hash
    uint64_t hash_key_;

    // Move history for unmake
    struct MoveInfo {
        Move move;
        PieceType captured;
        bool castle_kingside[2];
        bool castle_queenside[2];
        Square ep_square;
        int halfmove_clock;
        uint64_t hash_key;
    };
    std::vector<MoveInfo> history_;


    // Zobrist keys
    static std::array<std::array<std::array<uint64_t, 64>, 7>, 2> zobrist_pieces_;
    static std::array<uint64_t, 4> zobrist_castling_;
    static std::array<uint64_t, 8> zobrist_en_passant_;
    static uint64_t zobrist_side_;

    static void init_zobrist();
    static bool zobrist_initialized_;
};

} // namespace fianchetto

