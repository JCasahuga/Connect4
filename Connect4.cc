#include <iostream>
#include <vector>
#include <chrono>
using namespace std;
using namespace std::chrono;

// Current Turn and Table Global Variables
int turn = 1;
int table[42] = {};
bool pruning = true;

// Move Properties: Position and Value of the Move
class Move {
public:
    int id;
    int score;
};

// Checks Board State
int checkState(const int (&cTable)[42]) {
    // Vertical Win/Lose?

    for (int i = 0; i < 36; ++i) {
        int j = 7;
        if (i/7 < 3)
            while (j < 4 && cTable[i+j] == cTable[i]) j+=7;
        if (j == 3) return cTable[i];
    }

    /*for(int i = 0; i < 7; i+=3) {
        if(cTable[i] == cTable[i+1] && cTable[i+1] == cTable[i+2] && cTable[i] != 0) {
            return cTable[i];
        }
    }*/
    // Horizontal Win/Lose?
    for (int i = 0; i < 39; ++i) {
        int j = 1;
        if (i%7 < 4)
            while (j < 4 && cTable[i+j] == cTable[i]) ++j;
        if (j == 3) return cTable[i];
    }

    for(int i = 0; i < 3; ++i) {
        if(cTable[i] == cTable[i+3] && cTable[i+3] == cTable[i+6] && cTable[i] != 0) {
            return cTable[i];
        }
    }
    // Diagonal Win/Lose?
    if(cTable[0] == cTable[4] && cTable[4] == cTable[8] && cTable[0] != 0) {
            return cTable[0];
    } else if (cTable[2] == cTable[4] && cTable[4] == cTable[6] && cTable[2] != 0) {
            return cTable[2];
    }
    // Continue?
    for(int i = 0; i < 9; ++i){
        if(cTable[i] == 0) {
            return 2;
        }
    }
    // Tie
    return 0;
}
9
// Minimax
Move minimax(int cTurn, int (&cTable)[9], int &totalNodes) {
    // Checks Current State
    int s = checkState(cTable);
    if(s != 2) {
        Move move;
        if (s == 1) move.score = 1;
        else if (s == -1) move.score = -1;
        else move.score = 0;
        return move;
    }
    
    // Moves Vector
    vector<Move> moves;
    // Possible Moves
    for(int i = 0; i < 9; ++i) {
        if(cTable[i] == 0) {
            Move move = Move();
            move.id = i;
            cTable[i] = cTurn;
            move.score = minimax(-cTurn, cTable, totalNodes).score;
            moves.push_back(move);
            cTable[i] = 0;
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
void displayGame(const int (&t)[9]) {
    char tD[9] = {};
    for(int i = 0; i < 9; ++i){
        if(t[i] == 0) tD[i] = '-';
        else if(t[i] == 1) tD[i] = 'O';
        else tD[i] = 'X';
    }
    cout << tD[0] << " | " << tD[1] << " | " << tD[2] << endl;
    cout << tD[3] << " | " << tD[4] << " | " << tD[5] << endl;
    cout << tD[6] << " | " << tD[7] << " | " << tD[8] << endl;
}

// Keeps Track of Turns and Plays
void game() {
    bool allEmpty = true;
    for(int i = 0; i < 9; ++i) {
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
    if(turn == 1){
        cout << "CPU PLAY:" << endl;
        int totalNodes = 0;
        auto start = high_resolution_clock::now();
        table[minimax(turn, table, totalNodes).id] = 1;
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
        char m;
        cin >> m;
        if (m > '8' || m < '0' || table[m-'0'] != 0){
            game();
            return;
        }
        table[m] = -1;
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