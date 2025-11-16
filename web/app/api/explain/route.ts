import { NextRequest, NextResponse } from 'next/server'
import axios from 'axios'

const EXPLAIN_URL = process.env.EXPLAIN_URL || 'http://localhost:8001'
const NEURAL_URL = process.env.NEURAL_URL || 'http://localhost:8000'

export async function POST(request: NextRequest) {
  try {
    const body = await request.json()
    const { fen } = body

    if (!fen) {
      return NextResponse.json(
        { error: 'FEN string required' },
        { status: 400 }
      )
    }

    // Get evaluation from neural service
    let fianchetto_eval = 0
    try {
      const neuralResponse = await axios.post(`${NEURAL_URL}/evaluate`, {
        fen,
      })
      fianchetto_eval = neuralResponse.data.score || 0
    } catch (error) {
      console.warn('Neural service not available, using 0 evaluation')
    }

    // Get explanation
    const explainResponse = await axios.post(`${EXPLAIN_URL}/explain`, {
      fen,
      fianchetto_eval,
    })

    return NextResponse.json(explainResponse.data)
  } catch (error: any) {
    console.error('Error in /api/explain:', error)
    return NextResponse.json(
      { error: error.message || 'Internal server error' },
      { status: 500 }
    )
  }
}

