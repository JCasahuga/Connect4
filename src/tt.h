#ifndef TT_H
#define TT_H

#include <cstddef>
#include <vector>
#include <utility>
#include <cstdint>

// Define TTEntry flags
const int TT_EXACT = 0;
const int TT_LOWER = 1;
const int TT_UPPER = 2;

struct TTEntry {
    uint64_t key;
    int score;
    uint8_t depth;
    uint8_t flag; // 0: exact, 1: lower bound, 2: upper bound
    int best_move_id;
};

class TranspositionTable {
public:
    TranspositionTable(size_t size = 8388593);

    void put(uint64_t key, int score, uint8_t depth, uint8_t flag, int best_move_id);
    TTEntry get(uint64_t key);
    void reset();
    static uint64_t hash(uint64_t val);

private:
    std::vector<TTEntry> table;
    unsigned int index(uint64_t key) const;

    // Hashing constants
    static constexpr int randA = 827;
    static constexpr int randB = 683;
    static constexpr int randC = 38327;
};

#endif // TT_H
