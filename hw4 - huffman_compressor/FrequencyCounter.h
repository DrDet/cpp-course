//
// Created by drden on 24.06.2017.
//

#ifndef HUFFMAN_DEBUG_FREQUENCYCOUNTER_H
#define HUFFMAN_DEBUG_FREQUENCYCOUNTER_H

#include "init_.h"

class FrequencyCounter {
    friend class Encoder;
private:
    size_t cnt[SIZE_ALPHABET];
    bool isEmpty;
public:
    FrequencyCounter();
    void calc_freq(vector <byte> const & block);
    bool empty();
};


#endif //HUFFMAN_DEBUG_FREQUENCYCOUNTER_H
