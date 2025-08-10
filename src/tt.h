#ifndef TT_H
#define TT_H

#include <cstddef>
#include <vector>
#include <utility>
#include <cstdint>

class TranspositionTable {
public:
    TranspositionTable(size_t size = 8388593);

    void put(uint64_t key, int value);
    int get(uint64_t key);
    void reset();
    static uint64_t hash(uint64_t val);

private:
    struct TTEntry {
        uint64_t key;
        int value;
    };
    std::vector<TTEntry> table;
    unsigned int index(uint64_t key) const;

    // Hashing constants
    static constexpr int randA = 827;
    static constexpr int randB = 683;
    static constexpr int randC = 38327;
};

#endif // TT_H
