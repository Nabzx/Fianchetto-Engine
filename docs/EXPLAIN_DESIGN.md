# Explainability Service Design

## Overview

The explainability service provides human-readable explanations for chess positions by combining rule-based feature extraction, Stockfish comparison, and template-based natural language generation.

## Architecture

### Components

1. **Stockfish Client** (`stockfish_client.py`)
   - Interface to Stockfish engine
   - Position evaluation
   - Best move analysis

2. **Feature Extraction** (`features.py`)
   - Material balance
   - Center control
   - King safety
   - Pawn structure
   - Piece activity

3. **Explanation Engine** (`explanation_engine.py`)
   - Combines features and evaluations
   - Identifies themes
   - Generates explanations

4. **Templates** (`templates.py`)
   - Natural language templates
   - Theme descriptions

5. **API Server** (`api.py`)
   - FastAPI endpoint
   - `/explain` route

## Feature Extraction

### Material Balance

Compares material values:

```python
piece_values = {
    chess.PAWN: 100,
    chess.KNIGHT: 320,
    chess.BISHOP: 330,
    chess.ROOK: 500,
    chess.QUEEN: 900,
    chess.KING: 20000,
}

white_material = sum(values for white pieces)
black_material = sum(values for black pieces)
material_balance = white_material - black_material
```

### Center Control

Counts pieces on central squares (e4, e5, d4, d5):

```python
center_squares = [chess.E4, chess.E5, chess.D4, chess.D5]
white_center = sum(1 for sq in center_squares if white_piece)
black_center = sum(1 for sq in center_squares if black_piece)
center_control = white_center - black_center
```

### King Safety

Measures king distance from center (simplified):

```python
king_center_dist = abs(king_rank - 3.5) + abs(king_file - 3.5)
```

Kings closer to the center are generally safer in the opening/middlegame.

### Pawn Structure

#### Doubled Pawns

Pawns on the same file:

```python
def count_doubled_pawns(pawns):
    files = [chess.square_file(p) for p in pawns]
    file_counts = {}
    for f in files:
        file_counts[f] = file_counts.get(f, 0) + 1
    return sum(max(0, count - 1) for count in file_counts.values())
```

#### Isolated Pawns

Pawns with no friendly pawns on adjacent files:

```python
def count_isolated_pawns(pawns):
    files = set(chess.square_file(p) for p in pawns)
    isolated = 0
    for pawn in pawns:
        file = chess.square_file(pawn)
        has_neighbor = (file - 1 in files) or (file + 1 in files)
        if not has_neighbor:
            isolated += 1
    return isolated
```

### Open Files

Files with no pawns (good for rooks):

```python
def count_open_files(board, color):
    all_pawns = white_pawns + black_pawns
    occupied_files = set(chess.square_file(p) for p in all_pawns)
    return 8 - len(occupied_files)
```

### Piece Activity

Number of legal moves (simplified measure of activity):

```python
activity = len(list(board.legal_moves))
```

## Stockfish Integration

### Evaluation

The service uses Stockfish to get a reference evaluation:

```python
info = engine.analyse(board, chess.engine.Limit(depth=10))
score = info["score"].white()

if score.is_mate():
    cp = 30000 if score.mate() > 0 else -30000
else:
    cp = score.score()
```

### Comparison

The service compares Fianchetto's evaluation with Stockfish:

```python
delta_cp = fianchetto_eval - stockfish_eval
```

This delta indicates how well Fianchetto's evaluation matches Stockfish.

## Theme Identification

Themes are identified from features:

```python
def _identify_themes(self, features, delta_cp):
    themes = []
    
    if abs(material_balance) > 300:
        themes.append("Material imbalance")
    
    if abs(center_control) > 1:
        themes.append("Center control")
    
    if abs(king_safety) > 2.0:
        themes.append("King safety")
    
    # ... more themes
    
    return themes
```

## Explanation Generation

### Template-Based

Explanations are built from templates:

```python
def generate_explanation(delta_cp, features, stockfish_eval, themes):
    parts = []
    
    # Material
    if abs(material_balance) > 200:
        parts.append(f"White has a material advantage ({material_balance} centipawns).")
    
    # Center control
    if abs(center_control) > 0:
        parts.append(f"{side} controls more center squares.")
    
    # ... more features
    
    # Overall evaluation
    if abs(delta_cp) < 50:
        parts.append("The position is roughly equal.")
    
    return " ".join(parts)
```

### Classification

Positions are classified as:
- **Winning**: Evaluation > 200 centipawns
- **Equal**: Evaluation between -200 and 200
- **Losing**: Evaluation < -200 centipawns

## API Endpoint

### Request

```json
{
  "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "fianchetto_eval": 25
}
```

### Response

```json
{
  "classification": "equal",
  "delta_cp": -5,
  "themes": ["Center control", "Piece activity"],
  "explanation": "The position is roughly equal. White controls more center squares. White has more active pieces.",
  "stockfish_eval": 30,
  "fianchetto_eval": 25
}
```

## Use Cases

1. **Engine Development**: Compare Fianchetto's evaluation with Stockfish
2. **User Education**: Explain why a position is good/bad
3. **Debugging**: Identify evaluation weaknesses
4. **Analysis**: Understand position characteristics

## Future Improvements

- **Tactical patterns**: Detect forks, pins, skewers
- **Strategic plans**: Identify long-term plans
- **Move explanations**: Explain why a move is good
- **Visual highlights**: Highlight important squares/pieces
- **Comparison modes**: Compare multiple engines
- **Learning from games**: Improve explanations from game outcomes

