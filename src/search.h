#ifndef SEARCH_H
#define SEARCH_H

#include "position.h"
#include "tt.h"
#include <chrono>

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
