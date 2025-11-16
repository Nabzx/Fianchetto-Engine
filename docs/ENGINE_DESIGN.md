# Fianchetto Engine Design

## Overview

The Fianchetto Engine is a hybrid chess engine combining classical search algorithms with optional neural network evaluation. The engine is written in C++20 and implements a complete chess engine with bitboard representation, move generation, search, and UCI protocol support.

## Architecture

### Core Components

1. **Board Representation** (`board.hpp`, `board.cpp`)
   - Bitboard-based position representation
   - Zobrist hashing for transposition tables
   - FEN parsing and export
   - Move make/unmake with history

2. **Move Generation** (`movegen.hpp`, `movegen.cpp`)
   - Legal move generation for all piece types
   - Attack generation for check detection
   - Perft testing support

3. **Search Algorithm** (`search.hpp`, `search.cpp`)
   - Negamax with alpha-beta pruning
   - Iterative deepening
   - Transposition table
   - Move ordering (hash move, MVV-LVA, killers, history)
   - Quiescence search

4. **Evaluation** (`search.cpp`)
   - Material counting
   - Piece-square tables (PSTs)
   - Pawn structure evaluation
   - Optional neural network integration

5. **UCI Protocol** (`uci_main.cpp`)
   - Full UCI command support
   - Position setting
   - Search commands
   - Best move output

## Board Representation

### Bitboards

The engine uses bitboards (64-bit integers) to represent piece positions. Each piece type and color combination has its own bitboard:

```cpp
std::array<std::array<Bitboard, 7>, 2> bitboards_;
// [color][piece_type]
```

### Zobrist Hashing

Zobrist hashing is used for transposition table lookups. The hash key is computed from:
- Piece positions (12 piece types × 64 squares)
- Castling rights (4 combinations)
- En passant square (8 files)
- Side to move

## Move Generation

### Piece-Specific Generators

- **Pawns**: Single/double pushes, captures, promotions, en passant
- **Knights**: 8-square jumps
- **Bishops**: Diagonal sliding
- **Rooks**: Orthogonal sliding
- **Queens**: Combined bishop + rook
- **King**: 8-square moves + castling

### Legality Checking

Moves are checked for legality by:
1. Making the move on a temporary board
2. Checking if the king is in check
3. Unmaking the move

## Search Algorithm

### Negamax with Alpha-Beta

The search uses a negamax framework with alpha-beta pruning:

```cpp
int negamax(Board& board, int depth, int alpha, int beta, ...)
```

### Move Ordering

Moves are ordered by:
1. **Hash move**: Best move from transposition table
2. **Captures**: Sorted by MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
3. **Killer moves**: Moves that caused beta cutoffs at the same depth
4. **History heuristic**: Moves that performed well historically

### Transposition Table

The transposition table stores:
- Position hash
- Search depth
- Evaluation score
- Best move
- Node type (exact, lower bound, upper bound)

### Quiescence Search

After reaching depth 0, quiescence search continues with capture-only moves to avoid horizon effects.

## Evaluation Function

### Material

Piece values (centipawns):
- Pawn: 100
- Knight: 320
- Bishop: 330
- Rook: 500
- Queen: 900
- King: 20000

### Piece-Square Tables

Each piece type has a PST that rewards centralization and good positioning.

### Pawn Structure

- Doubled pawns penalty
- Isolated pawns penalty (simplified)

### Neural Integration

When `USE_NEURAL` is enabled, the engine can call the neural service via HTTP:

```cpp
#ifdef USE_NEURAL
NeuralClient neural_client("http://neural:8000/evaluate");
int score = neural_client.evaluate(board);
#endif
```

## UCI Protocol

The engine implements the Universal Chess Interface (UCI) protocol:

- `uci`: Identify engine
- `isready`: Check readiness
- `ucinewgame`: Start new game
- `position [fen <fen>|startpos] moves <move1> <move2> ...`: Set position
- `go depth <n>`: Search to depth n
- `stop`: Stop search
- `quit`: Exit

## Build Configuration

### CMake Options

- `USE_NEURAL`: Enable neural network HTTP integration (requires libcurl)

### Compilation

```bash
mkdir build && cd build
cmake .. -DUSE_NEURAL=ON
make -j$(nproc)
```

## Testing

### Perft Testing

The engine includes perft (perft = performance test) functionality:

```bash
./fianchetto_perft 4
```

This counts the number of legal positions at depth 4, useful for validating move generation correctness.

## Performance Considerations

- Bitboard operations use compiler intrinsics (`__builtin_ctzll`, `__builtin_popcountll`)
- Transposition table uses modulo hashing
- Move ordering significantly improves alpha-beta efficiency
- History heuristic learns from search patterns

## Future Improvements

- Magic bitboards for sliding pieces
- Late move reductions (LMR)
- Null move pruning
- Endgame tablebases
- Multi-threaded search
- Time management
- Opening book

