#include "movegen.hpp"
#include "board.hpp"
#include <algorithm>

namespace fianchetto {
namespace movegen {

// Precomputed attack tables
static constexpr int KNIGHT_DELTAS[] = {-17, -15, -10, -6, 6, 10, 15, 17};
static constexpr int KING_DELTAS[] = {-9, -8, -7, -1, 1, 7, 8, 9};
static constexpr int BISHOP_DELTAS[] = {-9, -7, 7, 9};
static constexpr int ROOK_DELTAS[] = {-8, -1, 1, 8};

// Magic bitboard helpers (simplified - using lookup tables for now)
static Bitboard RANK_MASKS[8];
static Bitboard FILE_MASKS[8];
static Bitboard DIAG_MASKS[15];
static Bitboard ANTI_DIAG_MASKS[15];

void init_masks() {
    static bool initialized = false;
    if (initialized) return;

    for (int rank = 0; rank < 8; rank++) {
        RANK_MASKS[rank] = 0xFFULL << (rank * 8);
    }
    for (int file = 0; file < 8; file++) {
        FILE_MASKS[file] = 0x0101010101010101ULL << file;
    }
    for (int diag = 0; diag < 15; diag++) {
        Bitboard bb = 0;
        for (int i = 0; i < 8; i++) {
            int r = diag - 7 + i;
            int f = i;
            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                bb |= 1ULL << square(f, r);
            }
        }
        DIAG_MASKS[diag] = bb;
    }
    for (int diag = 0; diag < 15; diag++) {
        Bitboard bb = 0;
        for (int i = 0; i < 8; i++) {
            int r = diag - 7 + i;
            int f = 7 - i;
            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                bb |= 1ULL << square(f, r);
            }
        }
        ANTI_DIAG_MASKS[diag] = bb;
    }
    initialized = true;
}

Bitboard pawn_attacks(Square sq, Color color) {
    Bitboard attacks = 0;
    int file = file_of(sq);
    int rank = rank_of(sq);

    if (color == Color::WHITE) {
        if (file > 0 && rank < 7) attacks |= 1ULL << square(file - 1, rank + 1);
        if (file < 7 && rank < 7) attacks |= 1ULL << square(file + 1, rank + 1);
    } else {
        if (file > 0 && rank > 0) attacks |= 1ULL << square(file - 1, rank - 1);
        if (file < 7 && rank > 0) attacks |= 1ULL << square(file + 1, rank - 1);
    }
    return attacks;
}

Bitboard knight_attacks(Square sq) {
    Bitboard attacks = 0;
    int file = file_of(sq);
    int rank = rank_of(sq);

    int deltas[] = {-17, -15, -10, -6, 6, 10, 15, 17};
    for (int delta : deltas) {
        int new_sq = sq + delta;
        if (new_sq >= 0 && new_sq < 64) {
            int new_file = file_of(new_sq);
            int new_rank = rank_of(new_sq);
            if (abs(new_file - file) <= 2 && abs(new_rank - rank) <= 2 && 
                (abs(new_file - file) + abs(new_rank - rank) == 3)) {
                attacks |= 1ULL << new_sq;
            }
        }
    }
    return attacks;
}

Bitboard bishop_attacks(Square sq, Bitboard occupied) {
    init_masks();
    Bitboard attacks = 0;
    int file = file_of(sq);
    int rank = rank_of(sq);
    int diag = rank - file + 7;
    int anti_diag = rank + file;

    // Diagonal
    Bitboard diag_mask = DIAG_MASKS[diag];
    Bitboard diag_occupied = diag_mask & occupied;
    Bitboard diag_blockers = diag_occupied & ~(1ULL << sq);
    
    // Ray attacks
    for (int d : BISHOP_DELTAS) {
        int target = sq;
        while (true) {
            target += d;
            if (target < 0 || target >= 64) break;
            int tfile = file_of(target);
            int trank = rank_of(target);
            if (abs(tfile - file) != abs(trank - rank)) break;
            attacks |= 1ULL << target;
            if (occupied & (1ULL << target)) break;
        }
    }
    return attacks;
}

Bitboard rook_attacks(Square sq, Bitboard occupied) {
    init_masks();
    Bitboard attacks = 0;
    int file = file_of(sq);
    int rank = rank_of(sq);

    // Ray attacks
    for (int d : ROOK_DELTAS) {
        int target = sq;
        while (true) {
            target += d;
            if (target < 0 || target >= 64) break;
            int tfile = file_of(target);
            int trank = rank_of(target);
            if (d == -8 || d == 8) {
                if (tfile != file) break;
            } else {
                if (trank != rank) break;
            }
            attacks |= 1ULL << target;
            if (occupied & (1ULL << target)) break;
        }
    }
    return attacks;
}

Bitboard queen_attacks(Square sq, Bitboard occupied) {
    return bishop_attacks(sq, occupied) | rook_attacks(sq, occupied);
}

Bitboard king_attacks(Square sq) {
    Bitboard attacks = 0;
    int file = file_of(sq);
    int rank = rank_of(sq);

    for (int d : KING_DELTAS) {
        int target = sq + d;
        if (target >= 0 && target < 64) {
            int tfile = file_of(target);
            int trank = rank_of(target);
            if (abs(tfile - file) <= 1 && abs(trank - rank) <= 1) {
                attacks |= 1ULL << target;
            }
        }
    }
    return attacks;
}

std::vector<Move> generate_moves(const Board& board) {
    std::vector<Move> moves;
    Color stm = board.side_to_move();
    Color enemy = (stm == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Bitboard own_pieces = board.all_pieces(stm);
    Bitboard enemy_pieces = board.all_pieces(enemy);
    Bitboard all_occupied = own_pieces | enemy_pieces;

    // Pawn moves
    Bitboard pawns = board.pieces(PieceType::PAWN, stm);
    while (pawns) {
        Square from = __builtin_ctzll(pawns);
        pawns &= pawns - 1;

        int rank = rank_of(from);
        int file = file_of(from);
        int push_dir = (stm == Color::WHITE) ? 8 : -8;
        int start_rank = (stm == Color::WHITE) ? 1 : 6;
        int promo_rank = (stm == Color::WHITE) ? 7 : 0;

        // Single push
        Square to = from + push_dir;
        if (to < 64 && board.piece_on(to) == PieceType::NONE) {
            if (rank_of(to) == promo_rank) {
                for (PieceType promo : {PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT}) {
                    moves.push_back(Move(from, to, PieceType::PAWN, PieceType::NONE, promo, MOVE_FLAG_PROMOTION));
                }
            } else {
                moves.push_back(Move(from, to, PieceType::PAWN));
            }

            // Double push
            if (rank == start_rank) {
                Square to2 = to + push_dir;
                if (to2 < 64 && board.piece_on(to2) == PieceType::NONE) {
                    moves.push_back(Move(from, to2, PieceType::PAWN));
                }
            }
        }

        // Captures
        Bitboard attacks = pawn_attacks(from, stm);
        attacks &= enemy_pieces;
        while (attacks) {
            Square to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            PieceType captured = board.piece_on(to);
            if (rank_of(to) == promo_rank) {
                for (PieceType promo : {PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT}) {
                    moves.push_back(Move(from, to, PieceType::PAWN, captured, promo, MOVE_FLAG_PROMOTION));
                }
            } else {
                moves.push_back(Move(from, to, PieceType::PAWN, captured));
            }
        }

        // En passant
        Square ep_sq = board.en_passant_square();
        if (ep_sq < 64) {
            Bitboard ep_attacks = pawn_attacks(from, stm);
            if (ep_attacks & (1ULL << ep_sq)) {
                moves.push_back(Move(from, ep_sq, PieceType::PAWN, PieceType::PAWN, PieceType::NONE, MOVE_FLAG_EN_PASSANT));
            }
        }
    }

    // Knight moves
    Bitboard knights = board.pieces(PieceType::KNIGHT, stm);
    while (knights) {
        Square from = __builtin_ctzll(knights);
        knights &= knights - 1;
        Bitboard attacks = knight_attacks(from) & ~own_pieces;
        while (attacks) {
            Square to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            PieceType captured = board.piece_on(to);
            moves.push_back(Move(from, to, PieceType::KNIGHT, captured));
        }
    }

    // Bishop moves
    Bitboard bishops = board.pieces(PieceType::BISHOP, stm);
    while (bishops) {
        Square from = __builtin_ctzll(bishops);
        bishops &= bishops - 1;
        Bitboard attacks = bishop_attacks(from, all_occupied) & ~own_pieces;
        while (attacks) {
            Square to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            PieceType captured = board.piece_on(to);
            moves.push_back(Move(from, to, PieceType::BISHOP, captured));
        }
    }

    // Rook moves
    Bitboard rooks = board.pieces(PieceType::ROOK, stm);
    while (rooks) {
        Square from = __builtin_ctzll(rooks);
        rooks &= rooks - 1;
        Bitboard attacks = rook_attacks(from, all_occupied) & ~own_pieces;
        while (attacks) {
            Square to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            PieceType captured = board.piece_on(to);
            moves.push_back(Move(from, to, PieceType::ROOK, captured));
        }
    }

    // Queen moves
    Bitboard queens = board.pieces(PieceType::QUEEN, stm);
    while (queens) {
        Square from = __builtin_ctzll(queens);
        queens &= queens - 1;
        Bitboard attacks = queen_attacks(from, all_occupied) & ~own_pieces;
        while (attacks) {
            Square to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            PieceType captured = board.piece_on(to);
            moves.push_back(Move(from, to, PieceType::QUEEN, captured));
        }
    }

    // King moves
    Bitboard king = board.pieces(PieceType::KING, stm);
    if (king) {
        Square from = __builtin_ctzll(king);
        Bitboard attacks = king_attacks(from) & ~own_pieces;
        while (attacks) {
            Square to = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            PieceType captured = board.piece_on(to);
            moves.push_back(Move(from, to, PieceType::KING, captured));
        }

        // Castling
        int rank = rank_of(from);
        if (stm == Color::WHITE) {
            if (board.can_castle_kingside(Color::WHITE)) {
                if ((all_occupied & (0x60ULL << (rank * 8))) == 0) {
                    moves.push_back(Move(from, square(6, rank), PieceType::KING, PieceType::NONE, PieceType::NONE, MOVE_FLAG_CASTLE_KINGSIDE));
                }
            }
            if (board.can_castle_queenside(Color::WHITE)) {
                if ((all_occupied & (0x0EULL << (rank * 8))) == 0) {
                    moves.push_back(Move(from, square(2, rank), PieceType::KING, PieceType::NONE, PieceType::NONE, MOVE_FLAG_CASTLE_QUEENSIDE));
                }
            }
        } else {
            if (board.can_castle_kingside(Color::BLACK)) {
                if ((all_occupied & (0x60ULL << (rank * 8))) == 0) {
                    moves.push_back(Move(from, square(6, rank), PieceType::KING, PieceType::NONE, PieceType::NONE, MOVE_FLAG_CASTLE_KINGSIDE));
                }
            }
            if (board.can_castle_queenside(Color::BLACK)) {
                if ((all_occupied & (0x0EULL << (rank * 8))) == 0) {
                    moves.push_back(Move(from, square(2, rank), PieceType::KING, PieceType::NONE, PieceType::NONE, MOVE_FLAG_CASTLE_QUEENSIDE));
                }
            }
        }
    }

    return moves;
}

std::vector<Move> generate_legal_moves(const Board& board) {
    std::vector<Move> pseudo_legal = generate_moves(board);
    std::vector<Move> legal;
    
    for (Move move : pseudo_legal) {
        if (board.is_legal_move(move)) {
            legal.push_back(move);
        }
    }
    
    return legal;
}

uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1;
    if (depth == 1) {
        return generate_legal_moves(board).size();
    }

    uint64_t nodes = 0;
    std::vector<Move> moves = generate_legal_moves(board);
    
    for (Move move : moves) {
        board.make_move(move);
        nodes += perft(board, depth - 1);
        board.unmake_move(move);
    }
    
    return nodes;
}

} // namespace movegen
} // namespace fianchetto

