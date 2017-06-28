//
// Created by drden on 24.06.2017.
//

#ifndef HUFFMAN_DEBUG_DECODER_H
#define HUFFMAN_DEBUG_DECODER_H

#include "init_.h"

#include <iostream>
using std::cerr;

class Decoder {
private:
    vidx_t root;
    pair <vidx_t, vidx_t> tree[TREE_MAX_SIZE];

    vidx_t parent[TREE_MAX_SIZE];
    byte list_data[TREE_MAX_SIZE]; ///vidx -> data_byte

    byte decode_word(const vector<byte>& block, int& byte_idx, int& bit_idx, size_t& counter);
    static size_t to_uint64(const vector<byte> &src, size_t pos);
public:
    Decoder();

    static size_t get_block_size(vector<byte> const & init_size_buf); ///length_vec = 8;

    void decode_tree(vector<byte> const & buf);
    vector<byte> decode_block(vector <byte> const & block);

    void print_tree(int v) {
        cerr << v << "\n" << tree[v].first << "\n"
                  << tree[v].second << "\n";
        if (tree[v].first == -1 && tree[v].second == -1) {
            cerr << "this is list: " << int(list_data[v]) << " " << list_data[v] << "\n~~~~~~~~~~~~~~\n";
            return;
        }
        cerr << "~~~~~~~~~~~~~~\n";
        print_tree(tree[v].first);
        print_tree(tree[v].second);
    }
};

#endif //HUFFMAN_DEBUG_DECODER_H
