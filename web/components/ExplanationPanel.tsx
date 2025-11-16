'use client'

interface ExplanationPanelProps {
  explanation: any
}

export default function ExplanationPanel({ explanation }: ExplanationPanelProps) {
  if (!explanation) {
    return (
      <div className="bg-white rounded-lg shadow-lg p-4">
        <h3 className="text-lg font-semibold mb-2">Explanation</h3>
        <p className="text-gray-400">No explanation available</p>
      </div>
    )
  }

  return (
    <div className="bg-white rounded-lg shadow-lg p-4">
      <h3 className="text-lg font-semibold mb-3">Position Analysis</h3>
      
      <div className="space-y-3">
        {/* Classification */}
        <div>
          <span className="text-sm font-medium text-gray-600">Classification: </span>
          <span
            className={`px-2 py-1 rounded text-sm font-semibold ${
              explanation.classification === 'winning'
                ? 'bg-green-100 text-green-800'
                : explanation.classification === 'losing'
                ? 'bg-red-100 text-red-800'
                : 'bg-gray-100 text-gray-800'
            }`}
          >
            {explanation.classification}
          </span>
        </div>

        {/* Evaluation comparison */}
        <div className="text-sm">
          <div className="flex justify-between">
            <span className="text-gray-600">Fianchetto:</span>
            <span className="font-semibold">
              {explanation.fianchetto_eval > 0 ? '+' : ''}
              {(explanation.fianchetto_eval / 100).toFixed(2)}
            </span>
          </div>
          <div className="flex justify-between">
            <span className="text-gray-600">Stockfish:</span>
            <span className="font-semibold">
              {explanation.stockfish_eval > 0 ? '+' : ''}
              {(explanation.stockfish_eval / 100).toFixed(2)}
            </span>
          </div>
          <div className="flex justify-between border-t pt-1 mt-1">
            <span className="text-gray-600">Delta:</span>
            <span className="font-semibold">
              {explanation.delta_cp > 0 ? '+' : ''}
              {(explanation.delta_cp / 100).toFixed(2)}
            </span>
          </div>
        </div>

        {/* Themes */}
        {explanation.themes && explanation.themes.length > 0 && (
          <div>
            <span className="text-sm font-medium text-gray-600">Themes: </span>
            <div className="flex flex-wrap gap-1 mt-1">
              {explanation.themes.map((theme: string, idx: number) => (
                <span
                  key={idx}
                  className="px-2 py-1 bg-blue-100 text-blue-800 rounded text-xs"
                >
                  {theme}
                </span>
              ))}
            </div>
          </div>
        )}

        {/* Explanation text */}
        <div className="pt-2 border-t">
          <p className="text-sm text-gray-700 leading-relaxed">
            {explanation.explanation}
          </p>
        </div>
      </div>
    </div>
  )
}

