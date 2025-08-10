# Connect 4 AI Engine

This repository contains a high-performance Connect 4 engine written in C++. It uses a bitboard representation for the game state and a minimax search algorithm with alpha-beta pruning, transposition tables, and iterative deepening to find the best move.

This project also includes a Python script to pit two AI engines against each other in a tournament.

## Features

- **C++ Core Engine**: A highly optimized, modular C++ engine.
- **Bitboard Representation**: A fast and efficient way to represent the game board, allowing for rapid move generation and evaluation.
- **Minimax Search**: Implements the minimax algorithm with alpha-beta pruning to efficiently search the game tree.
- **Transposition Tables**: Uses a hash table to cache previously evaluated positions, significantly speeding up the search.
- **Iterative Deepening**: The engine starts with a shallow search and progressively deepens it until the time limit is reached, ensuring a good move is found even under tight time constraints.
- **Modular Architecture**: The source code is separated into logical components:
    - `Position`: Manages the board state.
    - `Search`: Contains the core AI search logic.
    - `TranspositionTable`: Manages the caching of game states.

## Directory Structure

```
.
├── src/              # Source code for the main C++ AI engine
├── scripts/          # Helper scripts for testing and playing
├── LICENSE           # Project License
├── README.md         # This file
└── .gitignore        # Files and directories ignored by Git
```

## How to Build the AI Engine

To build the main AI, navigate to the `src` directory and run `make`.

```bash
cd src
make
```

This will create an executable file named `connect4_ai` in the `src` directory.

## How to Run AI vs. AI Matches

The primary way to test engine strength is to use the `player.py` script. This script manages a tournament between two AI executables.

1.  **Configure `player.py`**: Open `scripts/player.py` and modify the `ai1_executable` and `ai2_executable` variables to point to the AI engines you want to test.

2.  **Run the script**: Execute the script from the root directory of the project.

    ```bash
    python3 scripts/player.py
    ```

The script will run a series of games and print the results and calculated Elo difference to the console.
