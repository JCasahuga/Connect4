#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;
using namespace std::chrono;

uint8_t stackSize[7] = {};

int distribution[42] = {0, 1, 2, 4, 2, 1, 0,
                        0, 2, 4, 6, 4, 2, 0,
                        0, 3, 6, 8, 6, 3, 0,
                        0, 3, 6, 8, 6, 3, 0,
                        0, 2, 4, 6, 4, 2, 0,
                        0, 1, 2, 4, 2, 1, 0};

int totalNodes = 0;
int maxAllowedDepth = 0;
uint8_t movesPlayed = 0;

auto start = high_resolution_clock::now();
int timeOutMicro = 5000000;
bool timeOut;

int minH = 128;
int maxH = -128;

vector<pair<uint64_t,int>> transposition = vector<pair<uint64_t,int>>(8388593, make_pair(0b0, 0));
int randA = 827;
int randB = 683;
int randC = 38327;
int hitTT = 0;

vector<int> perm = vector<int>(7);

// Move Properties: Position and Value of the Move
class Move {
public:
    int id;
    int score;
};

// Mod Table Size
unsigned int indexTT(const uint64_t key) {
    return key%transposition.size();
}

// Returns Hash Value
uint64_t hashTT(const uint64_t val) {
    uint32_t l = val;
    uint32_t h = val >> 32;
    return l*randA + h*randB + randC;
}

// Reset TT
void resetTT() {
    memset(&transposition[0], 0, transposition.size()*sizeof(pair<uint64_t,int>));
}

// Put TT Value
void putTT(const uint64_t key, const int val) {
    unsigned int i = indexTT(key);
    transposition[i].first = key;
    transposition[i].second = val;
}

// Get TT Value
int getTT(const uint64_t key) {
    unsigned int i = indexTT(key);
    if (transposition[i].first == key)
        return transposition[i].second;
    else return -2049;
} 

// Checks If The Board Has a Winning Position
bool checkState(const uint64_t p) {
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
    while (pC) {
        ++t;
        pC &= pC - 1;
    }
    return t;
}

// Evaluates Position
int heuristicEval(const uint64_t pC, const uint64_t pP, const int8_t t) {
    // Diagonal 1
    int positive = countTotalBits(pC & (pC >> 6) & (pC >> 12) & (~pP >> 18));
    positive += countTotalBits(~pP & (pC >> 6) & (pC >> 12) & (pC >> 18));

    int negative = countTotalBits(pP & (pP >> 6) & (pP >> 12) & (~pC >> 18));
    negative += countTotalBits(~pC & (pP >> 6) & (pP >> 12) & (pP >> 18));

    // Diagonal 2
    positive += countTotalBits(pC & (pC >> 9) & (pC >> 18) & (~pP >> 27));
    positive += countTotalBits(~pP & (pC >> 9) & (pC >> 18) & (pC >> 27));

    negative += countTotalBits(pP & (pP >> 9) & (pP >> 18) & (~pC >> 27));
    negative += countTotalBits(~pC & (pP >> 9) & (pP >> 18) & (pP >> 27));

    // Horizontal
    positive += countTotalBits(pC & (pC >> 1) & (pC >> 2) & (~pP >> 3));
    positive += countTotalBits(~pP & (pC >> 1) & (pC >> 2) & (pC >> 3));

    negative += countTotalBits(pP & (pP >> 1) & (pP >> 2) & (~pC >> 3));
    negative += countTotalBits(~pC & (pP >> 1) & (pP >> 2) & (pP >> 3));

    // Vertical
    positive += countTotalBits(pC & (pC >> 8) & (pC >> 16) & (~pP >> 24)) << (t == 1);
    negative += countTotalBits(pP & (pP >> 8) & (pP >> 16) & (~pC >> 24)) << (t == -1);

    positive *= positive;
    negative *= negative;

    positive -= negative;

    positive >>= 2;

    if (positive < minH) minH = positive;
    if (positive > maxH) maxH = positive;

    if (positive >= 128) return 127;
    if (positive <= -128) return -127;

    return positive;
}

// Minimax
Move minimax(const int8_t cTurn, uint64_t pC, uint64_t pP, const uint8_t depth, const uint8_t movesPlayed, int32_t alpha, int32_t beta) {
    Move m;
    // Checks Time Out
    auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
    if (duration.count() > timeOutMicro) {
        timeOut = true;
        return m;
    }

    // Transposition Table
    int val = getTT(hashTT(pP)^hashTT(pC));
    if (val != -2049) {
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
    bool wins = false;
    if (cTurn == 1) wins = checkState(pP);
    else wins = checkState(pC);
    if (wins) {
        m.score = -cTurn*(maxAllowedDepth-depth + 2) << 7;
        return m;
    }
    if (movesPlayed == 42) return m;

    // Depth Limit
    if (depth > maxAllowedDepth) {
        Move move;
        move.score = heuristicEval(pC, pP, cTurn);
        return move;
    }

    Move bMove;
    int bScore = -2048;
    if (cTurn == -1) bScore = 2048;
    // Possible Moves
    for(int i = 0; i < 7; ++i) {
        int index = perm[i];
        if(stackSize[index] <= 40) {
            Move move = Move();
            move.id = index;

            if (cTurn == -1) pP |= (UINT64_C(1) << (stackSize[index]+index));
            else pC |= (UINT64_C(1) << (stackSize[index]+index));
            stackSize[index] += 8;

            move.score = minimax(-cTurn, pC, pP, depth+1, movesPlayed+1, alpha, beta).score;

            stackSize[index] -= 8;
            if (cTurn == -1) pP &= ~(UINT64_C(1) << (stackSize[index]+index));
            else pC &= ~(UINT64_C(1) << (stackSize[index]+index));
        
            ++totalNodes;
            // Alpha-Beta Pruning
            if (cTurn == 1) {
                alpha = max(alpha, move.score);
                if (beta <= alpha)
                    return move;
                if (bScore < move.score) {
                    bMove.id = index;
                    bScore = move.score;
                }
            } else {
                beta = min(beta, move.score);
                if (beta <= alpha)
                    return move;
                if (bScore > move.score) {
                    bMove.id = index;
                    bScore = move.score;
                }
            }
        }
    }
    putTT(hashTT(pP)^hashTT(pC), bScore);
    bMove.score = bScore;
    return bMove;
}

// Keeps Track of Turns and Plays
int getMove(int currentMove, uint64_t bC, uint64_t bP) {
    // Move Ordering
    vector<pair<int,int>> dist = vector<pair<int,int>> (7);
    for (int i = 0; i < 7; ++i) {
        int tVal = distribution[i+stackSize[i]];
        dist[i] = make_pair(distribution[i+stackSize[i]], i);
    }
    sort(dist.begin(), dist.end());

    for (int i = 6; i >= 0; --i) perm[6-i] = dist[i].second;

    //cout << "CPU PLAY:" << endl;
    totalNodes = 0;
    hitTT = 0;
    start = high_resolution_clock::now();

    Move bM;
    for (uint8_t i = 0; i < 42 - currentMove; ++i) {
        resetTT();
        maxAllowedDepth = i;
        Move m;
        // Window
        int min = -2048;
        int max = 2048;
        m = minimax(1, bC, bP, 0, currentMove, min, max);
        if (timeOut) break;
        bM = m;
    }
    
    cout << "Depth " << maxAllowedDepth << " Best Move: " << bM.id << " ";
    if (bM.score > 0) cout << " ";
    cout << (float(bM.score)/10) << endl;
    timeOut = false;
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Time Elapsed: " << duration.count() / 1000 << " Miliseconds" << endl;
    cout << "Nodes Explored: " << totalNodes << endl;
    cout << "MNodes per Second: " << double(totalNodes) / duration.count() << endl;
    cout << "Total Transpositions: " << hitTT << endl;
    cout << "Max H: " << maxH << endl;
    cout << "Min H: " << minH << endl;
    minH = 128;
    maxH = -128;
    
    return bM.id;
}

void displayGame(const uint64_t tY, const uint64_t tR) {
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
    cout << "0   1   2   3   4   5   6" << endl;
}


void ReceiveInput() {
    string currentMove;
    getline(cin, currentMove);

    int intValue = stoi(currentMove);
    //cout << currentMove << endl;

    string myPos;
    getline(cin, myPos);
    uint64_t uintValueP = stoull(myPos);
    //cout << uintValueP << endl;

    string opponentPos;
    getline(cin, opponentPos);
    uint64_t uintValueO = stoull(opponentPos);
    //cout << uintValueO << endl;

    for (int i = 0; i < 7; ++i) {
        string stack;
        getline(cin, stack);
        stackSize[i] = stoi(stack);
    }

    int move = getMove(intValue, uintValueP, uintValueO);

    uintValueP |= (UINT64_C(1) << (stackSize[move]+move));
    stackSize[move] += 8;

    displayGame(uintValueP, uintValueO);

    cout << move << endl;

    cout << "continue" << endl;

}

// Main
int main() {
    cout << setprecision(2) << fixed;
    string playTime;
    getline(cin, playTime);
    timeOutMicro = stoi(playTime);
    while (true) {
        ReceiveInput();
    }
}