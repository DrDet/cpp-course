//
// Created by drden on 24.06.2017.
//

#include <iostream>
#include <stdexcept>
#include <cassert>
#include "Decoder.h"

using std::make_pair;

Decoder::Decoder() : root(0) {
    for (size_t i = 0; i < TREE_MAX_SIZE; ++i) {
        tree[i] = make_pair(-1, -1);
        parent[i] = -1;
    }
}

void Decoder::decode_tree(vector<byte> const &buf) {
    size_t bits_steps_size = to_uint64(buf, 0);
    root = 0;
    vidx_t cur_v = root, new_v = 0;
    int bit_idx = 8, byte_idx = 8;
    bool prev_step = 0, cur_step;
    vector<vidx_t> lists_vidx;
    for (size_t i = 0; i < bits_steps_size; ++i) {
        if (cur_v == -1) {
            throw std::runtime_error("Decoding trial has failed - input file is incorrect");
        }

        bit_idx--;
        if (bit_idx < 0) {
            bit_idx = 7;
            byte_idx++;
        }

        assert(byte_idx < buf.size());
        cur_step = static_cast<bool>((buf[byte_idx] >> bit_idx) & 1);
        if (cur_step) {
            if (!prev_step)    ///(D - list)U
                lists_vidx.push_back(cur_v);
            cur_v = parent[cur_v];
        } else if (prev_step) { ///UD - вправо
            new_v++;
            parent[new_v] = cur_v;
            tree[cur_v].second = new_v;
            cur_v = new_v;
        } else {                 ///DD - влево
            new_v++;
            parent[new_v] = cur_v;
            tree[cur_v].first = new_v;
            cur_v = new_v;
        };
        std::swap(prev_step, cur_step);
    }
    byte_idx++;
    assert(lists_vidx.size() == buf.size() - byte_idx);
    for (int i = byte_idx, j = 0; static_cast<size_t>(i) < buf.size(); ++i, ++j) {
        list_data[lists_vidx[j]] = buf[i];
    }
}

vector<byte> Decoder::decode_block(vector<byte> const &block) {
    size_t bit_size = to_uint64(block, 0);
    int bit_idx = 7, byte_idx = 8;
    vector <byte> res;
    for (size_t i = 0; i < bit_size; ) {

        bit_idx--;          ///skip 0-start_bit
        if (bit_idx < 0) {
            bit_idx = 7;
            byte_idx++;
        }
        i++;

        res.push_back(decode_word(block, byte_idx, bit_idx, i));
    }
    return res;
}

byte Decoder::decode_word(const vector<byte> &block, int &byte_idx, int &bit_idx, size_t &counter) {
    int v = root;
    while (tree[v].first != -1 || tree[v].second != -1) {
        if ((block[byte_idx] >> bit_idx) & 1)
            v = tree[v].second;
        else
            v = tree[v].first;
        bit_idx--;
        counter++;
        if (bit_idx < 0) {
            bit_idx = 7;
            byte_idx++;
        }
    }
    return list_data[v];
}

size_t Decoder::to_uint64(const vector<byte> &src, size_t pos) {
    size_t res = 0;
    for (int i = 0; i < 8; ++i) {
        res <<= 8;
        res |= src[pos + i];
    }
    return res;
}

size_t Decoder::get_block_size(vector<byte> const & init_size_buf) {
    if (init_size_buf.size() == 0)
        return 0;
    return to_uint64(init_size_buf, 0);
}


