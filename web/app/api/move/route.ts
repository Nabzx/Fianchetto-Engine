import { NextRequest, NextResponse } from 'next/server'
import axios from 'axios'

const ENGINE_URL = process.env.ENGINE_URL || 'http://localhost:8080'

export async function POST(request: NextRequest) {
  try {
    const body = await request.json()
    const { fen, depth = 5 } = body

    if (!fen) {
      return NextResponse.json(
        { error: 'FEN string required' },
        { status: 400 }
      )
    }

    // Call engine service (simplified - in production would use UCI protocol)
    // For now, return a placeholder
    // In a real implementation, you would:
    // 1. Start UCI engine process
    // 2. Send "position fen <fen>"
    // 3. Send "go depth <depth>"
    // 4. Parse "bestmove" response

    // Placeholder: return a random legal move
    const response = await axios.post(`${ENGINE_URL}/move`, {
      fen,
      depth,
    }).catch(() => {
      // Fallback: use chess.js to get a random move
      const { Chess } = require('chess.js')
      const chess = new Chess(fen)
      const moves = chess.moves({ verbose: true })
      if (moves.length === 0) {
        return { data: { move: null } }
      }
      const move = moves[Math.floor(Math.random() * moves.length)]
      return {
        data: {
          move: move.from + move.to + (move.promotion || ''),
        },
      }
    })

    return NextResponse.json(response.data)
  } catch (error: any) {
    console.error('Error in /api/move:', error)
    return NextResponse.json(
      { error: error.message || 'Internal server error' },
      { status: 500 }
    )
  }
}

