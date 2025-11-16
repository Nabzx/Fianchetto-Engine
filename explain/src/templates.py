"""
Template strings for generating explanations.
"""

from typing import List

EXPLANATION_TEMPLATES = {
    "material_advantage": "White has a material advantage of {delta} centipawns.",
    "material_disadvantage": "Black has a material advantage of {delta} centipawns.",
    "center_control": "The position shows {side} controlling the center squares.",
    "king_safety": "The {side} king is {status}.",
    "pawn_structure": "The pawn structure favors {side} due to {reason}.",
    "piece_activity": "The position shows {side} has more active pieces.",
    "tactical": "There is a tactical opportunity: {description}.",
    "positional": "The position is strategically favorable for {side}.",
}


def generate_explanation(
    delta_cp: int,
    features: dict,
    stockfish_eval: float,
    themes: List[str]
) -> str:
    """
    Generate a natural language explanation from features and evaluation.
    
    Args:
        delta_cp: Evaluation difference in centipawns
        features: Extracted positional features
        stockfish_eval: Stockfish evaluation
        themes: List of identified themes
    
    Returns:
        Natural language explanation
    """
    parts = []
    
    # Material
    material_balance = features.get("material_balance", 0)
    if abs(material_balance) > 200:
        if material_balance > 0:
            parts.append(f"White has a material advantage ({material_balance} centipawns).")
        else:
            parts.append(f"Black has a material advantage ({-material_balance} centipawns).")
    
    # Center control
    center_control = features.get("center_control", 0)
    if abs(center_control) > 0:
        side = "White" if center_control > 0 else "Black"
        parts.append(f"{side} controls more center squares.")
    
    # King safety
    king_safety = features.get("king_safety", 0)
    if abs(king_safety) > 1.0:
        if king_safety > 0:
            parts.append("White's king is safer.")
        else:
            parts.append("Black's king is safer.")
    
    # Pawn structure
    doubled_pawns = features.get("doubled_pawns", 0)
    isolated_pawns = features.get("isolated_pawns", 0)
    if abs(doubled_pawns) > 0 or abs(isolated_pawns) > 0:
        if doubled_pawns > 0 or isolated_pawns > 0:
            parts.append("White has better pawn structure.")
        else:
            parts.append("Black has better pawn structure.")
    
    # Activity
    activity = features.get("activity", 0)
    if activity > 5:
        parts.append("White has more active pieces.")
    elif activity < -5:
        parts.append("Black has more active pieces.")
    
    # Add themes
    for theme in themes:
        parts.append(theme)
    
    # Overall evaluation
    if abs(delta_cp) < 50:
        parts.append("The position is roughly equal.")
    elif delta_cp > 0:
        parts.append(f"White is better by {delta_cp} centipawns.")
    else:
        parts.append(f"Black is better by {-delta_cp} centipawns.")
    
    return " ".join(parts) if parts else "The position is balanced."

