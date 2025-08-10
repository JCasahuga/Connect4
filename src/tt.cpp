#include "tt.h"
#include <string.h> // For memset

TranspositionTable::TranspositionTable(size_t size) : table(size, {0, 0, 0, 0, 0}) {}

unsigned int TranspositionTable::index(uint64_t key) const {
    return key % table.size();
}

void TranspositionTable::put(uint64_t key, int score, uint8_t depth, uint8_t flag, int best_move_id) {
    unsigned int i = index(key);
    // Depth-based replacement policy: only replace if new entry has greater depth
    // or if the key is the same (overwrite with new info)
    if (table[i].key == key || depth >= table[i].depth) {
        table[i].key = key;
        table[i].score = score;
        table[i].depth = depth;
        table[i].flag = flag;
        table[i].best_move_id = best_move_id;
    }
}

TTEntry TranspositionTable::get(uint64_t key) {
    unsigned int i = index(key);
    if (table[i].key == key) {
        return table[i];
    }
    return {0, 0, 0, 0, 0}; // Return a default-constructed TTEntry indicating not found
}

void TranspositionTable::reset() {
    // Using memset for performance, assuming TTEntry is a POD type.
    memset(table.data(), 0, table.size() * sizeof(TTEntry));
}

uint64_t TranspositionTable::hash(uint64_t val) {
    uint32_t l = val;
    uint32_t h = val >> 32;
    return l * randA + h * randB + randC;
}
