# API Specification

## Overview

The Fianchetto Engine system exposes several HTTP APIs for interaction:

1. **Neural Service** (Port 8000)
2. **Explainability Service** (Port 8001)
3. **Web Frontend** (Port 3000) - with API routes

## Neural Service API

### Base URL

```
http://neural:8000
```

### Endpoints

#### GET `/`

Health check endpoint.

**Response:**
```json
{
  "message": "Fianchetto Neural Evaluation Service"
}
```

#### GET `/health`

Service health status.

**Response:**
```json
{
  "status": "healthy",
  "model_loaded": true
}
```

#### POST `/evaluate`

Evaluate a chess position.

**Request Body:**
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

**Response Fields:**
- `score` (int): Centipawn evaluation (positive = white better)

**Error Responses:**
- `400`: Invalid FEN string
- `503`: Model not loaded

## Explainability Service API

### Base URL

```
http://explain:8001
```

### Endpoints

#### GET `/`

Health check endpoint.

**Response:**
```json
{
  "message": "Fianchetto Explainability Service"
}
```

#### GET `/health`

Service health status.

**Response:**
```json
{
  "status": "healthy",
  "engine_loaded": true
}
```

#### POST `/explain`

Generate explanation for a position.

**Request Body:**
```json
{
  "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "fianchetto_eval": 25
}
```

**Request Fields:**
- `fen` (string, required): FEN string of position
- `fianchetto_eval` (int, optional): Fianchetto engine evaluation in centipawns

**Response:**
```json
{
  "classification": "equal",
  "delta_cp": -5,
  "themes": ["Center control", "Piece activity"],
  "explanation": "The position is roughly equal. White controls more center squares.",
  "stockfish_eval": 30,
  "fianchetto_eval": 25
}
```

**Response Fields:**
- `classification` (string): "winning", "equal", or "losing"
- `delta_cp` (int): Difference between Fianchetto and Stockfish evaluation
- `themes` (array): List of identified strategic themes
- `explanation` (string): Natural language explanation
- `stockfish_eval` (int): Stockfish evaluation in centipawns
- `fianchetto_eval` (int): Fianchetto evaluation in centipawns

**Error Responses:**
- `400`: Invalid FEN string or missing required fields
- `503`: Explanation engine not loaded

## Web Frontend API Routes

### Base URL

```
http://web:3000
```

### Endpoints

#### POST `/api/move`

Get engine move recommendation.

**Request Body:**
```json
{
  "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "depth": 5
}
```

**Request Fields:**
- `fen` (string, required): FEN string of position
- `depth` (int, optional): Search depth (default: 5)

**Response:**
```json
{
  "move": "e2e4"
}
```

**Response Fields:**
- `move` (string): UCI format move (e.g., "e2e4", "e7e5q" for promotion)

**Error Responses:**
- `400`: Invalid FEN string
- `500`: Internal server error

#### POST `/api/explain`

Get position explanation (proxies to explainability service).

**Request Body:**
```json
{
  "fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
}
```

**Request Fields:**
- `fen` (string, required): FEN string of position

**Response:**
Same as explainability service `/explain` endpoint.

**Error Responses:**
- `400`: Invalid FEN string
- `500`: Internal server error

## UCI Protocol (Engine)

The C++ engine also supports the Universal Chess Interface (UCI) protocol via stdin/stdout.

### Commands

#### `uci`

Identify engine.

**Response:**
```
id name Fianchetto Engine
id author Fianchetto Team
uciok
```

#### `isready`

Check if engine is ready.

**Response:**
```
readyok
```

#### `ucinewgame`

Start a new game.

#### `position [fen <fen>|startpos] [moves <move1> <move2> ...]`

Set position.

**Examples:**
```
position startpos
position startpos moves e2e4 e7e5
position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
```

#### `go depth <n>`

Search to depth n.

**Response:**
```
bestmove e2e4
```

#### `stop`

Stop current search.

#### `quit`

Exit engine.

## Error Handling

All HTTP APIs return standard HTTP status codes:

- `200`: Success
- `400`: Bad request (invalid input)
- `500`: Internal server error
- `503`: Service unavailable

Error responses include a JSON body:

```json
{
  "error": "Error message"
}
```

## Rate Limiting

Currently, no rate limiting is implemented. In production, consider:

- Per-IP rate limits
- Per-endpoint limits
- Request queuing for expensive operations

## Authentication

Currently, no authentication is required. In production:

- API keys for external access
- JWT tokens for user sessions
- Rate limiting per user

## CORS

The web frontend API routes should allow CORS from the frontend domain. Configure in `next.config.js`:

```javascript
headers: async () => [
  {
    source: '/api/:path*',
    headers: [
      { key: 'Access-Control-Allow-Origin', value: '*' },
    ],
  },
],
```

## Versioning

API versioning is not currently implemented. For future versions:

- URL versioning: `/api/v1/move`
- Header versioning: `API-Version: 1.0`
- Query parameter: `?version=1.0`

