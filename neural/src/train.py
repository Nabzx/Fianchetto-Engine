"""
Training script for the neural evaluation model.
"""

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader
import argparse
import os
from pathlib import Path

from model import create_model
from dataset import generate_synthetic_dataset, create_dataloader


def train_epoch(model, dataloader, criterion, optimizer, device):
    """Train for one epoch."""
    model.train()
    total_loss = 0.0
    num_batches = 0
    
    for planes, targets in dataloader:
        planes = planes.to(device)
        targets = targets.to(device)
        
        optimizer.zero_grad()
        outputs = model(planes)
        loss = criterion(outputs, targets)
        loss.backward()
        optimizer.step()
        
        total_loss += loss.item()
        num_batches += 1
    
    return total_loss / num_batches if num_batches > 0 else 0.0


def validate(model, dataloader, criterion, device):
    """Validate the model."""
    model.eval()
    total_loss = 0.0
    num_batches = 0
    
    with torch.no_grad():
        for planes, targets in dataloader:
            planes = planes.to(device)
            targets = targets.to(device)
            
            outputs = model(planes)
            loss = criterion(outputs, targets)
            
            total_loss += loss.item()
            num_batches += 1
    
    return total_loss / num_batches if num_batches > 0 else 0.0


def main():
    parser = argparse.ArgumentParser(description="Train chess evaluation model")
    parser.add_argument("--model-type", choices=["cnn", "mlp"], default="cnn")
    parser.add_argument("--epochs", type=int, default=10)
    parser.add_argument("--batch-size", type=int, default=32)
    parser.add_argument("--learning-rate", type=float, default=0.001)
    parser.add_argument("--num-positions", type=int, default=10000)
    parser.add_argument("--output-dir", type=str, default="models")
    parser.add_argument("--device", type=str, default="cpu")
    
    args = parser.parse_args()
    
    device = torch.device(args.device)
    print(f"Using device: {device}")
    
    # Create output directory
    os.makedirs(args.output_dir, exist_ok=True)
    
    # Generate dataset
    print("Generating dataset...")
    positions, evaluations = generate_synthetic_dataset(args.num_positions)
    
    # Split train/val
    split_idx = int(0.8 * len(positions))
    train_positions = positions[:split_idx]
    train_evaluations = evaluations[:split_idx]
    val_positions = positions[split_idx:]
    val_evaluations = evaluations[split_idx:]
    
    # Create dataloaders
    train_loader = create_dataloader(train_positions, train_evaluations, 
                                     batch_size=args.batch_size, shuffle=True)
    val_loader = create_dataloader(val_positions, val_evaluations,
                                   batch_size=args.batch_size, shuffle=False)
    
    # Create model
    model = create_model(args.model_type).to(device)
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=args.learning_rate)
    
    # Training loop
    best_val_loss = float('inf')
    
    for epoch in range(args.epochs):
        train_loss = train_epoch(model, train_loader, criterion, optimizer, device)
        val_loss = validate(model, val_loader, criterion, device)
        
        print(f"Epoch {epoch+1}/{args.epochs}")
        print(f"  Train Loss: {train_loss:.4f}")
        print(f"  Val Loss: {val_loss:.4f}")
        
        # Save best model
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            model_path = Path(args.output_dir) / f"best_{args.model_type}.pt"
            torch.save({
                "model_state_dict": model.state_dict(),
                "model_type": args.model_type,
                "epoch": epoch,
                "val_loss": val_loss,
            }, model_path)
            print(f"  Saved best model to {model_path}")
    
    print("Training complete!")


if __name__ == "__main__":
    main()

