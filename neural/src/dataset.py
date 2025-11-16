"""
Dataset loading and preprocessing for training.
"""

import torch
from torch.utils.data import Dataset, DataLoader
import chess
import chess.pgn
import numpy as np
from typing import Optional
from encode import fen_to_planes


class ChessDataset(Dataset):
    """
    Dataset of chess positions with evaluations.
    """
    
    def __init__(self, positions: list[str], evaluations: list[float]):
        """
        Args:
            positions: List of FEN strings
            evaluations: List of centipawn scores (from white's perspective)
        """
        self.positions = positions
        self.evaluations = np.array(evaluations, dtype=np.float32)
        
    def __len__(self):
        return len(self.positions)
    
    def __getitem__(self, idx):
        fen = self.positions[idx]
        planes = fen_to_planes(fen)
        eval_score = self.evaluations[idx]
        return torch.from_numpy(planes), torch.tensor(eval_score, dtype=torch.float32)


def generate_synthetic_dataset(num_positions: int = 10000) -> tuple[list[str], list[float]]:
    """
    Generate a synthetic dataset for training.
    In production, you would load from PGN files or databases.
    """
    positions = []
    evaluations = []
    
    board = chess.Board()
    
    # Simple evaluation function (material + PST)
    piece_values = {
        chess.PAWN: 100,
        chess.KNIGHT: 320,
        chess.BISHOP: 330,
        chess.ROOK: 500,
        chess.QUEEN: 900,
        chess.KING: 20000,
    }
    
    def evaluate_position(b: chess.Board) -> float:
        score = 0.0
        for square in chess.SQUARES:
            piece = b.piece_at(square)
            if piece:
                value = piece_values[piece.piece_type]
                if piece.color == chess.WHITE:
                    score += value
                else:
                    score -= value
        return score
    
    # Generate random positions
    import random
    for _ in range(num_positions):
        board.reset()
        # Make random moves
        for _ in range(random.randint(0, 20)):
            moves = list(board.legal_moves)
            if moves:
                board.push(random.choice(moves))
        
        fen = board.fen()
        eval_score = evaluate_position(board)
        
        positions.append(fen)
        evaluations.append(eval_score)
    
    return positions, evaluations


def create_dataloader(
    positions: list[str],
    evaluations: list[float],
    batch_size: int = 32,
    shuffle: bool = True
) -> DataLoader:
    """
    Create a DataLoader from positions and evaluations.
    """
    dataset = ChessDataset(positions, evaluations)
    return DataLoader(dataset, batch_size=batch_size, shuffle=shuffle)

