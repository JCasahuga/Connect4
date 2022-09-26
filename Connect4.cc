#include <iostream>
#include <vector>
#include <chrono>
#include <math.h>
#include <algorithm>
#include <iomanip>
using namespace std;
using namespace std::chrono;

// Current Turn and Table Global Variables
int turn = 1;
int table[42] = {};
int stackSize[7] = {};

int pLMove = 3;
int cLMove = 3;
int distribution[42] = {0, 1, 2, 4, 2, 1, 0,
                        0, 2, 4, 6, 4, 2, 0,
                        0, 3, 6, 8, 6, 3, 0,
                        0, 3, 6, 8, 6, 3, 0,
                        0, 2, 4, 6, 4, 2, 0,
                        0, 1, 2, 4, 2, 1, 0};
int totalNodes = 0;

int maxAllowedDepth = 0;
int movesPlayed = 0;

auto start = high_resolution_clock::now();
int timeOutMicro = 1000000;
bool timeOut;

int minH = 100;
int maxH = -100;

vector<int> perm = vector<int>(7);

// Move Properties: Position and Value of the Move
class Move {
public:
    int id;
    int score;
};

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

    // Diagonal 1 Win/Lose?
    for (int i = 0; i < 21; ++i) {
        int j = 8;
        if (i%7 < 4 && cTable[i] != 0)
            while (j < 32 && cTable[i+j] == cTable[i]) j+=8;
        if (j == 32) return cTable[i];
    }

    // Diagonal 2 Win/Lose?
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

int heuristicState(const int (&cTable)[42], int turn) {    
    int result = 0;

    // Vertical Left To Connect
    for (int i = 0; i < 21; ++i) {
        int j = 0;
        int chips = 0;
        int type = 0;
        bool fillable = true;
        while (j < 28) {
            if (cTable[i+j] != 0) {
                if (type == 0) type = cTable[i+j];
                else if (type != cTable[i+j]) fillable = false;
                ++chips;
            }
            j+=7;
        }
        if (fillable) result += type*pow(type*chips,2);
    }
    
    // Horizontal Left To Connect
    for (int i = 0; i < 42; ++i) {
        int j = 0;
        int chips = 0;
        int type = 0;
        bool fillable = true;
        if (i%7 < 4) {
            while (j < 4) {
                if (cTable[i+j] != 0) {
                    if (type == 0) type = cTable[i+j];
                    else if (type != cTable[i+j]) fillable = false;
                    ++chips;
                }
                ++j;
            }
        }
        if (fillable) result += type*pow(type*chips,2);
    }

    // Diagonal Left To Connect
    for (int i = 0; i < 21; ++i) {
        int j = 0;
        int chips = 0;
        int type = 0;
        bool fillable = true;
        if (i%7 < 4) {
            while (j < 32) {
                if (cTable[i+j] != 0) {
                    if (type == 0) type = cTable[i+j];
                    else if (type != cTable[i+j]) fillable = false;
                    ++chips;
                }
                j+=8;
            }
        }
        if (fillable) result += type*pow(type*chips,2);
    }

    // Diagonal 2 Left To Connect
    for (int i = 0; i < 21; ++i) {
        int j = 0;
        int chips = 0;
        int type = 0;
        bool fillable = true;
        if (i%7 > 2) {
            while (j < 24) {
                if (cTable[i+j] != 0) {
                    if (type == 0) type = cTable[i+j];
                    else if (type != cTable[i+j]) fillable = false;
                    ++chips;
                }
                j += 6;
            }
        }
        if (fillable) result += type*pow(type*chips,2);
    }

    result /= 5;
    
    if (result < minH) minH = result;
    if (result > maxH) maxH = result;

    if (result > 30) return 29;
    if (result < -30) return -29;


    return result;
}

// Minimax
Move minimax(int cTurn, int (&cTable)[42], int (&cStackSize)[7], int depth, int alpha, int beta) {
    // Checks Time Out
    auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);;
    if (duration.count() > timeOutMicro) {
        timeOut = true;
        Move move;
        return move;
    }
    // Checks Current State
    int s = checkState(cTable);
    if(s != 2) {
        Move move;
        if (s == 1) move.score = 30*(maxAllowedDepth-depth + 1);
        else if (s == -1) move.score = -30*(maxAllowedDepth-depth + 1);
        else move.score = 0;
        return move;
    }
    // Depth Limit
    if (depth > maxAllowedDepth) {
        Move move;
        move.score = heuristicState(cTable, -cTurn);
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
            move.score = minimax(-cTurn, cTable, cStackSize, depth+1, alpha, beta).score;
            moves.push_back(move);
            cStackSize[index] -= 7;
            cTable[index+cStackSize[index]] = 0;            
            ++totalNodes;
            // Alpha-Beta Pruning
            if (cTurn == 1) {
                alpha = max(alpha, move.score);
                if (beta <= alpha)
                    return move;
            } else {
                beta = min(beta, move.score);
                if (beta <= alpha)
                    return move;
            }
        }
    }
    // Minimizes and Maximizes Score
    int bMove = -1;
    int totalMoves = moves.size();
    if(cTurn == 1){
        // Max
        int bScore = -1000;
        for(int i = 0; i < totalMoves; ++i) {
            if (moves[i].score > bScore) {
                bMove = i;
                bScore = moves[i].score;
            }
        }
    } else {
        // Min
        int bScore = 1000;
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
    if (turn == 1) {
        // Move Ordering
        vector<pair<int,int>> dist = vector<pair<int,int>> (7);
        for (int i = 0; i < 7; ++i) {
            int pDiff = 7 - abs(i - pLMove);
            int cDiff = 7 - abs(i - cLMove);
            int tVal = distribution[i+stackSize[i]];
            dist[i] = make_pair(pDiff/3 + cDiff/5 + distribution[i], i);
        }
        sort(dist.begin(), dist.end());

        for (int i = 6; i >= 0; --i) perm[6-i] = dist[i].second;

        cout << "CPU PLAY:" << endl;
        totalNodes = 0;
        start = high_resolution_clock::now();

        Move bM;
        for (int i = 0; i < 42-movesPlayed; ++i) {
            maxAllowedDepth = i;
            Move m;
            // Null Window
            int min = -512;
            int max = 512;
            /*while (min < max) {
                int med = min + (max - min) / 2;
                if (med <= 0 && min / 2 < med) med = min/2;
                else if (med >= 0 && max / 2 > med) med = max / 2;
                m = minimax(turn, table, stackSize, 0, med, med+1);
                if (m.score <= med) max = m.score;
                else min = m.score;
            }*/
            
            m = minimax(turn, table, stackSize, 0, min, max);
            if (timeOut) break;
            bM = m;
        }
        cout << "Depth " << maxAllowedDepth << " Best Move: " << bM.id << " ";
        if (bM.score > 0) cout << " ";
        cout << (float(bM.score)/10) << endl;
        timeOut = false;

        table[bM.id + stackSize[bM.id]] = 1;
        stackSize[bM.id] += 7;
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Time Elapsed: " << duration.count() / 1000 << " Miliseconds" << endl;
        cout << "Nodes Explored: " << totalNodes << endl;
        cout << minH << endl;
        cout << maxH << endl;
        minH = 100;
        maxH = -100;
        displayGame(table);
        turn = -1;
        ++movesPlayed;
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
        pLMove = m;
        ++movesPlayed;
        game();
    }
}

// Main
int main() {
    cout << setprecision(2) << fixed;
    cout << "Do you want to start? Type Y to confirm or N to deny" << endl;
    char i;
    cin >> i;
    if (i == 'Y' || i == 'y') turn = -1;
    
    game();
}