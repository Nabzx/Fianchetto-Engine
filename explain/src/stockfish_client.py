"""
Interface to Stockfish for comparison and best move analysis.
"""

import chess
import chess.engine
from typing import Optional, Tuple
import os


class StockfishClient:
    """
    Wrapper around Stockfish engine for evaluation and best move.
    """
    
    def __init__(self, stockfish_path: Optional[str] = None):
        """
        Initialize Stockfish client.
        
        Args:
            stockfish_path: Path to Stockfish executable. If None, tries to find it.
        """
        self.stockfish_path = stockfish_path or self._find_stockfish()
        self.engine = None
        
    def _find_stockfish(self) -> str:
        """Try to find Stockfish executable."""
        # Common paths
        paths = [
            "/usr/bin/stockfish",
            "/usr/local/bin/stockfish",
            "stockfish",  # In PATH
        ]
        
        for path in paths:
            if os.path.exists(path) or path == "stockfish":
                return path
        
        raise RuntimeError("Stockfish not found. Please install Stockfish or provide path.")
    
    def start(self):
        """Start the engine."""
        if self.engine is None:
            self.engine = chess.engine.SimpleEngine.popen_uci(self.stockfish_path)
    
    def stop(self):
        """Stop the engine."""
        if self.engine:
            self.engine.quit()
            self.engine = None
    
    def evaluate(self, board: chess.Board, depth: int = 10) -> Tuple[float, Optional[chess.Move]]:
        """
        Evaluate position and get best move.
        
        Returns:
            (score, best_move) where score is in centipawns
        """
        if self.engine is None:
            self.start()
        
        try:
            info = self.engine.analyse(board, chess.engine.Limit(depth=depth))
            score = info["score"].white()
            
            # Convert score to centipawns
            if score.is_mate():
                cp = 30000 if score.mate() > 0 else -30000
            else:
                cp = score.score()
            
            best_move = info.get("pv", [None])[0] if info.get("pv") else None
            
            return cp, best_move
        except Exception as e:
            print(f"Error evaluating with Stockfish: {e}")
            return 0, None
    
    def __enter__(self):
        self.start()
        return self
    
    def __exit__(self, *args):
        self.stop()

