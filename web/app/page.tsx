'use client'

import { useState, useEffect } from 'react'
import { Chessboard } from 'react-chessboard'
import { Chess } from 'chess.js'
import axios from 'axios'
import EvaluationBar from '@/components/EvaluationBar'
import MoveHistory from '@/components/MoveHistory'
import ExplanationPanel from '@/components/ExplanationPanel'

const ENGINE_URL = process.env.NEXT_PUBLIC_ENGINE_URL || 'http://localhost:8080'
const EXPLAIN_URL = process.env.NEXT_PUBLIC_EXPLAIN_URL || 'http://localhost:8001'

export default function Home() {
  const [game, setGame] = useState(new Chess())
  const [boardOrientation, setBoardOrientation] = useState<'white' | 'black'>('white')
  const [moveHistory, setMoveHistory] = useState<string[]>([])
  const [evaluation, setEvaluation] = useState<number>(0)
  const [explanation, setExplanation] = useState<any>(null)
  const [isThinking, setIsThinking] = useState(false)

  useEffect(() => {
    // Get initial evaluation
    updateEvaluation()
  }, [game])

  const updateEvaluation = async () => {
    try {
      const fen = game.fen()
      const response = await axios.post('/api/explain', { fen })
      if (response.data) {
        setExplanation(response.data)
        setEvaluation(response.data.fianchetto_eval || 0)
      }
    } catch (error) {
      console.error('Error getting evaluation:', error)
    }
  }

  const makeMove = async (from: string, to: string, promotion?: string) => {
    try {
      const move = game.move({
        from,
        to,
        promotion: promotion as any,
      })

      if (move) {
        setGame(new Chess(game.fen()))
        setMoveHistory([...moveHistory, move.san])
        await updateEvaluation()
        return true
      }
      return false
    } catch (error) {
      return false
    }
  }

  const onDrop = async (sourceSquare: string, targetSquare: string) => {
    const move = await makeMove(sourceSquare, targetSquare)
    if (move) {
      // Engine's turn
      await engineMove()
    }
    return move
  }

  const engineMove = async () => {
    setIsThinking(true)
    try {
      const response = await axios.post('/api/move', {
        fen: game.fen(),
        depth: 5,
      })

      if (response.data && response.data.move) {
        const move = response.data.move
        await makeMove(move.substring(0, 2), move.substring(2, 4), move.substring(4))
      }
    } catch (error) {
      console.error('Error getting engine move:', error)
    } finally {
      setIsThinking(false)
    }
  }

  const resetGame = () => {
    setGame(new Chess())
    setMoveHistory([])
    setEvaluation(0)
    setExplanation(null)
  }

  const flipBoard = () => {
    setBoardOrientation(boardOrientation === 'white' ? 'black' : 'white')
  }

  return (
    <main className="min-h-screen p-8 bg-gradient-to-br from-gray-100 to-gray-200">
      <div className="max-w-7xl mx-auto">
        <h1 className="text-4xl font-bold text-center mb-8 text-gray-800">
          Fianchetto Engine
        </h1>

        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* Left Column: Board and Controls */}
          <div className="lg:col-span-2 space-y-4">
            <div className="bg-white rounded-lg shadow-lg p-4">
              <div className="flex justify-between items-center mb-4">
                <h2 className="text-xl font-semibold">Chess Board</h2>
                <div className="space-x-2">
                  <button
                    onClick={flipBoard}
                    className="px-4 py-2 bg-blue-500 text-white rounded hover:bg-blue-600"
                  >
                    Flip Board
                  </button>
                  <button
                    onClick={resetGame}
                    className="px-4 py-2 bg-red-500 text-white rounded hover:bg-red-600"
                  >
                    New Game
                  </button>
                </div>
              </div>
              
              {isThinking && (
                <div className="mb-4 text-center text-gray-600">
                  Engine is thinking...
                </div>
              )}

              <div className="flex justify-center">
                <Chessboard
                  position={game.fen()}
                  onPieceDrop={onDrop}
                  boardOrientation={boardOrientation}
                  customBoardStyle={{
                    borderRadius: '4px',
                    boxShadow: '0 4px 6px rgba(0, 0, 0, 0.1)',
                  }}
                />
              </div>
            </div>

            {/* Evaluation Bar */}
            <EvaluationBar evaluation={evaluation} />
          </div>

          {/* Right Column: Move History and Explanation */}
          <div className="space-y-4">
            <MoveHistory moves={moveHistory} />
            <ExplanationPanel explanation={explanation} />
          </div>
        </div>
      </div>
    </main>
  )
}

