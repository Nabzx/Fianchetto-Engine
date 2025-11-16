"""
Main explanation engine that combines features, Stockfish, and templates.
"""

import chess
from typing import Dict, List, Tuple
from stockfish_client import StockfishClient
from features import extract_features
from templates import generate_explanation


class ExplanationEngine:
    """
    Generates explanations for chess positions.
    """
    
    def __init__(self, stockfish_path: str = None):
        self.stockfish = StockfishClient(stockfish_path)
        self.stockfish.start()
    
    def explain(
        self,
        fen: str,
        fianchetto_eval: int = 0
    ) -> Dict:
        """
        Generate explanation for a position.
        
        Args:
            fen: FEN string of the position
            fianchetto_eval: Evaluation from Fianchetto engine (centipawns)
        
        Returns:
            Dictionary with:
                - classification: "winning", "equal", "losing"
                - delta_cp: Difference in evaluation
                - themes: List of identified themes
                - explanation: Natural language explanation
        """
        board = chess.Board(fen)
        
        # Get Stockfish evaluation
        stockfish_cp, best_move = self.stockfish.evaluate(board, depth=10)
        
        # Extract features
        features = extract_features(board)
        
        # Calculate delta
        delta_cp = fianchetto_eval - stockfish_cp
        
        # Identify themes
        themes = self._identify_themes(features, delta_cp)
        
        # Classify position
        classification = self._classify_position(stockfish_cp)
        
        # Generate explanation
        explanation = generate_explanation(
            delta_cp,
            features,
            stockfish_cp,
            themes
        )
        
        return {
            "classification": classification,
            "delta_cp": int(delta_cp),
            "themes": themes,
            "explanation": explanation,
            "stockfish_eval": int(stockfish_cp),
            "fianchetto_eval": fianchetto_eval,
        }
    
    def _identify_themes(self, features: Dict, delta_cp: int) -> List[str]:
        """Identify strategic themes."""
        themes = []
        
        material = features.get("material_balance", 0)
        if abs(material) > 300:
            themes.append("Material imbalance")
        
        center = features.get("center_control", 0)
        if abs(center) > 1:
            themes.append("Center control")
        
        king_safety = features.get("king_safety", 0)
        if abs(king_safety) > 2.0:
            themes.append("King safety")
        
        doubled = features.get("doubled_pawns", 0)
        isolated = features.get("isolated_pawns", 0)
        if abs(doubled) > 0 or abs(isolated) > 0:
            themes.append("Pawn structure")
        
        activity = features.get("activity", 0)
        if abs(activity) > 10:
            themes.append("Piece activity")
        
        return themes
    
    def _classify_position(self, eval_cp: float) -> str:
        """Classify position as winning, equal, or losing."""
        if eval_cp > 200:
            return "winning"
        elif eval_cp < -200:
            return "losing"
        else:
            return "equal"
    
    def __del__(self):
        """Cleanup."""
        if hasattr(self, 'stockfish'):
            self.stockfish.stop()

