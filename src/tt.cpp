#include "tt.h"
#include <string.h> // For memset

TranspositionTable::TranspositionTable(size_t size) : table(size, {0, 0}) {}

unsigned int TranspositionTable::index(uint64_t key) const {
    return key % table.size();
}

void TranspositionTable::put(uint64_t key, int value) {
    unsigned int i = index(key);
    table[i].key = key;
    table[i].value = value;
}

int TranspositionTable::get(uint64_t key) {
    unsigned int i = index(key);
    if (table[i].key == key) {
        return table[i].value;
    }
    return -2049; // Special value indicating not found
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
