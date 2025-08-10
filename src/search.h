#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "tt.h"
#include <chrono>

// Constants for search
const int SCORE_MIN = -2048;
const int SCORE_MAX = 2048;
const int WIN_SCORE = 512;
const int TOTAL_MOVES = 42;
const uint64_t TURN_HASH_CONSTANT = 0x123456789ABCDEF0ULL;

struct Move {
    int id;
    int score;
};

class Search {
public:
    Search();
    Move get_best_move(Position& pos, int time_limit_micro);

private:
    TranspositionTable tt;
    std::chrono::high_resolution_clock::time_point start_time;
    int time_limit_micro;
    bool time_out;
    int max_allowed_depth;
    long total_nodes;
    long hit_tt;

    Move minimax(Position& pos, uint8_t depth, int32_t alpha, int32_t beta, int8_t turn);
    int heuristic_eval(const Position& pos, int8_t turn);
    int count_total_bits(uint64_t p) const;
};

#endif // SEARCH_H
