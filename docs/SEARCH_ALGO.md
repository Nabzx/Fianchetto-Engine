# Search Algorithm Documentation

## Overview

The Fianchetto Engine uses a classical minimax search with alpha-beta pruning, implemented in negamax form. The search is enhanced with iterative deepening, transposition tables, and sophisticated move ordering.

## Negamax Algorithm

### Basic Structure

The negamax algorithm is a variant of minimax that simplifies the code by always evaluating from the current player's perspective:

```cpp
int negamax(Board& board, int depth, int alpha, int beta, ...) {
    if (depth == 0) {
        return quiescence(board, alpha, beta, stats);
    }
    
    // Generate and search moves
    for (Move move : moves) {
        board.make_move(move);
        int score = -negamax(board, depth - 1, -beta, -alpha, ...);
        board.unmake_move(move);
        
        if (score >= beta) {
            return beta;  // Beta cutoff
        }
        if (score > alpha) {
            alpha = score;  // New best
        }
    }
    
    return alpha;
}
```

### Alpha-Beta Pruning

Alpha-beta pruning eliminates branches that cannot affect the final result:

- **Alpha**: Best score the maximizing player can guarantee
- **Beta**: Best score the minimizing player can guarantee
- When `alpha >= beta`, the branch is pruned (beta cutoff)

## Iterative Deepening

Instead of searching directly to the target depth, the engine searches incrementally:

1. Search to depth 1
2. Search to depth 2
3. ...
4. Search to target depth

**Benefits:**
- Better move ordering from previous iterations
- Transposition table gets populated with shallower results
- Can stop early if time runs out
- Best move from previous iteration is tried first

## Transposition Table

### Structure

Each transposition table entry contains:
- **Hash**: Position hash (Zobrist)
- **Depth**: Search depth
- **Score**: Evaluation score
- **Best Move**: Best move found
- **Flag**: Node type (exact, lower bound, upper bound)
- **Age**: For replacement strategy

### Replacement Strategy

When storing an entry:
- Replace if entry is empty
- Replace if new entry has greater depth
- Replace if entry is from an older search (age)

### Lookup

When probing:
- If hash matches and depth >= current depth:
  - **Exact**: Return score directly
  - **Lower bound >= beta**: Return score (beta cutoff)
  - **Upper bound <= alpha**: Return score (alpha cutoff)

## Move Ordering

Move ordering is critical for alpha-beta efficiency. Moves are ordered by:

### 1. Hash Move

The best move from the transposition table is tried first.

### 2. Captures (MVV-LVA)

Captures are sorted by MVV-LVA (Most Valuable Victim - Least Valuable Attacker):

```cpp
int mvv_lva_score(Move move) {
    int victim = victim_values[move.captured()];
    int attacker = attacker_values[move.piece()];
    return victim * 10 - attacker;
}
```

This prioritizes capturing valuable pieces with less valuable pieces.

### 3. Killer Moves

Moves that caused beta cutoffs at the same depth are stored as "killer moves" and tried early in sibling nodes.

### 4. History Heuristic

Moves that performed well historically (caused cutoffs) are scored higher. The history table is indexed by `[from_square][to_square]`.

## Quiescence Search

After reaching depth 0, quiescence search continues with captures to avoid the "horizon effect" (missing tactics just beyond the search depth).

```cpp
int quiescence(Board& board, int alpha, int beta, ...) {
    int stand_pat = evaluate(board);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;
    
    // Search captures only
    for (Move capture : captures) {
        // ... search capture ...
    }
    
    return alpha;
}
```

## Evaluation Function

### Material

Basic piece values in centipawns:
- Pawn: 100
- Knight: 320
- Bishop: 330
- Rook: 500
- Queen: 900
- King: 20000

### Piece-Square Tables

Each piece type has a table of positional bonuses:

```cpp
static const int PAWN_PST[64] = {
    // Rank 8 (from white's perspective)
    0,  0,  0,  0,  0,  0,  0,  0,
    // Rank 7
    50, 50, 50, 50, 50, 50, 50, 50,
    // ... more ranks
};
```

### Positional Features

- **Pawn structure**: Doubled pawns penalty
- **Center control**: Bonus for central squares
- **King safety**: Penalty for exposed king

## Search Statistics

The engine tracks:
- **Nodes**: Total nodes searched
- **QNodes**: Quiescence nodes
- **TTHits**: Transposition table hits
- **Depth**: Current search depth
- **Best Move**: Best move found
- **Best Score**: Best evaluation

## Performance Optimizations

1. **Bitboard operations**: Fast piece location and attack generation
2. **Move ordering**: Reduces nodes searched by 10-100x
3. **Transposition table**: Avoids re-searching known positions
4. **Quiescence**: Prevents horizon effect without deep search
5. **History heuristic**: Learns from search patterns

## Search Example

```
Position: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
Depth: 4

Iteration 1 (depth 1):
  e2e4: +20
  d2d4: +20
  ... (20 moves)
  Best: e2e4

Iteration 2 (depth 2):
  e2e4 e7e5: +10
  e2e4 c7c5: +15
  ... (400 positions)
  Best: e2e4

Iteration 3 (depth 3):
  ... (8902 positions)
  Best: e2e4

Iteration 4 (depth 4):
  ... (197281 positions)
  Best: e2e4
  Score: +25
```

## Future Enhancements

- **Late Move Reductions (LMR)**: Reduce depth for moves tried late
- **Null Move Pruning**: Skip opponent's turn to detect zugzwang
- **Razoring**: Reduce depth in quiet positions
- **Multi-threading**: Parallel search with shared transposition table
- **Time management**: Allocate time based on game phase
- **Aspiration windows**: Narrow alpha-beta window around expected score

