#include "position.h"
#include <iostream>
#include <algorithm>

Position::Position() : player_board(0), opponent_board(0), movesPlayed(0) {
    for (int i = 0; i < 7; ++i) {
        stackSize[i] = 0;
    }
}

void Position::make_move(int column, int8_t turn) {
    uint64_t move = (UINT64_C(1) << (stackSize[column] + column));
    if (turn == 1) {
        player_board |= move;
    } else {
        opponent_board |= move;
    }
    stackSize[column] += 8;
    movesPlayed++;
}

bool Position::is_legal(int column) const {
    return stackSize[column] <= 40; // 5 rows * 8 bits per row
}

bool Position::check_win(uint64_t board) const {
    // Horizontal
    if (board & (board >> 1) & (board >> 2) & (board >> 3)) return true;
    // Vertical
    if (board & (board >> 8) & (board >> 16) & (board >> 24)) return true;
    // Diag Left To Right
    if (board & (board >> 9) & (board >> 18) & (board >> 27)) return true;
    // Diag Right To Left
    if (board & (board >> 7) & (board >> 14) & (board >> 21)) return true;
    return false;
}

void Position::display() const {
    char tD[48] = {};
    for (int i = 0; i < 48; ++i) {
        if (player_board & (UINT64_C(1) << i)) tD[i] = 'X';
        else if (opponent_board & (UINT64_C(1) << i)) tD[i] = 'O';
        else tD[i] = '-';
    }

    for (int i = 5; i >= 0; --i) {
        for (int j = 0; j < 8; ++j) {
            if (j != 7) {
                std::cout << tD[i * 8 + j];
                if (j != 6) std::cout << " | ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "0   1   2   3   4   5   6" << std::endl;
}

std::vector<int> Position::get_move_order() {
    return {3, 4, 2, 5, 1, 6, 0};
}
