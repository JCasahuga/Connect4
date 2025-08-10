#ifndef POSITION_H
#define POSITION_H

#include <cstdint>
#include <vector>

class Position {
public:
    uint64_t player_board;
    uint64_t opponent_board;
    uint8_t stackSize[7];
    uint8_t movesPlayed;

    Position();

    void make_move(int column, int8_t turn);
    void undo_move(int column, int8_t turn);
    bool is_legal(int column) const;
    bool check_win(uint64_t board) const;
    void display() const;
    static std::vector<int> get_move_order();
};

#endif // POSITION_H
