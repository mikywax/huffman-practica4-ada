#pragma once
#include "huffman_node.h"
#include <array>
#include <memory>
#include <string>
#include <vector>

class HuffmanTree {
public:
    using FreqTable = std::array<uint64_t, 256>;
    using CodeTable = std::array<std::string, 256>;

    static std::shared_ptr<HuffmanNode> build(const FreqTable& freq);
    static CodeTable buildCodes(const std::shared_ptr<HuffmanNode>& root);

    static void serialize(const std::shared_ptr<HuffmanNode>& root,
                          std::vector<uint8_t>& out_bits, uint32_t& bit_len);
    static std::shared_ptr<HuffmanNode> deserialize(const std::vector<uint8_t>& in,
                                                    size_t start_bit, uint32_t bit_len);
};