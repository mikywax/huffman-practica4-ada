#include "huffman_tree.h"
#include <queue>

namespace {
struct Cmp {
    bool operator()(const std::shared_ptr<HuffmanNode>& a,
                    const std::shared_ptr<HuffmanNode>& b) const {
        if (a->frequency != b->frequency) return a->frequency > b->frequency;
        return a->id > b->id;
    }
};
}

std::shared_ptr<HuffmanNode> HuffmanTree::build(const FreqTable& freq) {
    std::priority_queue<std::shared_ptr<HuffmanNode>,
                        std::vector<std::shared_ptr<HuffmanNode>>, Cmp> pq;
    uint64_t next_id = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            auto n = std::make_shared<HuffmanNode>(static_cast<uint8_t>(i), freq[i]);
            n->id = next_id++;
            pq.push(n);
        }
    }
    if (pq.empty()) return nullptr;
    if (pq.size() == 1) return pq.top();
    while (pq.size() > 1) {
        auto a = pq.top(); pq.pop();
        auto b = pq.top(); pq.pop();
        auto parent = std::make_shared<HuffmanNode>(a, b);
        parent->id = next_id++;
        pq.push(parent);
    }
    return pq.top();
}

HuffmanTree::CodeTable HuffmanTree::buildCodes(const std::shared_ptr<HuffmanNode>& root) {
    CodeTable codes;
    if (!root) return codes;
    struct Frame { std::shared_ptr<HuffmanNode> node; std::string prefix; };
    std::vector<Frame> stack;
    stack.push_back({root, ""});
    while (!stack.empty()) {
        auto f = std::move(stack.back()); stack.pop_back();
        if (f.node->isLeaf()) {
            codes[f.node->character] = f.prefix.empty() ? "0" : f.prefix;
        } else {
            stack.push_back({f.node->right, f.prefix + "1"});
            stack.push_back({f.node->left,  f.prefix + "0"});
        }
    }
    return codes;
}

static void serialize_rec(const std::shared_ptr<HuffmanNode>& n,
                          std::vector<uint8_t>& out, uint32_t& len) {
    if (n->isLeaf()) {
        out.push_back(1);
        for (int i = 7; i >= 0; --i) out.push_back((n->character >> i) & 1);
        len += 9;
    } else {
        out.push_back(0); ++len;
        serialize_rec(n->left,  out, len);
        serialize_rec(n->right, out, len);
    }
}

void HuffmanTree::serialize(const std::shared_ptr<HuffmanNode>& root,
                            std::vector<uint8_t>& out_bits, uint32_t& bit_len) {
    out_bits.clear(); bit_len = 0;
    if (root) serialize_rec(root, out_bits, bit_len);
}

static std::shared_ptr<HuffmanNode> deserialize_rec(const std::vector<uint8_t>& in,
                                                    size_t& pos, size_t end) {
    if (pos >= end) return nullptr;
    uint8_t bit = in[pos++];
    if (bit == 1) {
        if (pos + 8 > end) return nullptr;
        uint8_t c = 0;
        for (int i = 0; i < 8; ++i) c = static_cast<uint8_t>((c << 1) | (in[pos++] & 1));
        return std::make_shared<HuffmanNode>(c, 0);
    }
    auto left  = deserialize_rec(in, pos, end);
    auto right = deserialize_rec(in, pos, end);
    if (!left || !right) return nullptr;
    return std::make_shared<HuffmanNode>(left, right);
}

std::shared_ptr<HuffmanNode> HuffmanTree::deserialize(const std::vector<uint8_t>& in,
                                                      size_t start_bit, uint32_t bit_len) {
    size_t pos = start_bit;
    return deserialize_rec(in, pos, start_bit + bit_len);
}