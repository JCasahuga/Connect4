#include "position.h"
#include "search.h"
#include <iostream>
#include <string>
#include <vector>
// Global time limit set at startup
void receive_input(int time_limit_micro) {
    std::string line;
    try {
        std::getline(std::cin, line);
        if (line.empty() && !std::cin.good()) return;
        int currentMove = std::stoi(line);

        std::getline(std::cin, line);
        uint64_t myPos = std::stoull(line);

        std::getline(std::cin, line);
        uint64_t opponentPos = std::stoull(line);

        Position pos;
        pos.player_board = myPos;
        pos.opponent_board = opponentPos;
        pos.movesPlayed = currentMove;

        for (int i = 0; i < 7; ++i) {
            std::getline(std::cin, line);
            pos.stackSize[i] = std::stoi(line);
        }

        Search search;
        Move best_move = search.get_best_move(pos, time_limit_micro);

        pos.make_move(best_move.id, 1); // Assume we are player 'X' (turn 1)
        pos.display();

        std::cout << best_move.id << std::endl;
        std::cout << "continue" << std::endl;

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error parsing input line: '" << line << "'. Exception: " << e.what() << std::endl;
        return;
    }
}

int main() {
    std::string playTime;
    if (std::getline(std::cin, playTime)) {
        try {
            int time_limit_micro = std::stoi(playTime);
            while (std::cin.good()) {
                receive_input(time_limit_micro);
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error parsing time limit: '" << playTime << "'. Exiting." << std::endl;
            return 1;
        }
    } else {
        return 1;
    }

    return 0;
}
