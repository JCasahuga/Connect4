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

Move Search::get_best_move(Position& pos, int time_limit_micro) {
    start_time = std::chrono::high_resolution_clock::now();
    this->time_limit_micro = time_limit_micro;
    time_out = false;
    total_nodes = 0;
    hit_tt = 0;

    Move best_move = {-1, SCORE_MIN};
    tt.reset();

    for (int i = 1; i <= TOTAL_MOVES - pos.movesPlayed; ++i) {
        max_allowed_depth = i;
        Move current_move = minimax(pos, 0, SCORE_MIN, SCORE_MAX, 1);
        
        if (time_out) {
            break;
        }
        best_move = current_move;
    }

    auto stop_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time);
    
    std::cout << "Depth " << max_allowed_depth << " Best Move: " << best_move.id 
              << " Score: " << std::fixed << std::setprecision(2) << (float(best_move.score) / 10) << std::endl;
    std::cout << "Time Elapsed: " << duration.count() / 1000 << " Miliseconds" << std::endl;
    std::cout << "Nodes Explored: " << total_nodes << std::endl;
    std::cout << "MNodes per Second: " << double(total_nodes) / duration.count() << std::endl;
    std::cout << "Total Transpositions: " << hit_tt << std::endl;
    
    return best_move;
}

Move Search::minimax(Position& pos, uint8_t depth, int32_t alpha, int32_t beta, int8_t turn) {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time);
    if (duration.count() > time_limit_micro) {
        time_out = true;
        return {-1, 0};
    }

    uint64_t hash_key = TranspositionTable::hash(pos.player_board) ^ TranspositionTable::hash(pos.opponent_board) ^ (turn * TURN_HASH_CONSTANT);
    TTEntry tt_entry = tt.get(hash_key);
    if (tt_entry.depth != 0 && tt_entry.depth >= max_allowed_depth - depth) {
        hit_tt++;
        if (tt_entry.flag == TT_EXACT || 
            (tt_entry.flag == TT_LOWER && tt_entry.score >= beta) ||
            (tt_entry.flag == TT_UPPER && tt_entry.score <= alpha)) {
            return {tt_entry.best_move_id, tt_entry.score};
        }
    }

    if (pos.check_win(turn == 1 ? pos.opponent_board : pos.player_board)) {
        return {-1, -turn * (WIN_SCORE + (depth << 5))};
    }

    if (pos.movesPlayed == TOTAL_MOVES) {
        return {-1, 0};
    }

    if (depth >= max_allowed_depth) {
        return {-1, heuristic_eval(pos, turn)};
    }

    Move best_move = {-1, (turn == 1) ? SCORE_MIN : SCORE_MAX};
    std::vector<int> ordered_moves;
    std::vector<int> all_moves = Position::get_move_order();

    // Prioritize the move from the Transposition Table if available and legal
    int tt_best_move_id = -1;
    if (tt_entry.best_move_id != -1 && pos.is_legal(tt_entry.best_move_id)) {
        tt_best_move_id = tt_entry.best_move_id;
        ordered_moves.push_back(tt_best_move_id);
    }

    for (int move_id : all_moves) {
        if (move_id != tt_best_move_id && pos.is_legal(move_id)) {
            ordered_moves.push_back(move_id);
        }
    }

    for (int move_id : ordered_moves) {
        if (pos.is_legal(move_id)) {
            pos.make_move(move_id, turn);
            total_nodes++;

            Move current_move = minimax(pos, depth + 1, alpha, beta, -turn);

            pos.undo_move(move_id, turn);

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

    uint8_t flag = TT_EXACT;
    if (best_move.score <= alpha) {
        flag = TT_UPPER;
    } else if (best_move.score >= beta) {
        flag = TT_LOWER;
    }
    tt.put(hash_key, best_move.score, max_allowed_depth - depth, flag, best_move.id);
    return best_move;
}

int Search::count_total_bits(uint64_t p) const {
    return __builtin_popcountll(p);
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

    if (positive >= WIN_SCORE) return WIN_SCORE -1;
    if (positive <= -WIN_SCORE) return -WIN_SCORE + 1;

    return positive;
}