'use client'

interface MoveHistoryProps {
  moves: string[]
}

export default function MoveHistory({ moves }: MoveHistoryProps) {
  // Pair moves (white, black)
  const pairedMoves: Array<[string?, string?]> = []
  for (let i = 0; i < moves.length; i += 2) {
    pairedMoves.push([moves[i], moves[i + 1]])
  }

  return (
    <div className="bg-white rounded-lg shadow-lg p-4">
      <h3 className="text-lg font-semibold mb-3">Move History</h3>
      <div className="max-h-64 overflow-y-auto">
        <div className="space-y-1">
          {pairedMoves.map((pair, idx) => (
            <div key={idx} className="flex items-center space-x-2 text-sm">
              <span className="text-gray-500 w-8">{idx + 1}.</span>
              <span className="px-2 py-1 bg-gray-100 rounded w-20 text-center">
                {pair[0] || '-'}
              </span>
              <span className="px-2 py-1 bg-gray-100 rounded w-20 text-center">
                {pair[1] || '-'}
              </span>
            </div>
          ))}
          {moves.length === 0 && (
            <div className="text-gray-400 text-center py-4">No moves yet</div>
          )}
        </div>
      </div>
    </div>
  )
}

