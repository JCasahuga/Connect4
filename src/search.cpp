#include "search.h"
#include <iostream>
#include <algorithm>
#include <iomanip>

// Distribution table for heuristic evaluation, used only within this file.
namespace {
    const int distribution[42] = {0, 1, 2, 4, 2, 1, 0,
                                  1, 2, 4, 6, 4, 2, 1,
                                  2, 3, 6, 8, 6, 3, 2,
                                  2, 3, 6, 8, 6, 3, 2,
                                  1, 2, 4, 6, 4, 2, 1,
                                  0, 1, 2, 4, 2, 1, 0};
}

Search::Search() : time_out(false), max_allowed_depth(0), total_nodes(0), hit_tt(0) {}

Move Search::get_best_move(Position& pos, int time_limit_ms) {
    start_time = std::chrono::high_resolution_clock::now();
    time_limit_micro = time_limit_ms * 1000;
    time_out = false;
    total_nodes = 0;
    hit_tt = 0;

    Move best_move = {-1, -2048};

    for (int i = 1; i <= 42 - pos.movesPlayed; ++i) {
        max_allowed_depth = i;
        tt.reset();
        Move current_move = minimax(pos, 0, -2048, 2048, 1);

        if (time_out) {
            break;
        }
        best_move = current_move;
    }

    auto stop_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time);

    std::cout << "Depth " << max_allowed_depth << " Best Move: " << best_move.id << " Score: " << std::fixed << std::setprecision(2) << (float(best_move.score) / 10) << std::endl;
    std::cout << "Time Elapsed: " << duration.count() / 1000 << " Miliseconds" << std::endl;
    std::cout << "Nodes Explored: " << total_nodes << std::endl;
    std::cout << "MNodes per Second: " << double(total_nodes) / duration.count() << std::endl;
    std::cout << "Total Transpositions: " << hit_tt << std::endl;

    return best_move;
}

Move Search::minimax(Position pos, uint8_t depth, int32_t alpha, int32_t beta, int8_t turn) {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time);
    if (duration.count() > time_limit_micro) {
        time_out = true;
        return {-1, 0};
    }

    uint64_t hash_key = TranspositionTable::hash(pos.player_board) ^ TranspositionTable::hash(pos.opponent_board);
    int tt_val = tt.get(hash_key);
    if (tt_val != -2049) {
        hit_tt++;
        if (beta > tt_val) {
            beta = tt_val;
            if (alpha >= beta) {
                return {-1, tt_val};
            }
        }
    }

    if (pos.check_win(turn == 1 ? pos.opponent_board : pos.player_board)) {
        return {-1, -turn * (512 + (depth << 5))};
    }

    if (pos.movesPlayed == 42) {
        return {-1, 0};
    }

    if (depth >= max_allowed_depth) {
        return {-1, heuristic_eval(pos, turn)};
    }

    Move best_move = {-1, (turn == 1) ? -2048 : 2048};
    std::vector<int> moves = Position::get_move_order();

    for (int move_id : moves) {
        if (pos.is_legal(move_id)) {
            Position next_pos = pos;
            next_pos.make_move(move_id, turn);
            total_nodes++;

            Move current_move = minimax(next_pos, depth + 1, alpha, beta, -turn);

            if (turn == 1) { // Maximizing player
                if (current_move.score > best_move.score) {
                    best_move.score = current_move.score;
                    best_move.id = move_id;
                }
                alpha = std::max(alpha, best_move.score);
            } else { // Minimizing player
                if (current_move.score < best_move.score) {
                    best_move.score = current_move.score;
                    best_move.id = move_id;
                }
                beta = std::min(beta, best_move.score);
            }

            if (beta <= alpha) {
                break; // Pruning
            }
        }
    }

    tt.put(hash_key, best_move.score);
    return best_move;
}

int Search::count_total_bits(uint64_t p) const {
    int t = 0;
    while (p) {
        t++;
        p &= p - 1;
    }
    return t;
}

int Search::heuristic_eval(const Position& pos, int8_t turn) {
    uint64_t pC = (turn == 1) ? pos.player_board : pos.opponent_board;
    uint64_t pP = (turn == 1) ? pos.opponent_board : pos.player_board;

    // Diagonal 1
    int positive = count_total_bits(pC & (pC >> 6) & (pC >> 12) & (~pP >> 18));
    positive += count_total_bits(~pP & (pC >> 6) & (pC >> 12) & (pC >> 18));
    positive += count_total_bits(pC & (~pP >> 6) & (pC >> 12) & (pC >> 18));
    positive += count_total_bits(pC & (pC >> 6) & (~pP >> 12) & (pC >> 18));

    int negative = count_total_bits(pP & (pP >> 6) & (pP >> 12) & (~pC >> 18));
    negative += count_total_bits(~pC & (pP >> 6) & (pP >> 12) & (pP >> 18));
    negative += count_total_bits(pP & (~pC >> 6) & (pP >> 12) & (pP >> 18));
    negative += count_total_bits(pP & (pP >> 6) & (~pC >> 12) & (pP >> 18));

    // Diagonal 2
    positive += count_total_bits(pC & (pC >> 9) & (pC >> 18) & (~pP >> 27));
    positive += count_total_bits(~pP & (pC >> 9) & (pC >> 18) & (pC >> 27));
    positive += count_total_bits(pC & (~pP >> 9) & (pC >> 18) & (pC >> 27));
    positive += count_total_bits(pC & (pC >> 9) & (~pP >> 18) & (pC >> 27));

    negative += count_total_bits(pP & (pP >> 9) & (pP >> 18) & (~pC >> 27));
    negative += count_total_bits(~pC & (pP >> 9) & (pP >> 18) & (pP >> 27));
    negative += count_total_bits(pP & (~pC >> 9) & (pP >> 18) & (pP >> 27));
    negative += count_total_bits(pP & (pP >> 9) & (~pC >> 18) & (pP >> 27));

    // Horizontal
    positive += count_total_bits(pC & (pC >> 1) & (pC >> 2) & (~pP >> 3));
    positive += count_total_bits(~pP & (pC >> 1) & (pC >> 2) & (pC >> 3));
    positive += count_total_bits(pC & (~pP >> 1) & (pC >> 2) & (pC >> 3));
    positive += count_total_bits(pC & (pC >> 1) & (~pP >> 2) & (pC >> 3));

    negative += count_total_bits(pP & (pP >> 1) & (pP >> 2) & (~pC >> 3));
    negative += count_total_bits(~pC & (pP >> 1) & (pP >> 2) & (pP >> 3));
    negative += count_total_bits(pP & (~pC >> 1) & (pP >> 2) & (pP >> 3));
    negative += count_total_bits(pP & (pP >> 1) & (~pC >> 2) & (pP >> 3));

    // Vertical
    positive += count_total_bits(pC & (pC >> 8) & (pC >> 16) & (~pP >> 24));
    negative += count_total_bits(pP & (pP >> 8) & (pP >> 16) & (~pC >> 24));

    positive *= positive;
    negative *= negative;

    uint16_t combinedIndex;
    for (uint16_t index = 0; index < 42; ++index) {
        combinedIndex = index + index / 7;
        if ((pC >> combinedIndex) & 1) {
            positive += distribution[index];
        } else if ((pP >> combinedIndex) & 1) {
            negative += distribution[index];
        }
    }

    positive -= negative;

    if (positive >= 512) return 511;
    if (positive <= -512) return -511;

    return positive;
}