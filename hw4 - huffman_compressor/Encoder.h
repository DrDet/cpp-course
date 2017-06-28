//
// Created by drden on 24.06.2017.
//

#ifndef HUFFMAN_DEBUG_ENCODER_H
#define HUFFMAN_DEBUG_ENCODER_H

#include <iostream>
using std::cerr;

#include "init_.h"
#include "FrequencyCounter.h"

class Encoder {
private:
    vidx_t root;
    pair<vidx_t, vidx_t> tree[TREE_MAX_SIZE];


    /// assert: idx_list < SIZE_ALPHABET
    byte list_data[SIZE_ALPHABET];           ///vidx -> byte_data
    encoded_word_t list_code[SIZE_ALPHABET]; ///vidx -> code

    encoded_word_t code[SIZE_ALPHABET];      ///data -> code


    void init_dfs(vidx_t v, vector<bool>& buf);
    void enc_tree_dfs(vidx_t v, vector <bool> & steps, vector <byte> & lists);
    static void push_back_to_byte_vec(vector <byte>& dst, size_t src);
    static int push_back_to_byte_vec(vector <byte>& dst, encoded_word_t const& src, int dst_bit_idx = -1);
public:
    Encoder();

    void build_tree(FrequencyCounter freq_cnt);
    vector<byte> encode_tree(); /// /*byte[8] byte_size_of_encoded*/, byte[8] size_descr_bits, byte[] tree_descr, byte[] lists
    vector<byte> encode_block(vector<byte> const & block);  /// /*byte[8] byte_size_of_encoded*/, byte[8] size_block_bits, byte[] encoded_blocks
    void print_tree(vidx_t v) {
        std::cerr << v << "\n" << tree[v].first << " " << tree[v].second << "\n";
        if (tree[v].first == -1 && tree[v].second == -1) {
            cerr << "this is list: " << int(list_data[v]) << " " << list_data[v] << "\n~~~~~~~~~~~~~~\n";
            return;
        }
        cerr << "~~~~~~~~~~~~~~\n";
        print_tree(tree[v].first);
        print_tree(tree[v].second);
    }
};

#endif //HUFFMAN_DEBUG_ENCODER_H
