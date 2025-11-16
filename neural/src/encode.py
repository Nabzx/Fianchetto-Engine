"""
Board encoding: Convert FEN to tensor planes for neural network input.
"""

import numpy as np
import chess
import chess.pgn


def fen_to_planes(fen: str) -> np.ndarray:
    """
    Convert FEN string to 12 plane representation (6 piece types × 2 colors).
    Each plane is 8×8.
    
    Returns: (12, 8, 8) numpy array
    """
    board = chess.Board(fen)
    planes = np.zeros((12, 8, 8), dtype=np.float32)
    
    # Piece type order: PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
    piece_to_plane = {
        chess.PAWN: 0,
        chess.KNIGHT: 1,
        chess.BISHOP: 2,
        chess.ROOK: 3,
        chess.QUEEN: 4,
        chess.KING: 5,
    }
    
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            color_offset = 0 if piece.color == chess.WHITE else 6
            plane_idx = color_offset + piece_to_plane[piece.piece_type]
            rank = chess.square_rank(square)
            file = chess.square_file(square)
            planes[plane_idx, 7 - rank, file] = 1.0
    
    return planes


def fen_to_tensor(fen: str) -> np.ndarray:
    """
    Convert FEN to flat tensor (12 * 8 * 8 = 768 features).
    """
    planes = fen_to_planes(fen)
    return planes.flatten()


def batch_encode(fens: list[str]) -> np.ndarray:
    """
    Encode multiple FENs into a batch tensor.
    
    Returns: (batch_size, 12, 8, 8) numpy array
    """
    batch = np.stack([fen_to_planes(fen) for fen in fens])
    return batch

