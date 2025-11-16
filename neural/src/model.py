"""
Neural network model for chess position evaluation.
Simple CNN architecture for centipawn score prediction.
"""

import torch
import torch.nn as nn
import torch.nn.functional as F


class ChessEvalModel(nn.Module):
    """
    Simple CNN model for chess evaluation.
    Input: (batch, 12, 8, 8) - 12 planes (6 piece types × 2 colors)
    Output: (batch, 1) - centipawn score
    """
    
    def __init__(self, hidden_dim=256):
        super().__init__()
        
        # Convolutional layers
        self.conv1 = nn.Conv2d(12, 64, kernel_size=3, padding=1)
        self.conv2 = nn.Conv2d(64, 128, kernel_size=3, padding=1)
        self.conv3 = nn.Conv2d(128, 128, kernel_size=3, padding=1)
        
        # Fully connected layers
        self.fc1 = nn.Linear(128 * 8 * 8, hidden_dim)
        self.fc2 = nn.Linear(hidden_dim, hidden_dim)
        self.fc3 = nn.Linear(hidden_dim, 1)
        
        self.dropout = nn.Dropout(0.3)
        
    def forward(self, x):
        # x: (batch, 12, 8, 8)
        x = F.relu(self.conv1(x))
        x = F.relu(self.conv2(x))
        x = F.relu(self.conv3(x))
        
        # Flatten
        x = x.view(x.size(0), -1)
        
        # Fully connected
        x = F.relu(self.fc1(x))
        x = self.dropout(x)
        x = F.relu(self.fc2(x))
        x = self.dropout(x)
        x = self.fc3(x)
        
        return x.squeeze(-1)  # (batch,)


class SimpleMLP(nn.Module):
    """
    Alternative: Simple MLP model (faster, less accurate).
    """
    
    def __init__(self, hidden_dim=512):
        super().__init__()
        self.fc1 = nn.Linear(12 * 8 * 8, hidden_dim)
        self.fc2 = nn.Linear(hidden_dim, hidden_dim)
        self.fc3 = nn.Linear(hidden_dim, hidden_dim // 2)
        self.fc4 = nn.Linear(hidden_dim // 2, 1)
        self.dropout = nn.Dropout(0.2)
        
    def forward(self, x):
        x = x.view(x.size(0), -1)  # Flatten
        x = F.relu(self.fc1(x))
        x = self.dropout(x)
        x = F.relu(self.fc2(x))
        x = self.dropout(x)
        x = F.relu(self.fc3(x))
        x = self.fc4(x)
        return x.squeeze(-1)


def create_model(model_type: str = "cnn", **kwargs) -> nn.Module:
    """
    Factory function to create a model.
    
    Args:
        model_type: "cnn" or "mlp"
        **kwargs: Model-specific parameters
    """
    if model_type == "cnn":
        return ChessEvalModel(**kwargs)
    elif model_type == "mlp":
        return SimpleMLP(**kwargs)
    else:
        raise ValueError(f"Unknown model type: {model_type}")


def load_model(model_path: str, device: str = "cpu") -> nn.Module:
    """
    Load a trained model from file.
    """
    checkpoint = torch.load(model_path, map_location=device)
    model_type = checkpoint.get("model_type", "cnn")
    model = create_model(model_type)
    model.load_state_dict(checkpoint["model_state_dict"])
    model.eval()
    return model

