#include "position.h"
#include "search.h"
#include <iostream>
#include <string>
#include <vector>

// Global time limit set at startup
int time_limit_ms = 5000;

void receive_input() {
    std::string currentMoveStr, myPosStr, opponentPosStr;
    
    std::getline(std::cin, currentMoveStr);
    int currentMove = std::stoi(currentMoveStr);

    std::getline(std::cin, myPosStr);
    uint64_t myPos = std::stoull(myPosStr);

    std::getline(std::cin, opponentPosStr);
    uint64_t opponentPos = std::stoull(opponentPosStr);

    Position pos;
    pos.player_board = myPos;
    pos.opponent_board = opponentPos;
    pos.movesPlayed = currentMove;

    for (int i = 0; i < 7; ++i) {
        std::string stack;
        std::getline(std::cin, stack);
        pos.stackSize[i] = std::stoi(stack);
    }

    Search search;
    Move best_move = search.get_best_move(pos, time_limit_ms);

    pos.make_move(best_move.id, 1); // Assume we are player 'X' (turn 1)
    pos.display();

    std::cout << best_move.id << std::endl;
    std::cout << "continue" << std::endl;
}

int main() {
    std::string playTime;
    std::getline(std::cin, playTime);
    time_limit_ms = std::stoi(playTime);

    while (true) {
        receive_input();
    }
    return 0;
}
