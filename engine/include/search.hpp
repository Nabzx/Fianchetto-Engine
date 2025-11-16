#pragma once

#include "board.hpp"
#include "types.hpp"
#include <vector>
#include <unordered_map>

namespace fianchetto {

// Transposition table entry
struct TTEntry {
    uint64_t hash;
    int depth;
    int score;
    Move best_move;
    uint8_t flag; // 0 = exact, 1 = lower bound, 2 = upper bound
    uint8_t age;
};

// Transposition table
class TranspositionTable {
public:
    TranspositionTable(size_t size_mb = 16);
    void store(uint64_t hash, int depth, int score, Move best_move, uint8_t flag);
    TTEntry* probe(uint64_t hash);
    void clear();
    void age();

private:
    std::vector<TTEntry> table_;
    size_t size_;
    uint8_t current_age_;
};

// Search statistics
struct SearchStats {
    uint64_t nodes;
    uint64_t qnodes;
    uint64_t tthits;
    int depth;
    Move best_move;
    int best_score;
};

// Search parameters
struct SearchParams {
    int depth = 6;
    int time_limit_ms = 0;
    bool use_neural = false;
    std::string neural_url = "http://neural:8000/evaluate";
};

// Killer moves (moves that caused beta cutoffs)
class KillerMoves {
public:
    void add(int depth, Move move);
    bool is_killer(int depth, Move move) const;
    void clear();

private:
    std::array<std::array<Move, 2>, 64> killers_;
};

// History heuristic (move ordering)
class HistoryHeuristic {
public:
    void update(Color color, Move move, int depth);
    int get_score(Color color, Move move) const;
    void clear();

private:
    std::array<std::array<int, 64>, 64> history_; // [from][to]
};

// Evaluation function
int evaluate(const Board& board);

// Search functions
int negamax(Board& board, int depth, int alpha, int beta, SearchStats& stats, 
            TranspositionTable& tt, KillerMoves& killers, HistoryHeuristic& history,
            const SearchParams& params);

int quiescence(Board& board, int alpha, int beta, SearchStats& stats);

Move search_root(Board& board, const SearchParams& params, SearchStats& stats);

} // namespace fianchetto

