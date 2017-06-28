//
// Created by drden on 26.06.2017.
//

#ifndef HUFFMAN_DEBUG_INIT_H
#define HUFFMAN_DEBUG_INIT_H

#include <vector>
#include <cstdint>

using std::vector;
using std::pair;

typedef uint8_t byte;
typedef int16_t vidx_t;
typedef vector<bool> encoded_word_t;

const size_t SIZE_ALPHABET = 256;
const size_t TREE_MAX_SIZE = SIZE_ALPHABET * 2 - 1;

#endif //HUFFMAN_DEBUG_INIT_H
