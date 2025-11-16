# Quick Start Guide

## Prerequisites

- **C++ Compiler**: GCC 10+ or Clang 12+ with C++20 support
- **CMake**: 3.20+
- **Python**: 3.11+
- **Node.js**: 20+
- **Docker** (optional): For containerized deployment
- **Stockfish** (optional): For explainability service

## Local Development Setup

### 1. Build C++ Engine

```bash
cd engine
mkdir build && cd build
cmake ..
make -j$(nproc)

# Test perft
./fianchetto_perft 3

# Run UCI engine
./fianchetto_uci
```

### 2. Setup Neural Service

```bash
cd neural
pip install -r requirements.txt

# Train a model (optional)
python src/train.py --epochs 5 --num-positions 1000

# Run API server
python src/api.py
```

The neural service will be available at http://localhost:8000

### 3. Setup Explainability Service

```bash
cd explain
pip install -r requirements.txt

# Install Stockfish (if not already installed)
# Ubuntu/Debian: sudo apt-get install stockfish
# macOS: brew install stockfish

# Run API server
python src/api.py
```

The explainability service will be available at http://localhost:8001

### 4. Setup Web Frontend

```bash
cd web
npm install
npm run dev
```

The web frontend will be available at http://localhost:3000

## Docker Setup

### Build and Run All Services

```bash
cd docker
docker-compose up --build
```

This will:
1. Build all Docker images
2. Start all services
3. Create a shared network

### Access Services

- Web UI: http://localhost:3000
- Neural API: http://localhost:8000
- Explain API: http://localhost:8001

### Stop Services

```bash
docker-compose down
```

## Testing

### Engine Perft Tests

```bash
cd engine/build
./fianchetto_perft 1  # Should output 20
./fianchetto_perft 2  # Should output 400
./fianchetto_perft 3  # Should output 8902
```

### Neural Service

```bash
curl -X POST http://localhost:8000/evaluate \
  -H "Content-Type: application/json" \
  -d '{"fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"}'
```

### Explainability Service

```bash
curl -X POST http://localhost:8001/explain \
  -H "Content-Type: application/json" \
  -d '{"fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "fianchetto_eval": 25}'
```

## Troubleshooting

### Engine Won't Build

- Check C++20 support: `g++ --version` or `clang++ --version`
- Ensure CMake 3.20+: `cmake --version`
- Check for missing dependencies (libcurl for neural support)

### Neural Service Errors

- Ensure PyTorch is installed: `pip install torch`
- Check model path in environment variable `MODEL_PATH`
- If model doesn't exist, the service will create a dummy model

### Explainability Service Errors

- Ensure Stockfish is installed and in PATH
- Set `STOCKFISH_PATH` environment variable if needed
- Check python-chess installation

### Web Frontend Issues

- Clear `.next` directory: `rm -rf .next`
- Reinstall dependencies: `rm -rf node_modules && npm install`
- Check environment variables in `next.config.js`

## Next Steps

1. **Train Neural Model**: Use real game data to train a better evaluation model
2. **Tune Engine**: Adjust search parameters and evaluation weights
3. **Add Features**: Implement additional UCI commands, time management, etc.
4. **Improve UI**: Add analysis mode, move suggestions, etc.

## Getting Help

- Check documentation in `docs/` directory
- Review code comments
- Open an issue on GitHub

