# Frontend Specification

## Overview

The Fianchetto Engine frontend is a Next.js 14 application with TypeScript, Tailwind CSS, and React Chessboard integration. It provides an interactive chess interface with evaluation visualization and position explanations.

## Technology Stack

- **Framework**: Next.js 14 (App Router)
- **Language**: TypeScript
- **Styling**: Tailwind CSS
- **Chess Library**: chess.js
- **Chessboard**: react-chessboard
- **HTTP Client**: axios

## Project Structure

```
web/
  app/
    api/
      move/
        route.ts          # API route for engine moves
      explain/
        route.ts          # API route for explanations
    globals.css           # Global styles
    layout.tsx            # Root layout
    page.tsx              # Main page component
  components/
    EvaluationBar.tsx     # Evaluation visualization
    MoveHistory.tsx       # Move list display
    ExplanationPanel.tsx  # Position analysis panel
  package.json
  tsconfig.json
  next.config.js
  tailwind.config.js
```

## Components

### Main Page (`app/page.tsx`)

The main page orchestrates the chess interface:

**State:**
- `game`: Chess.js game instance
- `boardOrientation`: 'white' | 'black'
- `moveHistory`: Array of move strings (SAN)
- `evaluation`: Current position evaluation (centipawns)
- `explanation`: Explanation object from API
- `isThinking`: Whether engine is calculating

**Functions:**
- `makeMove()`: Make a move on the board
- `onDrop()`: Handle piece drop from chessboard
- `engineMove()`: Get and play engine move
- `updateEvaluation()`: Fetch position evaluation and explanation
- `resetGame()`: Start new game
- `flipBoard()`: Flip board orientation

### Evaluation Bar (`components/EvaluationBar.tsx`)

Visualizes position evaluation as a vertical bar.

**Props:**
- `evaluation`: Centipawn score (positive = white better)

**Visualization:**
- Vertical bar (height proportional to evaluation)
- White section (top) for positive evaluations
- Black section (bottom) for negative evaluations
- Center line at 0
- Evaluation text overlay

### Move History (`components/MoveHistory.tsx`)

Displays move history in a scrollable list.

**Props:**
- `moves`: Array of move strings (SAN)

**Layout:**
- Move pairs (white, black) per row
- Move numbers (1., 2., ...)
- Scrollable container (max height)

### Explanation Panel (`components/ExplanationPanel.tsx`)

Shows position analysis and explanation.

**Props:**
- `explanation`: Explanation object from API

**Displays:**
- Classification badge (winning/equal/losing)
- Fianchetto evaluation
- Stockfish evaluation
- Delta (difference)
- Themes (badges)
- Natural language explanation

## API Routes

### `/api/move`

Proxies move requests to engine service.

**Implementation:**
```typescript
export async function POST(request: NextRequest) {
  const { fen, depth } = await request.json()
  // Call engine service
  // Return best move
}
```

**Note:** In production, this would interface with the UCI engine via a wrapper service.

### `/api/explain`

Proxies explanation requests to explainability service.

**Implementation:**
```typescript
export async function POST(request: NextRequest) {
  const { fen } = await request.json()
  // Get evaluation from neural service
  // Get explanation from explain service
  // Return combined result
}
```

## Styling

### Tailwind Configuration

Custom colors:
- `chess-light`: #f0d9b5
- `chess-dark`: #b58863

### Layout

- Responsive grid layout (3 columns on large screens)
- Board and controls on left (2 columns)
- Move history and explanation on right (1 column)
- Mobile: Single column stack

### Components

- **Cards**: White background, rounded corners, shadow
- **Buttons**: Colored backgrounds, hover effects
- **Badges**: Small colored pills for themes/classification

## User Interactions

### Making Moves

1. User drags piece on chessboard
2. `onDrop()` handler called
3. Move validated with chess.js
4. If valid, move made and engine move requested
5. Evaluation updated

### Engine Moves

1. User makes move
2. `engineMove()` called
3. POST to `/api/move`
4. Engine calculates best move
5. Move played automatically
6. Evaluation updated

### Board Controls

- **Flip Board**: Toggles orientation
- **New Game**: Resets to starting position

## State Management

Currently using React `useState` hooks. For more complex state:

- Consider Zustand or Redux
- Move game state to context
- Cache evaluations

## Performance

### Optimizations

- **Memoization**: Memoize expensive calculations
- **Debouncing**: Debounce evaluation requests
- **Caching**: Cache evaluations by FEN
- **Lazy loading**: Lazy load components

### Future Improvements

- WebSocket for real-time updates
- Service Worker for offline support
- IndexedDB for move history storage
- WebAssembly for client-side evaluation

## Responsive Design

### Breakpoints

- Mobile: < 640px (single column)
- Tablet: 640px - 1024px (2 columns)
- Desktop: > 1024px (3 columns)

### Mobile Considerations

- Stack components vertically
- Reduce board size
- Collapsible panels
- Touch-friendly controls

## Accessibility

### ARIA Labels

- Board: `aria-label="Chess board"`
- Buttons: Descriptive labels
- Evaluation: `aria-label="Position evaluation"`

### Keyboard Navigation

- Tab through controls
- Enter/Space to activate buttons
- Arrow keys for move navigation (future)

## Error Handling

### Network Errors

- Display error messages
- Retry failed requests
- Fallback to local evaluation

### Invalid Moves

- Highlight invalid squares
- Show error message
- Prevent move execution

## Testing

### Unit Tests

- Component rendering
- Move validation
- State updates

### Integration Tests

- API route handling
- Move flow
- Evaluation updates

### E2E Tests

- Full game flow
- Engine interaction
- UI interactions

## Deployment

### Build

```bash
npm run build
```

### Environment Variables

- `NEXT_PUBLIC_ENGINE_URL`: Engine service URL
- `NEXT_PUBLIC_NEURAL_URL`: Neural service URL
- `NEXT_PUBLIC_EXPLAIN_URL`: Explainability service URL

### Docker

See `docker/Dockerfile.web` for containerization.

## Future Enhancements

- **Analysis Mode**: Step through game with evaluations
- **Opening Book**: Show opening names
- **Endgame Tablebase**: Show tablebase results
- **Puzzle Mode**: Solve tactical puzzles
- **Game Replay**: Replay saved games
- **Multiplayer**: Play against other users
- **Engine Settings**: Adjust search depth, time limits

