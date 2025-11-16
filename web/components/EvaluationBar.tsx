'use client'

interface EvaluationBarProps {
  evaluation: number
}

export default function EvaluationBar({ evaluation }: EvaluationBarProps) {
  // Convert centipawns to a percentage for display
  // Clamp to reasonable range (-1000 to +1000 centipawns = -10 to +10 pawns)
  const clampedEval = Math.max(-1000, Math.min(1000, evaluation))
  const percentage = ((clampedEval + 1000) / 2000) * 100

  // Determine if white or black is better
  const isWhiteBetter = evaluation > 0

  return (
    <div className="bg-white rounded-lg shadow-lg p-4">
      <h3 className="text-lg font-semibold mb-2">Evaluation</h3>
      <div className="relative h-64 bg-gray-200 rounded overflow-hidden">
        {/* Evaluation bar */}
        <div
          className={`absolute bottom-0 w-full transition-all duration-300 ${
            isWhiteBetter ? 'bg-white' : 'bg-black'
          }`}
          style={{
            height: `${percentage}%`,
          }}
        />
        
        {/* Center line */}
        <div className="absolute top-1/2 left-0 right-0 h-0.5 bg-gray-400" />
        
        {/* Evaluation text */}
        <div className="absolute inset-0 flex items-center justify-center">
          <span className="text-lg font-bold text-gray-700">
            {evaluation > 0 ? '+' : ''}
            {(evaluation / 100).toFixed(2)}
          </span>
        </div>
      </div>
      <div className="mt-2 flex justify-between text-sm text-gray-600">
        <span>Black</span>
        <span>White</span>
      </div>
    </div>
  )
}

