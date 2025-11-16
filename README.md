Fianchetto Engine

A hybrid classical + neural chess engine with a C++ core, Python neural evaluation service, rule-based explainability engine, and modern Next.js frontend.

## Features

- **Classical Chess Engine**: Bitboard-based C++ engine with alpha-beta search
- **Neural Evaluation**: Optional PyTorch-based neural network evaluation
- **Explainability**: Rule-based position analysis with Stockfish comparison
- **Modern Frontend**: Next.js 14 with interactive chessboard and evaluation visualization
- **Docker Support**: Full containerization with docker-compose

## Project Structure

```
fianchetto-engine/
├── engine/          # C++ chess engine
├── neural/          # Python neural service
├── explain/          # Python explainability service
├── web/              # Next.js frontend
├── docker/           # Docker configurations
└── docs/             # Documentation
```

## Building

### C++ Engine

```bash
cd engine
mkdir build && cd build
cmake .. -DUSE_NEURAL=ON  # Optional: enable neural integration
make -j$(nproc)
```

### Python Services

```bash
# Neural service
cd neural
pip install -r requirements.txt

# Explainability service
cd ../explain
pip install -r requirements.txt
```

### Web Frontend

```bash
cd web
npm install
npm run dev
```

## Docker

Build and run all services:

```bash
cd docker
docker-compose up --build
```

Services will be available at:
- Web: http://localhost:3000
- Neural API: http://localhost:8000
- Explain API: http://localhost:8001

## Documentation

- [Engine Design](docs/ENGINE_DESIGN.md) - C++ engine architecture
- [Search Algorithm](docs/SEARCH_ALGO.md) - Search implementation details
- [Neural Design](docs/NEURAL_DESIGN.md) - Neural network architecture
- [Explainability Design](docs/EXPLAIN_DESIGN.md) - Explanation engine
- [API Specification](docs/API_SPEC.md) - HTTP API documentation
- [Frontend Specification](docs/FRONTEND_SPEC.md) - Web UI documentation

## Usage

### UCI Engine

```bash
./engine/build/fianchetto_uci
```

Then use UCI commands:
```
uci
isready
position startpos
go depth 5
```

### Perft Testing

```bash
./engine/build/fianchetto_perft 4
```

### Neural Service

```bash
cd neural/src
python api.py
```

### Explainability Service

```bash
cd explain/src
python api.py
```

## Configuration

### Environment Variables

**Web Frontend:**
- `NEXT_PUBLIC_ENGINE_URL`: Engine service URL
- `NEXT_PUBLIC_NEURAL_URL`: Neural service URL
- `NEXT_PUBLIC_EXPLAIN_URL`: Explainability service URL

**Neural Service:**
- `MODEL_PATH`: Path to trained model file

**Explainability Service:**
- `STOCKFISH_PATH`: Path to Stockfish executable

## License

This project is open-source and free to use.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📧 Contact

For questions or issues, please open an issue on GitHub.

