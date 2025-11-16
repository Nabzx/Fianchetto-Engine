"""
FastAPI inference server for neural evaluation.
"""

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import torch
import numpy as np
from pathlib import Path
import os

from model import load_model, create_model
from encode import fen_to_planes


app = FastAPI(title="Fianchetto Neural Evaluation Service")

# Global model
model = None
device = torch.device("cpu")


class EvaluateRequest(BaseModel):
    fen: str


class EvaluateResponse(BaseModel):
    score: int  # Centipawn score


@app.on_event("startup")
async def load_model_on_startup():
    """Load the model when the server starts."""
    global model, device
    
    model_path = os.getenv("MODEL_PATH", "models/best_cnn.pt")
    
    if not Path(model_path).exists():
        print(f"Warning: Model file {model_path} not found. Creating a dummy model.")
        # Create a dummy model for testing
        model = create_model("cnn")
        model.eval()
    else:
        print(f"Loading model from {model_path}")
        model = load_model(model_path, device=str(device))
    
    model = model.to(device)
    print("Model loaded successfully!")


@app.get("/")
async def root():
    return {"message": "Fianchetto Neural Evaluation Service"}


@app.get("/health")
async def health():
    return {"status": "healthy", "model_loaded": model is not None}


@app.post("/evaluate", response_model=EvaluateResponse)
async def evaluate(request: EvaluateRequest):
    """
    Evaluate a chess position from FEN string.
    Returns centipawn score (positive = white is better).
    """
    if model is None:
        raise HTTPException(status_code=503, detail="Model not loaded")
    
    try:
        # Encode FEN to planes
        planes = fen_to_planes(request.fen)
        
        # Convert to tensor and add batch dimension
        input_tensor = torch.from_numpy(planes).unsqueeze(0).to(device)
        
        # Inference
        with torch.no_grad():
            output = model(input_tensor)
            score = int(output.item() * 100)  # Convert to centipawns
        
        return EvaluateResponse(score=score)
    
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"Error evaluating position: {str(e)}")


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

