#pragma once
#include <cstdint>
#include <memory>

struct HuffmanNode {
    uint8_t character;
    uint64_t frequency;
    uint64_t id = 0;
    bool leaf;
    std::shared_ptr<HuffmanNode> left, right;

    HuffmanNode(uint8_t ch, uint64_t freq)
        : character(ch), frequency(freq), leaf(true), left(nullptr), right(nullptr) {}

    HuffmanNode(std::shared_ptr<HuffmanNode> l, std::shared_ptr<HuffmanNode> r)
        : character(0), frequency(l->frequency + r->frequency),
          leaf(false), left(std::move(l)), right(std::move(r)) {}

    bool isLeaf() const { return leaf; }
};