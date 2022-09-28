#include <iostream>
#include <vector>
#include <chrono>
#include <math.h>
#include <algorithm>
#include <iomanip>
#include <string.h>

using namespace std;
using namespace std::chrono;

// Current Turn and Table Global Variables
int turn = 1;
//int table[42] = {};
// Computer
uint64_t bC = 0x0;
// Player
uint64_t bP = 0x0;
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
int timeOutMicro = 3000000;
bool timeOut;

int minH = 100;
int maxH = -100;

int WIDTH = 7;
int HEIGHT = 6;

vector<pair<uint64_t,int>> transposition = vector<pair<uint64_t,int>>(8388593, make_pair(0b0, 0));
int randA = 38327;
int randB = 38327;
int randC = 38327;
int hitTT = 0;

uint64_t pos;
uint64_t mask;

vector<int> perm = vector<int>(7);

// Move Properties: Position and Value of the Move
class Move {
public:
    int id;
    int score;
};

// Bottom Board Mask
static uint64_t bot_mask(int col) {
    return UINT64_C(1) << col*(HEIGHT+1);
}

// Bottom Board Mask
static uint64_t top_mask(int col) {
    return (UINT64_C(1) << (HEIGHT-1)) << col*(HEIGHT+1);
}

// Mod Table Size
unsigned int indexTT(uint64_t key) {
    return key%transposition.size();
}

// Returns Hash Value
uint64_t hashTT(uint64_t val) {
    uint32_t l = val;
    uint32_t h = val >> 32;
    return l*randA + h*randB + randC;
}

// Reset TT
void resetTT() {
    memset(&transposition[0], 0, transposition.size()*sizeof(pair<uint64_t,int>));
}

// Put TT Value
void putTT(uint64_t key, int val) {
    unsigned int i = indexTT(key);
    transposition[i].first = key;
    transposition[i].second = val;
}

// Get TT Value
int getTT(uint64_t key) {
    unsigned int i = indexTT(key);
    if (transposition[i].first == key)
        return transposition[i].second;
    else return -1001;
} 

// Display Game Bitboard
void displayGame(uint64_t tY, uint64_t tR) {
    char tD[48] = {};
    for(int i = 0; i < 48; ++i){
        if (tY & (UINT64_C(1) <<  i)) tD[i] = 'X';
        else if (tR & (UINT64_C(1) <<  i)) tD[i] = 'O';
        else tD[i] = '-';
    }

    for (int i = 5; i >= 0; --i) {
        for (int j = 0; j < 8; ++j) {
            if (j != 7)
            {
                cout << tD[i*8+j];
                if (j != 6) cout << " | ";
            }
        }
        cout << endl;
    }
}

// Checks If The Board Has a Winning Position
bool checkState(uint64_t p) {
    // Horizontal
    if (p & (p >> 1) & (p >> 2) & (p >> 3)) return true;
    // Vertical
    if (p & (p >> 8) & (p >> 16) & (p >> 24)) return true;
    // Diag Left To Right
    if (p & (p >> 9) & (p >> 18) & (p >> 27)) return true;
    // Diag Right To Left
    if (p & (p >> 7) & (p >> 14) & (p >> 21)) return true;
    return false;
}

// Counts Total Set Bits Board
int countTotalBits(uint64_t pC) {
    int t = 0;
    while (pC != 0) {
        t += ((pC & UINT64_C(1)) == 1);
        pC >>= 1;
    }
    return t;
}

// Evaluates Position
int heuristicStateB(uint64_t pC, uint64_t pP, int t) {
    //int mult = (1 + (t == 1));
    int total = 0;
    int amount[] = {1, 7, 8, 9};
    for (int i = 0; i < 4; ++i) {
        total += pow(countTotalBits(pC & (pC >> amount[i]) & (pC >> 2*amount[i]) & (~pP >> 3*amount[i])), 2);
        total += pow(countTotalBits(~pP & (pC >> amount[i]) & (pC >> 2*amount[i]) & (pC >> 3*amount[i])), 2);
    }

    //int mult = (1 + (t == 1));
    for (int i = 0; i < 4; ++i) {
        total -= pow(countTotalBits(pP & (pP >> amount[i]) & (pP >> 2*amount[i]) & (~pC >> 3*amount[i])), 2);
        total -= pow(countTotalBits(~pC & (pP >> amount[i]) & (pP >> 2*amount[i]) & (pP >> 3*amount[i])), 2);
    }

    if (total < minH) minH = total;
    if (total > maxH) maxH = total;

    return total;
}

// Minimax
Move minimax(int cTurn, uint64_t pC, uint64_t pP, int (&cStackSize)[7], int depth, int movesPlayed, int alpha, int beta) {
    Move m;
    // Checks Time Out
    auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);;
    if (duration.count() > timeOutMicro) {
        timeOut = true;
        return m;
    }

    // Transposition Table
    int val = getTT(hashTT(pP)^hashTT(pC));
    if (val != -1001) {
        ++hitTT;
        if (beta > val) {
            beta = val;
            if (alpha >= beta) {
                m.score = val;
                return m;
            }
        }
    }

    // Checks Current State
    if (cTurn == -1 && checkState(pC)) {
        m.score = 30*(maxAllowedDepth-depth + 1);
        return m;
    } else if (cTurn == 1 && checkState(pP)) {
        m.score = -30*(maxAllowedDepth-depth + 1);
        return m;
    }
    if (movesPlayed == 42) return m;

    // Depth Limit
    if (depth > maxAllowedDepth) {
        Move move;
        move.score = heuristicStateB(pC, pP, cTurn);
        //move.score = 0;
        return move;
    }
    // Moves Vector
    vector<Move> moves = {};
    // Possible Movesq
    for(int i = 0; i < 7; ++i) {
        int index = perm[i];
        if(cStackSize[index] <= 40) {
            Move move = Move();
            move.id = index;

            if (cTurn == -1) pP |= (UINT64_C(1) << (cStackSize[index]+index));
            else pC |= (UINT64_C(1) << (cStackSize[index]+index));
            cStackSize[index] += 8;

            move.score = minimax(-cTurn, pC, pP, cStackSize, depth+1, movesPlayed+1, alpha, beta).score;
            moves.push_back(move);
            
            cStackSize[index] -= 8;
            if (cTurn == -1) pP &= ~(UINT64_C(1) << (cStackSize[index]+index));
            else pC &= ~(UINT64_C(1) << (cStackSize[index]+index));
        
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
    putTT(hashTT(pP)^hashTT(pC), moves[bMove].score);
    return moves[bMove];
}

// Keeps Track of Turns and Plays
void game() {
    // Checks Current State
    if (checkState(bC)) {
        displayGame(bC, bP);
        cout << "You LOST!" << endl;
        return;
    }
    else if (checkState(bP)) {
        cout << "You WON!" << endl;
        return;
    }
    else if (movesPlayed == 42) {
        cout << "TIE!" << endl;
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
            dist[i] = make_pair(distribution[i+stackSize[i]], i);
        }
        sort(dist.begin(), dist.end());

        for (int i = 6; i >= 0; --i) perm[6-i] = dist[i].second;

        cout << "CPU PLAY:" << endl;
        totalNodes = 0;
        hitTT = 0;
        start = high_resolution_clock::now();

        Move bM;
        for (int i = 0; i < 42-movesPlayed; ++i) {
            maxAllowedDepth = i;
            Move m;
            // Window
            int min = -512;
            int max = 512;

            m = minimax(turn, bC, bP, stackSize, 0, movesPlayed, min, max);
            if (timeOut) break;
            bM = m;
            resetTT();
        }
        cout << "Depth " << maxAllowedDepth << " Best Move: " << bM.id << " ";
        if (bM.score > 0) cout << " ";
        cout << (float(bM.score)/10) << endl;
        timeOut = false;

        bC |= (UINT64_C(1) << (stackSize[bM.id]+bM.id));
        stackSize[bM.id] += 8;

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Time Elapsed: " << duration.count() / 1000 << " Miliseconds" << endl;
        cout << "Nodes Explored: " << totalNodes << endl;
        cout << "Total Transpositions " << hitTT << endl;
        cout << minH << endl;
        cout << maxH << endl;
        minH = 100;
        maxH = -100;
        turn = -1;
        ++movesPlayed;
        game();
    } else {
        cout << "YOUR TURN:" << endl;
        displayGame(bC, bP);
        int m;
        cin >> m;
        if (m > 6 || m < 0 || stackSize[m] > 40){
            game();
            return;
        }
        if (!((bC|bP) & (UINT64_C(1) << (stackSize[m]+m)))) {
            bP |= (UINT64_C(1) << (stackSize[m]+m));
            stackSize[m] += 8;
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