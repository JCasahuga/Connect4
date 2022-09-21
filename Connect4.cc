#include <iostream>
#include <vector>
#include <chrono>
using namespace std;
using namespace std::chrono;

// Current Turn and Table Global Variables
int turn = 1;
int table[42] = {};
int stackSize[7] = {};
bool pruning = true;

// Move Properties: Position and Value of the Move
class Move {
public:
    int id;
    int score;
};

// 7 * 6

// Checks Board State
int checkState(const int (&cTable)[42]) {
    // Vertical Win/Lose?
    for (int i = 0; i < 21; ++i) {
        int j = 7;
        if (cTable[i] != 0)
            while (j < 28 && cTable[i+j] == cTable[i]) j+=7;
        if (j == 28) return cTable[i];
    }

    // Horizontal Win/Lose?
    for (int i = 0; i < 38; ++i) {
        int j = 1;
        if (i%7 < 4 && cTable[i] != 0)
            while (j < 4 && cTable[i+j] == cTable[i]) ++j;
        if (j == 4) return cTable[i];
    }

    // Diagonal 1
    for (int i = 0; i < 21; ++i) {
        int j = 8;
        if (i%7 < 4 && cTable[i] != 0)
            while (j < 32 && cTable[i+j] == cTable[i]) j+=8;
        if (j == 32) return cTable[i];
    }

    // Diagonal 2
    for (int i = 42; i >= 21; --i) {
        int j = 24;
        if (i%7 >= 4 && cTable[i] != 0)
            while (j < 24 && cTable[i+j] == cTable[i]) j-=8;
        if (j == 0) return cTable[i];
    }

    // Continue?
    for(int i = 0; i < 42; ++i){
        if(cTable[i] == 0) {
            return 2;
        }
    }
    // Tie
    return 0;
}

int heuristicState(const int (&cTable)[42]) {
    int playerState = 0;
    int CPUState = 0;

    // Vertical Win/Lose?
    for (int i = 0; i < 21; ++i) {
        int j = 7;
        if (cTable[i] != 0) {
            while (j < 28 && cTable[i+j] == cTable[i]) j+=7;
            if (cTable[i] == -1) playerState = max(playerState, j/7);
            else if (cTable[i]) CPUState = max(CPUState, j/7);
        }
    }

    // Horizontal Win/Lose?
    for (int i = 0; i < 38; ++i) {
        int j = 1;
        if (i%7 < 4 && cTable[i] != 0)
            while (j < 4 && cTable[i+j] == cTable[i]) ++j;
            if (cTable[i] == -1) playerState = max(playerState, j);
            else if (cTable[i]) CPUState = max(CPUState, j);
    }

    // Diagonal 1
    for (int i = 0; i < 21; ++i) {
        int j = 8;
        if (i%7 < 4 && cTable[i] != 0)
            while (j < 32 && cTable[i+j] == cTable[i]) j+=8;

        if (cTable[i] == -1) playerState = max(playerState, j/8);
        else if (cTable[i]) CPUState = max(CPUState, j/8);
    }

    // Diagonal 2
    for (int i = 42; i >= 21; --i) {
        int j = 24;
        if (i%7 >= 4 && cTable[i] != 0)
            while (j < 24 && cTable[i+j] == cTable[i]) j-=8;
        
        if (cTable[i] == -1) playerState = max(playerState, (24-j)/8);
        else if (cTable[i]) CPUState = max(CPUState, (24-j)/8);
    }

    // Continue?
    for(int i = 0; i < 42; ++i){
        if(cTable[i] == 0) {
            return -1;
        }
    }

    // Tie
    return -1;
}

// Minimax
Move minimax(int cTurn, int (&cTable)[42], int (&cStackSize)[7], int &totalNodes, int depth) {
    // Checks Current State
    int s = checkState(cTable);
    if(s != 2) {
        Move move;
        if (s == 1) move.score = 1;
        else if (s == -1) move.score = -1;
        else move.score = 0;
        return move;
    }

    if (depth > 11) {
        Move move;
        move.score = checkState(cTable);
        return move;
    }
    
    // Moves Vector
    vector<Move> moves;
    // Possible Moves
    for(int i = 0; i < 7; ++i) {
        if(cStackSize[i] < 42) {
            Move move = Move();
            move.id = i;
            cTable[i+cStackSize[i]] = cTurn;
            cStackSize[i] += 7;
            move.score = minimax(-cTurn, cTable, cStackSize, totalNodes, depth+1).score;
            moves.push_back(move);
            cStackSize[i] -= 7;
            cTable[i+cStackSize[i]] = 0;
            if(move.score == cTurn && pruning) return move;
            ++totalNodes;
        }
    }
    // Minimizes and Maximizes Score
    int bMove = 0;
    int totalMoves = moves.size();
    if(cTurn == 1){
        // Max
        int bScore = -10;
        for(int i = 0; i < totalMoves; ++i) {
            if (moves[i].score > bScore) {
                bMove = i;
                bScore = moves[i].score;
            }
        }
    } else {
        // Min
        int bScore = 10;
        for(int i = 0; i < totalMoves; ++i) {
            if (moves[i].score < bScore) {
                bMove = i;
                bScore = moves[i].score;
            }
        }
    }
    return moves[bMove];
}

// Displays Board
void displayGame(const int (&t)[42]) {
    char tD[42] = {};
    for(int i = 0; i < 42; ++i){
        if(t[i] == 0) tD[i] = '-';
        else if(t[i] == 1) tD[i] = 'O';
        else tD[i] = 'X';
    }

    for (int i = 5; i >= 0; --i) {
        for (int j = 0; j < 7; ++j) {
            cout << tD[i*7+j];
            if (j != 6) cout << " | ";
        }
        cout << endl;
    }
}

// Keeps Track of Turns and Plays
void game() {
    bool allEmpty = true;
    for(int i = 0; i < 42; ++i) {
        if(table[i] != 0) allEmpty = false; 
    }
    // Checks Current State
    int s = checkState(table);
    if(s != 2){
        if (s == 1) cout << "You LOST!" << endl;
        else if (s == -1) cout << "You WON!" << endl;
        else cout << "TIE!" << endl;
        return;
    }
    // Turn 1 = CPU / Turn -1 = Human
    if (turn == 1){
        cout << "CPU PLAY:" << endl;
        int totalNodes = 0;
        auto start = high_resolution_clock::now();
        int id = minimax(turn, table, stackSize, totalNodes, 0).id;
        table[id + stackSize[id]] = 1;
        stackSize[id] += 7;
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Time Elapsed: " << float(duration.count()) / 1000 << " Miliseconds" << endl;
        cout << "Nodes Explored: " << totalNodes << endl;
        displayGame(table);
        turn = -1;
        game();
    } else {
        if(allEmpty) { 
            cout << "YOUR TURN:" << endl;
            displayGame(table);
        }
        int m;
        cin >> m;
        if (m > 6 || m < 0){
            game();
            return;
        }
        if (table[m+stackSize[m]] == 0) {
            table[m+stackSize[m]] = -1;
            stackSize[m] += 7;
        }
        turn = 1;
        game();
    }
}

// Main
int main() {
    cout << "Do you want Pruning Enabled?" << endl;
    char p;
    cin >> p;
    if (p == 'N' || p == 'n') pruning = false;
    cout << "Do you want to start? Type Y to confirm or N to deny" << endl;
    char i;
    cin >> i;
    if (i == 'Y' || i == 'y') turn = -1;
    game();
}