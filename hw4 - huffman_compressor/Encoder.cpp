//
// Created by drden on 24.06.2017.
//

#include <map>
#include <cassert>
#include <cmath>
#include "Encoder.h"

using std::make_pair;
using std::ceil;

Encoder::Encoder() {
    for (size_t i = 0; i < TREE_MAX_SIZE; ++i) {
        tree[i] = make_pair(-1, -1);
    }
}

void Encoder::build_tree(FrequencyCounter freq_cnt) {
    std::multimap <size_t, vidx_t> vacant;
    vidx_t cur_vrx = 0;
    bool correct = false;
    for (size_t i = 0; i < SIZE_ALPHABET; ++i) {
        if (freq_cnt.cnt[i]) {
            vacant.insert(make_pair(freq_cnt.cnt[i], cur_vrx++));
            correct |= true;
        }
    }
    while (vacant.size() > 1) {
        pair<size_t, vidx_t> left = *(vacant.begin());
        vacant.erase(vacant.begin());
        pair<size_t, vidx_t> right = *(vacant.begin());
        vacant.erase(vacant.begin());

        tree[cur_vrx] = make_pair(left.second, right.second);

        vacant.insert(make_pair(left.first + right.first, cur_vrx++));
    }
    root = vacant.begin()->second;

    vector<bool> buf;
    buf.push_back(0);
    init_dfs(root, buf);

    for (size_t i = 0, j = 0; i < SIZE_ALPHABET; ++i) {
        if (freq_cnt.cnt[i]) {
            code[i] = list_code[j];
            list_data[j] = static_cast<byte>(i);
            j++;
        }
    }
}

void Encoder::init_dfs(vidx_t v, vector<bool>& buf) {
    if (tree[v].first == -1 && tree[v].second == -1) {
        list_code[v] = buf;
        buf.pop_back();
        return;
    }
    vidx_t  l = tree[v].first,
            r = tree[v].second;
    buf.push_back(0);
    init_dfs(l, buf);
    buf.push_back(1);
    init_dfs(r, buf);
    buf.pop_back();
}

vector<byte> Encoder::encode_tree() {
    vector <bool> steps;
    vector <byte> list_data_in_order_dfs;
    enc_tree_dfs(root, steps, list_data_in_order_dfs);

    vector <byte> res;
    push_back_to_byte_vec(res, 8 + static_cast<size_t>(ceil(steps.size() / 8.0)) + list_data_in_order_dfs.size());
    push_back_to_byte_vec(res, steps.size());
    push_back_to_byte_vec(res, steps);
    res.insert(res.end(), list_data_in_order_dfs.begin(), list_data_in_order_dfs.end());
    assert(res.size() == 8 + 8 + static_cast<size_t>(ceil(steps.size() / 8.0)) + list_data_in_order_dfs.size());
    return res;
}

void Encoder::enc_tree_dfs(vidx_t v, vector<bool>& steps, vector<byte>& list_data_in_order_dfs) {
    if (tree[v].first == -1 && tree[v].second == -1) {
        steps.push_back(1);
        list_data_in_order_dfs.push_back(list_data[v]);
        return;
    }
    vidx_t  l = tree[v].first,
            r = tree[v].second;
    steps.push_back(0);
    enc_tree_dfs(l, steps, list_data_in_order_dfs);
    steps.push_back(0);
    enc_tree_dfs(r, steps, list_data_in_order_dfs);
    steps.push_back(1);
}

vector<byte> Encoder::encode_block(vector<byte> const &block) {
    size_t bit_size = 0;
    for (size_t i = 0; i < block.size(); ++i)
        bit_size += code[block[i]].size();

    vector <byte> res;
    push_back_to_byte_vec(res, 8 + static_cast<size_t>(ceil(bit_size / 8.0)));
    push_back_to_byte_vec(res, bit_size);

    int dst_bit_idx = -1;
    for (size_t i = 0; i < block.size(); ++i)
        dst_bit_idx = push_back_to_byte_vec(res, code[block[i]], dst_bit_idx);
    assert(res.size() == 8 + 8 + static_cast<size_t>(ceil(bit_size / 8.0)));
    return res;
}

int Encoder::push_back_to_byte_vec(vector<byte>& dst, encoded_word_t const& src, int dst_bit_idx) {
    assert(dst_bit_idx < 8);
    for (size_t i = 0; i < src.size(); ++i, --dst_bit_idx) {
        if (dst_bit_idx < 0) {
            dst_bit_idx = 7;
            dst.push_back(0);
        }
        dst.back() |= src[i] << dst_bit_idx;
    }
    return dst_bit_idx;
}

void Encoder::push_back_to_byte_vec(vector<byte>& dst, size_t src) {
    dst.resize(dst.size() + 8);
    for (int i = 0; i < 8; ++i) {
        dst[dst.size() - 1 - i] = static_cast<byte>(src & 255);
        src >>= 8;
    }
}
