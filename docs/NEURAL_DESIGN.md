# Neural Network Design

## Overview

The Fianchetto Engine includes an optional neural network evaluation service that can replace or supplement the classical evaluation function. The neural service is implemented in Python using PyTorch and provides a FastAPI HTTP interface.

## Architecture

### Components

1. **Board Encoding** (`encode.py`)
   - Converts FEN strings to tensor representations
   - 12-plane input (6 piece types × 2 colors)
   - Each plane is 8×8

2. **Model Architecture** (`model.py`)
   - CNN (Convolutional Neural Network) variant
   - MLP (Multi-Layer Perceptron) variant
   - Output: Single scalar (centipawn score)

3. **Training** (`train.py`)
   - Training loop with validation
   - Model checkpointing
   - Synthetic dataset generation

4. **Inference API** (`api.py`)
   - FastAPI server
   - `/evaluate` endpoint
   - Model loading and caching

## Board Encoding

### 12-Plane Representation

Each position is encoded as 12 binary planes (8×8 each):

```
Plane 0:  White Pawns
Plane 1:  White Knights
Plane 2:  White Bishops
Plane 3:  White Rooks
Plane 4:  White Queens
Plane 5:  White King
Plane 6:  Black Pawns
Plane 7:  Black Knights
Plane 8:  Black Bishops
Plane 9:  Black Rooks
Plane 10: Black Queens
Plane 11: Black King
```

Each square is represented as 1.0 if the piece is present, 0.0 otherwise.

### Implementation

```python
def fen_to_planes(fen: str) -> np.ndarray:
    board = chess.Board(fen)
    planes = np.zeros((12, 8, 8), dtype=np.float32)
    
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            color_offset = 0 if piece.color == chess.WHITE else 6
            plane_idx = color_offset + piece_to_plane[piece.piece_type]
            rank = chess.square_rank(square)
            file = chess.square_file(square)
            planes[plane_idx, 7 - rank, file] = 1.0
    
    return planes
```

## Model Architectures

### CNN Model

The CNN model uses convolutional layers to detect patterns:

```python
class ChessEvalModel(nn.Module):
    def __init__(self, hidden_dim=256):
        # Convolutional layers
        self.conv1 = nn.Conv2d(12, 64, kernel_size=3, padding=1)
        self.conv2 = nn.Conv2d(64, 128, kernel_size=3, padding=1)
        self.conv3 = nn.Conv2d(128, 128, kernel_size=3, padding=1)
        
        # Fully connected layers
        self.fc1 = nn.Linear(128 * 8 * 8, hidden_dim)
        self.fc2 = nn.Linear(hidden_dim, hidden_dim)
        self.fc3 = nn.Linear(hidden_dim, 1)
```

**Architecture:**
- Input: (batch, 12, 8, 8)
- Conv layers extract spatial patterns
- FC layers combine features
- Output: (batch,) - centipawn score

### MLP Model

Simpler fully-connected model:

```python
class SimpleMLP(nn.Module):
    def __init__(self, hidden_dim=512):
        self.fc1 = nn.Linear(12 * 8 * 8, hidden_dim)
        self.fc2 = nn.Linear(hidden_dim, hidden_dim)
        self.fc3 = nn.Linear(hidden_dim, hidden_dim // 2)
        self.fc4 = nn.Linear(hidden_dim // 2, 1)
```

**Trade-offs:**
- Faster inference
- Less accurate
- Good for testing

## Training

### Dataset

The training script generates synthetic positions:

```python
def generate_synthetic_dataset(num_positions: int = 10000):
    positions = []
    evaluations = []
    
    board = chess.Board()
    for _ in range(num_positions):
        # Make random moves
        # Evaluate with simple material + PST
        # Store (fen, evaluation)
```

**In Production:**
- Use real game databases (PGN files)
- Use engine evaluations (Stockfish, Leela)
- Use self-play data

### Training Loop

```python
for epoch in range(epochs):
    for batch in train_loader:
        outputs = model(planes)
        loss = criterion(outputs, targets)
        loss.backward()
        optimizer.step()
    
    val_loss = validate(model, val_loader)
    if val_loss < best_val_loss:
        save_checkpoint(model)
```

### Loss Function

Mean Squared Error (MSE) for regression:

```python
criterion = nn.MSELoss()
```

## Inference API

### Endpoint

```python
@app.post("/evaluate")
async def evaluate(request: EvaluateRequest):
    planes = fen_to_planes(request.fen)
    input_tensor = torch.from_numpy(planes).unsqueeze(0)
    
    with torch.no_grad():
        output = model(input_tensor)
        score = int(output.item() * 100)  # Convert to centipawns
    
    return {"score": score}
```

### Request/Response

**Request:**
```json
{
  "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
}
```

**Response:**
```json
{
  "score": 25
}
```

## Integration with Engine

The C++ engine can call the neural service via HTTP:

```cpp
#ifdef USE_NEURAL
NeuralClient neural_client("http://neural:8000/evaluate");
int score = neural_client.evaluate(board);
#endif
```

The client:
1. Converts board to FEN
2. Sends HTTP POST request
3. Parses JSON response
4. Caches results by position hash

## ONNX Export

For faster inference, models can be exported to ONNX:

```python
torch.onnx.export(
    model,
    dummy_input,
    "model.onnx",
    input_names=["input"],
    output_names=["output"],
)
```

ONNX models can be loaded with `onnxruntime` for faster inference.

## Performance Considerations

- **Batch inference**: Process multiple positions at once
- **Model quantization**: Reduce precision for speed
- **ONNX runtime**: Faster than PyTorch for inference
- **Caching**: Cache evaluations in engine
- **GPU acceleration**: Use CUDA for training and inference

## Future Improvements

- **Residual networks**: Deeper models with skip connections
- **Attention mechanisms**: Focus on important squares
- **Transformer architecture**: Self-attention for position understanding
- **Self-play training**: Generate training data from engine games
- **Ensemble models**: Combine multiple models
- **Temporal features**: Include move history
- **Endgame specialization**: Separate models for endgames

