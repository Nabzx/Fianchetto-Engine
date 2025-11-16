"""
FastAPI server for explainability service.
"""

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import Optional
import os

from explanation_engine import ExplanationEngine


app = FastAPI(title="Fianchetto Explainability Service")

# Global explanation engine
engine: Optional[ExplanationEngine] = None


class ExplainRequest(BaseModel):
    fen: str
    fianchetto_eval: Optional[int] = 0


class ExplainResponse(BaseModel):
    classification: str
    delta_cp: int
    themes: list[str]
    explanation: str
    stockfish_eval: int
    fianchetto_eval: int


@app.on_event("startup")
async def startup():
    """Initialize explanation engine."""
    global engine
    stockfish_path = os.getenv("STOCKFISH_PATH", None)
    try:
        engine = ExplanationEngine(stockfish_path=stockfish_path)
        print("Explanation engine initialized successfully!")
    except Exception as e:
        print(f"Warning: Could not initialize Stockfish: {e}")
        print("Explanation service will work without Stockfish comparison.")


@app.on_event("shutdown")
async def shutdown():
    """Cleanup."""
    global engine
    if engine:
        engine.stockfish.stop()


@app.get("/")
async def root():
    return {"message": "Fianchetto Explainability Service"}


@app.get("/health")
async def health():
    return {"status": "healthy", "engine_loaded": engine is not None}


@app.post("/explain", response_model=ExplainResponse)
async def explain(request: ExplainRequest):
    """
    Generate explanation for a chess position.
    """
    if engine is None:
        raise HTTPException(status_code=503, detail="Explanation engine not loaded")
    
    try:
        result = engine.explain(request.fen, request.fianchetto_eval)
        return ExplainResponse(**result)
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"Error generating explanation: {str(e)}")


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8001)

