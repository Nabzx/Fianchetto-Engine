"""
Positional feature extraction for explainability.
"""

import chess
from typing import Dict, List


def extract_features(board: chess.Board) -> Dict[str, float]:
    """
    Extract positional features from a chess position.
    
    Returns:
        Dictionary of feature names to values
    """
    features = {}
    
    # Material balance
    piece_values = {
        chess.PAWN: 100,
        chess.KNIGHT: 320,
        chess.BISHOP: 330,
        chess.ROOK: 500,
        chess.QUEEN: 900,
        chess.KING: 20000,
    }
    
    white_material = 0
    black_material = 0
    
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            value = piece_values[piece.piece_type]
            if piece.color == chess.WHITE:
                white_material += value
            else:
                black_material += value
    
    features["material_balance"] = white_material - black_material
    
    # Center control
    center_squares = [chess.E4, chess.E5, chess.D4, chess.D5]
    white_center = sum(1 for sq in center_squares if board.piece_at(sq) and board.piece_at(sq).color == chess.WHITE)
    black_center = sum(1 for sq in center_squares if board.piece_at(sq) and board.piece_at(sq).color == chess.BLACK)
    features["center_control"] = white_center - black_center
    
    # King safety (simplified: distance from center)
    white_king = board.king(chess.WHITE)
    black_king = board.king(chess.BLACK)
    
    if white_king is not None:
        white_king_rank = chess.square_rank(white_king)
        white_king_file = chess.square_file(white_king)
        white_king_center_dist = abs(white_king_rank - 3.5) + abs(white_king_file - 3.5)
    else:
        white_king_center_dist = 10.0
    
    if black_king is not None:
        black_king_rank = chess.square_rank(black_king)
        black_king_file = chess.square_file(black_king)
        black_king_center_dist = abs(black_king_rank - 3.5) + abs(black_king_file - 3.5)
    else:
        black_king_center_dist = 10.0
    
    features["king_safety"] = black_king_center_dist - white_king_center_dist
    
    # Pawn structure
    white_pawns = list(board.pieces(chess.PAWN, chess.WHITE))
    black_pawns = list(board.pieces(chess.PAWN, chess.BLACK))
    
    # Doubled pawns
    white_doubled = count_doubled_pawns(white_pawns)
    black_doubled = count_doubled_pawns(black_pawns)
    features["doubled_pawns"] = black_doubled - white_doubled
    
    # Isolated pawns
    white_isolated = count_isolated_pawns(white_pawns)
    black_isolated = count_isolated_pawns(black_pawns)
    features["isolated_pawns"] = black_isolated - white_isolated
    
    # Open files (files with no pawns)
    open_files_white = count_open_files(board, chess.WHITE)
    open_files_black = count_open_files(board, chess.BLACK)
    features["open_files"] = open_files_white - open_files_black
    
    # Piece activity (simplified: number of legal moves)
    features["activity"] = len(list(board.legal_moves))
    
    return features


def count_doubled_pawns(pawns: List[int]) -> int:
    """Count doubled pawns."""
    files = [chess.square_file(p) for p in pawns]
    file_counts = {}
    for f in files:
        file_counts[f] = file_counts.get(f, 0) + 1
    return sum(max(0, count - 1) for count in file_counts.values())


def count_isolated_pawns(pawns: List[int]) -> int:
    """Count isolated pawns."""
    if not pawns:
        return 0
    
    files = set(chess.square_file(p) for p in pawns)
    isolated = 0
    
    for pawn in pawns:
        file = chess.square_file(pawn)
        has_neighbor = (file - 1 in files) or (file + 1 in files)
        if not has_neighbor:
            isolated += 1
    
    return isolated


def count_open_files(board: chess.Board, color: chess.Color) -> int:
    """Count open files (files with no pawns of either color)."""
    all_pawns = list(board.pieces(chess.PAWN, chess.WHITE)) + list(board.pieces(chess.PAWN, chess.BLACK))
    occupied_files = set(chess.square_file(p) for p in all_pawns)
    return 8 - len(occupied_files)

