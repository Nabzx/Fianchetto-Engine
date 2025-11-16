#include "search.hpp"
#include "movegen.hpp"
#ifdef USE_NEURAL
#include "neural_client.hpp"
#endif
#include <algorithm>
#include <climits>
#include <chrono>

namespace fianchetto {

// Piece-square tables (simplified)
static const int PAWN_PST[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

static const int KNIGHT_PST[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

static const int BISHOP_PST[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

static const int ROOK_PST[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};

static const int QUEEN_PST[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

static const int KING_PST[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

// Piece values (centipawns)
static const int PIECE_VALUES[7] = {0, 100, 320, 330, 500, 900, 20000};

int evaluate(const Board& board) {
    int score = 0;

    // Material and PST
    for (int sq = 0; sq < 64; sq++) {
        PieceType piece = board.piece_on(sq);
        if (piece == PieceType::NONE) continue;

        Color color = board.color_on(sq);
        int value = PIECE_VALUES[static_cast<int>(piece)];

        // Flip PST for black
        int pst_sq = sq;
        if (color == Color::BLACK) {
            pst_sq = 63 - sq;
        }

        int pst = 0;
        switch (piece) {
            case PieceType::PAWN: pst = PAWN_PST[pst_sq]; break;
            case PieceType::KNIGHT: pst = KNIGHT_PST[pst_sq]; break;
            case PieceType::BISHOP: pst = BISHOP_PST[pst_sq]; break;
            case PieceType::ROOK: pst = ROOK_PST[pst_sq]; break;
            case PieceType::QUEEN: pst = QUEEN_PST[pst_sq]; break;
            case PieceType::KING: pst = KING_PST[pst_sq]; break;
            default: break;
        }

        if (color == Color::WHITE) {
            score += value + pst;
        } else {
            score -= value + pst;
        }
    }

    // Pawn structure bonuses
    Bitboard white_pawns = board.pieces(PieceType::PAWN, Color::WHITE);
    Bitboard black_pawns = board.pieces(PieceType::PAWN, Color::BLACK);
    
    // Doubled pawns penalty
    for (int file = 0; file < 8; file++) {
        Bitboard file_mask = 0x0101010101010101ULL << file;
        int white_count = __builtin_popcountll(white_pawns & file_mask);
        int black_count = __builtin_popcountll(black_pawns & file_mask);
        if (white_count > 1) score -= 10 * (white_count - 1);
        if (black_count > 1) score += 10 * (black_count - 1);
    }

    // Return score from white's perspective
    return (board.side_to_move() == Color::WHITE) ? score : -score;
}

// TranspositionTable implementation
TranspositionTable::TranspositionTable(size_t size_mb) {
    size_ = (size_mb * 1024 * 1024) / sizeof(TTEntry);
    table_.resize(size_);
    current_age_ = 0;
}

void TranspositionTable::store(uint64_t hash, int depth, int score, Move best_move, uint8_t flag) {
    size_t index = hash % size_;
    TTEntry& entry = table_[index];
    
    // Replace if deeper or same depth with better flag
    if (entry.hash == 0 || entry.depth <= depth || entry.age != current_age_) {
        entry.hash = hash;
        entry.depth = depth;
        entry.score = score;
        entry.best_move = best_move;
        entry.flag = flag;
        entry.age = current_age_;
    }
}

TTEntry* TranspositionTable::probe(uint64_t hash) {
    size_t index = hash % size_;
    TTEntry& entry = table_[index];
    if (entry.hash == hash && entry.age == current_age_) {
        return &entry;
    }
    return nullptr;
}

void TranspositionTable::clear() {
    for (auto& entry : table_) {
        entry.hash = 0;
    }
}

void TranspositionTable::age() {
    current_age_++;
}

// KillerMoves implementation
void KillerMoves::add(int depth, Move move) {
    if (depth < 64 && move != killers_[depth][0]) {
        killers_[depth][1] = killers_[depth][0];
        killers_[depth][0] = move;
    }
}

bool KillerMoves::is_killer(int depth, Move move) const {
    if (depth >= 64) return false;
    return move == killers_[depth][0] || move == killers_[depth][1];
}

void KillerMoves::clear() {
    for (auto& k : killers_) {
        k[0] = Move();
        k[1] = Move();
    }
}

// HistoryHeuristic implementation
void HistoryHeuristic::update(Color color, Move move, int depth) {
    int from = move.from();
    int to = move.to();
    history_[from][to] += depth * depth;
}

int HistoryHeuristic::get_score(Color color, Move move) const {
    return history_[move.from()][move.to()];
}

void HistoryHeuristic::clear() {
    for (auto& row : history_) {
        row.fill(0);
    }
}

// MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
int mvv_lva_score(Move move) {
    static const int victim_values[] = {0, 100, 320, 330, 500, 900, 20000};
    static const int attacker_values[] = {0, 100, 320, 330, 500, 900, 20000};
    
    int victim = victim_values[static_cast<int>(move.captured())];
    int attacker = attacker_values[static_cast<int>(move.piece())];
    return victim * 10 - attacker;
}

// Move ordering
void order_moves(std::vector<Move>& moves, Move hash_move, const KillerMoves& killers, 
                 const HistoryHeuristic& history, int depth, Color stm) {
    std::vector<std::pair<int, Move>> scored;
    
    for (Move move : moves) {
        int score = 0;
        
        // Hash move first
        if (move == hash_move) {
            score = 1000000;
        }
        // Captures (MVV-LVA)
        else if (move.is_capture()) {
            score = 100000 + mvv_lva_score(move);
        }
        // Killer moves
        else if (killers.is_killer(depth, move)) {
            score = 50000;
        }
        // History heuristic
        else {
            score = history.get_score(stm, move);
        }
        
        scored.push_back({score, move});
    }
    
    // Sort by score (descending)
    std::sort(scored.begin(), scored.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    moves.clear();
    for (const auto& p : scored) {
        moves.push_back(p.second);
    }
}

int quiescence(Board& board, int alpha, int beta, SearchStats& stats) {
    stats.qnodes++;
    
    int stand_pat = evaluate(board);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    std::vector<Move> captures = movegen::generate_moves(board);
    // Filter to captures only
    captures.erase(std::remove_if(captures.begin(), captures.end(),
                                  [](Move m) { return !m.is_capture(); }),
                   captures.end());
    
    // Order captures by MVV-LVA
    std::sort(captures.begin(), captures.end(),
              [](Move a, Move b) { return mvv_lva_score(a) > mvv_lva_score(b); });

    for (Move move : captures) {
        if (!board.is_legal_move(move)) continue;
        
        board.make_move(move);
        int score = -quiescence(board, -beta, -alpha, stats);
        board.unmake_move(move);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

int negamax(Board& board, int depth, int alpha, int beta, SearchStats& stats,
            TranspositionTable& tt, KillerMoves& killers, HistoryHeuristic& history,
            const SearchParams& params) {
    stats.nodes++;

    // Check transposition table
    uint64_t hash = board.hash();
    TTEntry* tt_entry = tt.probe(hash);
    if (tt_entry && tt_entry->depth >= depth) {
        stats.tthits++;
        if (tt_entry->flag == 0) { // Exact
            return tt_entry->score;
        } else if (tt_entry->flag == 1 && tt_entry->score >= beta) { // Lower bound
            return tt_entry->score;
        } else if (tt_entry->flag == 2 && tt_entry->score <= alpha) { // Upper bound
            return tt_entry->score;
        }
    }

    // Terminal node
    if (depth == 0) {
        return quiescence(board, alpha, beta, stats);
    }

    // Check for checkmate/stalemate
    std::vector<Move> moves = movegen::generate_legal_moves(board);
    if (moves.empty()) {
        if (board.in_check(board.side_to_move())) {
            return -30000 - depth; // Checkmate
        }
        return 0; // Stalemate
    }

    Move best_move;
    int best_score = INT_MIN;
    Move hash_move = (tt_entry) ? tt_entry->best_move : Move();

    // Order moves
    order_moves(moves, hash_move, killers, history, depth, board.side_to_move());

    uint8_t tt_flag = 2; // Upper bound
    for (Move move : moves) {
        board.make_move(move);
        int score = -negamax(board, depth - 1, -beta, -alpha, stats, tt, killers, history, params);
        board.unmake_move(move);

        if (score > best_score) {
            best_score = score;
            best_move = move;
        }

        if (score > alpha) {
            alpha = score;
            tt_flag = 0; // Exact
        }

        if (alpha >= beta) {
            // Beta cutoff
            if (!move.is_capture()) {
                killers.add(depth, move);
                history.update(board.side_to_move(), move, depth);
            }
            tt.store(hash, depth, beta, move, 1); // Lower bound
            return beta;
        }
    }

    // Store in TT
    tt.store(hash, depth, best_score, best_move, tt_flag);

    return best_score;
}

Move search_root(Board& board, const SearchParams& params, SearchStats& stats) {
    stats = SearchStats{};
    TranspositionTable tt(16); // 16 MB
    KillerMoves killers;
    HistoryHeuristic history;

    Move best_move;
    int best_score = INT_MIN;

    // Iterative deepening
    for (int depth = 1; depth <= params.depth; depth++) {
        stats.depth = depth;
        int alpha = INT_MIN;
        int beta = INT_MAX;

        std::vector<Move> moves = movegen::generate_legal_moves(board);
        if (moves.empty()) break;

        Move current_best = moves[0];
        int current_score = INT_MIN;

        for (Move move : moves) {
            board.make_move(move);
            int score = -negamax(board, depth - 1, -beta, -alpha, stats, tt, killers, history, params);
            board.unmake_move(move);

            if (score > current_score) {
                current_score = score;
                current_best = move;
            }
            if (score > alpha) alpha = score;
        }

        best_move = current_best;
        best_score = current_score;
    }

    stats.best_move = best_move;
    stats.best_score = best_score;
    return best_move;
}

} // namespace fianchetto

