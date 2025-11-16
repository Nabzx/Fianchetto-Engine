"""
Export PyTorch model to ONNX format for faster inference.
"""

import torch
import argparse
from pathlib import Path
from model import load_model, create_model


def export_to_onnx(model_path: str, output_path: str, device: str = "cpu"):
    """
    Export a PyTorch model to ONNX format.
    """
    # Load model
    model = load_model(model_path, device=device)
    model.eval()
    
    # Create dummy input (batch_size=1, 12 planes, 8x8)
    dummy_input = torch.randn(1, 12, 8, 8).to(device)
    
    # Export
    torch.onnx.export(
        model,
        dummy_input,
        output_path,
        input_names=["input"],
        output_names=["output"],
        dynamic_axes={
            "input": {0: "batch_size"},
            "output": {0: "batch_size"},
        },
        opset_version=11,
    )
    
    print(f"Model exported to {output_path}")


def main():
    parser = argparse.ArgumentParser(description="Export model to ONNX")
    parser.add_argument("--model-path", type=str, required=True)
    parser.add_argument("--output-path", type=str, default=None)
    parser.add_argument("--device", type=str, default="cpu")
    
    args = parser.parse_args()
    
    if args.output_path is None:
        model_path = Path(args.model_path)
        args.output_path = str(model_path.with_suffix(".onnx"))
    
    export_to_onnx(args.model_path, args.output_path, args.device)


if __name__ == "__main__":
    main()

