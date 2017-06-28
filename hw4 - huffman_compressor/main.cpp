#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>

#include "huffman_lib.h"

const size_t BLOCK_SIZE = 128000;

using std::runtime_error;
using std::string;

void read_block(vector <byte>& block, vector<byte>& init_block_size, std::ifstream& in) {
    in.read((char *) init_block_size.data(), 8);
    size_t block_size = Decoder::get_block_size(init_block_size);

    if ((in.gcount() != 0 && block_size <= 8) || block_size > 32 * BLOCK_SIZE)
        throw runtime_error("Decoding trial has failed - input file is incorrect");

    block.resize(block_size);
    in.read((char *) block.data(), block.size());
}

void write(vector <byte> const & block, std::ofstream& out) {
    out.write((char *)block.data(), block.size());
}

int main(int argc, char ** argv) {
    try {
        if (argc != 4)
            throw std::runtime_error("Expected 4 arguments");
        std::ifstream in(argv[2], std::ios::in | std::ios::binary);
        std::ofstream out(argv[3], std::ios::out | std::ios::binary);
        if (!in || !out)
            throw runtime_error("Non-existed input or output file");
        string type = argv[1];

        if (type == "enc") {
            FrequencyCounter counter;
            vector<byte> block(BLOCK_SIZE);
            while (in) {
                in.read((char *) block.data(), BLOCK_SIZE);
                if (in.gcount() == 0)
                    break;
                block.resize(in.gcount());
                counter.calc_freq(block);
            }

            if (counter.empty())
                throw runtime_error("Input file is empty");

            Encoder myEncoder;
            myEncoder.build_tree(counter);
            write(myEncoder.encode_tree(), out);

            in.clear();
            in.seekg(0);

            block.resize(BLOCK_SIZE);
            while (in) {
                in.read((char *) block.data(), BLOCK_SIZE);
                if (in.gcount() == 0)
                    break;
                block.resize(in.gcount());
                write(myEncoder.encode_block(block), out);
            }
        } else if (type == "dec") {
            Decoder myDecoder;
            vector<byte> init_block_size(8);
            vector <byte> block;

            read_block(block, init_block_size, in);

            if (in.gcount() == 0)
                throw runtime_error("Decoding trial has failed - input file is incorrect");

            myDecoder.decode_tree(block);

            while (in) {
                read_block(block, init_block_size, in);
                if (in.gcount() == 0)
                    break;
                write(myDecoder.decode_block(block), out);
            }
        } else {
            throw runtime_error("Incorrect operation");
        }
    } catch (runtime_error& e) {
        std::cout << e.what();
    }
    return 0;
}