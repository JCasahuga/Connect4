#include <iostream>
#include <vector>
#include <chrono>
using namespace std;
using namespace std::chrono;

// Current Turn and Table Global Variables
int turn = 1;
int table[42] = {};
int stackSize[7] = {};
int lastMove = 3;
int maxVal = 0;
int minVal = 0;

int maxH = 0;
int minH = 0;

int selectedDepth = 6;
bool pruning = true;
vector<int> perm = vector<int>(7);

// Move Properties: Position and Value of the Move
class Move {
public:
    int id;
    int score;
};

// 7 * 6

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
    for (int i = 0; i < 42; ++i) {
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

    // Diagonal 2 Wrong Left to Right
    for (int i = 0; i < 21; ++i) {
        int j = 6;
        if (i%7 > 2 && cTable[i] != 0)
            while (j < 24 && cTable[i+j] == cTable[i]) j+=6;
        if (j == 24) return cTable[i];
    }

    // Continue?
    for(int i = 0; i < 42; ++i) {
        if(cTable[i] == 0) {
            return 2;
        }
    }
    
    // Tie
    return 0;
}

int heuristicState(const int (&cTable)[42]) {    
    int result = 0;

    // Vertical Win/Lose?
    for (int i = 0; i < 21; ++i) {
        int j = 7;
        if (cTable[i] != 0)
            while (j < 28 && cTable[i+j] == cTable[i]) j+=7;
        if (cTable[i+j] == 0) result += cTable[i]*j/7;
    }
    
    // Horizontal Win/Lose?
    for (int i = 0; i < 42; ++i) {
        int j = 1;
        if (i%7 < 4 && cTable[i] != 0)
            while (j < 4 && cTable[i+j] == cTable[i]) ++j;
        if (cTable[i+j] == 0) result += cTable[i]*j;
    }

    // Diagonal 1
    for (int i = 0; i < 21; ++i) {
        int j = 8;
        if (i%7 < 4 && cTable[i] != 0)
            while (j < 32 && cTable[i+j] == cTable[i]) j+=8;
        if (cTable[i+j] == 0) result += cTable[i]*j/8;
    }

    // Diagonal 2 Wrong Left to Right
    for (int i = 0; i < 21; ++i) {
        int j = 6;
        if (i%7 > 2 && cTable[i] != 0)
            while (j < 24 && cTable[i+j] == cTable[i]) j+=6;
        if (cTable[i+j] == 0) result += cTable[i]*j/6;
    }

    if (result < minH) minH = result;
    if (result > maxH) maxH = result;
    //if (result > 0) return 0;
    //if (result < -8) return -8;
    return result;
}

// Minimax
Move minimax(int cTurn, int (&cTable)[42], int (&cStackSize)[7], int &totalNodes, int depth) {

    // Checks Current State
    int s = checkState(cTable);
    if(s != 2) {
        Move move;
        if (s == 1) move.score = 30*(selectedDepth-depth);
        else if (s == -1) move.score = -30*(selectedDepth-depth);
        else move.score = 0;
        return move;
    }
    // 4 minim per vert + hor
    if (depth > selectedDepth) {
        Move move;
        //move.score = cTurn;
        move.score = heuristicState(cTable);
        return move;
    }
    // Moves Vector
    vector<Move> moves = {};
    // Possible Moves
    for(int i = 0; i < 7; ++i) {
        int index = perm[i];
        if(cStackSize[index] < 42) {
            Move move = Move();
            move.id = index;
            cTable[index+cStackSize[index]] = cTurn;
            cStackSize[index] += 7;
            move.score = minimax(-cTurn, cTable, cStackSize, totalNodes, depth+1).score;
            moves.push_back(move);
            cStackSize[index] -= 7;
            cTable[index+cStackSize[index]] = 0;
            //if (depth == 0) cout << index << " score " << move.score << endl;
            if (pruning) {
                bool winOrLose = (move.score == maxVal && cTurn) || (move.score == minVal && cTurn == -1);
                if(winOrLose) return move;
            }
            ++totalNodes;
        }
    }
    // Minimizes and Maximizes Score
    int bMove = -1;
    int totalMoves = moves.size();
    if(cTurn == 1){
        // Max
        int bScore = -300;
        for(int i = 0; i < totalMoves; ++i) {
            if (moves[i].score > bScore) {
                bMove = i;
                bScore = moves[i].score;
            }
        }
    } else {
        // Min
        int bScore = 300;
        for(int i = 0; i < totalMoves; ++i) {
            if (moves[i].score < bScore) {
                bMove = i;
                bScore = moves[i].score;
            }
        }
    }
    return moves[bMove];
}

// Keeps Track of Turns and Plays
void game() {
    bool allEmpty = true;
    for(int i = 0; i < 42; ++i) {
        if(table[i] != 0) allEmpty = false; 
    }
    // Checks Current State
    int s = checkState(table);
    if(s != 2) {
        if (s == 1) cout << "You LOST!" << endl;
        else if (s == -1) cout << "You WON!" << endl;
        else cout << "TIE!" << endl;
        return;
    }
    // Turn 1 = CPU / Turn -1 = Human
    if (turn == 1){
        int offset = (rand()%2);
        perm[offset] = lastMove;
        if (offset == 1) {
            offset = (rand()%2);
            if (offset == 1) {
                perm[0] = (lastMove+6)%7;
                perm[2] = (lastMove+1)%7;
            } else {
                perm[2] = (lastMove+6)%7;
                perm[0] = (lastMove+1)%7;
            }
        }
        perm[3] = (lastMove+5)%7;
        perm[4] = (lastMove+2)%7;
        perm[5] = (lastMove+4)%7;
        perm[6] = (lastMove+3)%7;

        for (int i = 0; i <7; ++i) cout << perm[i];
        cout << endl;
        cout << "CPU PLAY:" << endl;
        int totalNodes = 0;
        maxH = -100;
        minH = 100;
        auto start = high_resolution_clock::now();
        int id = minimax(turn, table, stackSize, totalNodes, 0).id;
        table[id + stackSize[id]] = 1;
        stackSize[id] += 7;
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Time Elapsed: " << float(duration.count()) / 1000 << " Miliseconds" << endl;
        cout << "Nodes Explored: " << totalNodes << endl;
        cout << "Max = " << maxH << endl;
        cout << "Min = " << minH << endl;
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
        if (m > 6 || m < 0 || stackSize[m] > 35){
            game();
            return;
        }
        if (table[m+stackSize[m]] == 0) {
            table[m+stackSize[m]] = -1;
            stackSize[m] += 7;
        }
        turn = 1;
        lastMove = m;
        game();
    }
}

// Main
int main() {
    minVal = (-selectedDepth+2)*30;
    maxVal = (selectedDepth-1)*30;
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