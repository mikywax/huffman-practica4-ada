#include "codec.h"
#include "huffman_tree.h"
#include "bit_io.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

namespace {

constexpr char MAGIC[4] = {'H','U','F','1'};

void write_u64(std::ostream& os, uint64_t v) {
    for (int i = 0; i < 8; ++i) os.put(static_cast<char>((v >> (8*i)) & 0xFF));
}
void write_u32(std::ostream& os, uint32_t v) {
    for (int i = 0; i < 4; ++i) os.put(static_cast<char>((v >> (8*i)) & 0xFF));
}
uint64_t read_u64(std::istream& is) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
        v |= static_cast<uint64_t>(static_cast<uint8_t>(is.get())) << (8*i);
    return v;
}
uint32_t read_u32(std::istream& is) {
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i)
        v |= static_cast<uint32_t>(static_cast<uint8_t>(is.get())) << (8*i);
    return v;
}

std::vector<uint8_t> packBits(const std::vector<uint8_t>& bits, uint8_t& valid_last) {
    std::vector<uint8_t> out;
    uint8_t cur = 0; int n = 0;
    for (uint8_t b : bits) {
        cur = static_cast<uint8_t>((cur << 1) | (b & 1));
        if (++n == 8) { out.push_back(cur); cur = 0; n = 0; }
    }
    if (n > 0) {
        valid_last = static_cast<uint8_t>(n);
        cur = static_cast<uint8_t>(cur << (8 - n));
        out.push_back(cur);
    } else {
        valid_last = 8;
    }
    return out;
}

std::vector<uint8_t> unpackBits(const std::vector<uint8_t>& bytes, uint8_t valid_last) {
    std::vector<uint8_t> out;
    if (bytes.empty()) return out;
    for (size_t i = 0; i < bytes.size(); ++i) {
        uint8_t byte = bytes[i];
        int limit = 8;
        if (i == bytes.size() - 1 && valid_last >= 1 && valid_last <= 8) limit = valid_last;
        for (int b = 7; b >= 8 - limit; --b) out.push_back((byte >> b) & 1);
    }
    return out;
}

}

namespace huf {

bool compress(const std::string& in_path, const std::string& out_path) {
    std::ifstream in(in_path, std::ios::binary);
    if (!in) { std::fprintf(stderr, "No se puede abrir %s\n", in_path.c_str()); return false; }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)),
                              std::istreambuf_iterator<char>());
    in.close();

    HuffmanTree::FreqTable freq{};
    for (uint8_t c : data) freq[c]++;

    auto root  = HuffmanTree::build(freq);
    auto codes = HuffmanTree::buildCodes(root);

    std::vector<uint8_t> tree_bits;
    uint32_t tree_bit_len = 0;
    HuffmanTree::serialize(root, tree_bits, tree_bit_len);
    uint8_t tree_valid = 8;
    auto tree_bytes = packBits(tree_bits, tree_valid);

    std::vector<uint8_t> payload_bytes;
    uint8_t payload_valid = 0;
    if (!data.empty() && root && !root->isLeaf()) {
        BitWriter bw;
        for (uint8_t c : data) {
            const std::string& code = codes[c];
            for (char b : code) bw.writeBit(b == '1' ? 1 : 0);
        }
        bw.flush();
        payload_bytes = bw.data();
        payload_valid = bw.lastValidBits();
    }
    // vacío o single-symbol: payload_valid = 0, payload_bytes vacío

    std::ofstream out(out_path, std::ios::binary);
    if (!out) { std::fprintf(stderr, "No se puede escribir %s\n", out_path.c_str()); return false; }
    out.write(MAGIC, 4);
    write_u64(out, data.size());
    write_u32(out, tree_bit_len);
    out.put(static_cast<char>(payload_valid));
    if (!tree_bytes.empty())
        out.write(reinterpret_cast<const char*>(tree_bytes.data()), tree_bytes.size());
    if (!payload_bytes.empty())
        out.write(reinterpret_cast<const char*>(payload_bytes.data()), payload_bytes.size());
    return true;
}

bool decompress(const std::string& in_path, const std::string& out_path) {
    std::ifstream in(in_path, std::ios::binary);
    if (!in) { std::fprintf(stderr, "No se puede abrir %s\n", in_path.c_str()); return false; }

    char magic[4];
    in.read(magic, 4);
    if (std::memcmp(magic, MAGIC, 4) != 0) { std::fprintf(stderr, "Magic inválido\n"); return false; }
    uint64_t original_size = read_u64(in);
    uint32_t tree_bit_len   = read_u32(in);
    uint8_t  payload_valid  = static_cast<uint8_t>(in.get());
    if (!in) { std::fprintf(stderr, "Header truncado\n"); return false; }

    size_t tree_bytes_len = (tree_bit_len + 7) / 8;
    std::vector<uint8_t> tree_bytes(tree_bytes_len);
    if (tree_bytes_len > 0)
        in.read(reinterpret_cast<char*>(tree_bytes.data()), tree_bytes_len);
    if (!in) { std::fprintf(stderr, "Árbol truncado\n"); return false; }

    std::vector<uint8_t> payload_bytes((std::istreambuf_iterator<char>(in)),
                                        std::istreambuf_iterator<char>());
    in.close();

    std::shared_ptr<HuffmanNode> root;
    if (tree_bit_len > 0) {
        auto tree_bits = unpackBits(tree_bytes, 8);
        root = HuffmanTree::deserialize(tree_bits, 0, tree_bit_len);
        if (!root) { std::fprintf(stderr, "Árbol inválido\n"); return false; }
    }

    std::ofstream out(out_path, std::ios::binary);
    if (!out) { std::fprintf(stderr, "No se puede escribir %s\n", out_path.c_str()); return false; }

    if (original_size == 0) return true;
    if (!root) { std::fprintf(stderr, "Sin árbol\n"); return false; }

    if (root->isLeaf()) {
        for (uint64_t i = 0; i < original_size; ++i)
            out.put(static_cast<char>(root->character));
        return true;
    }

    if (payload_valid == 0) { std::fprintf(stderr, "Payload vacío inesperado\n"); return false; }
    auto payload_bits = unpackBits(payload_bytes, payload_valid);
    size_t bit_pos = 0;
    const HuffmanNode* cur = root.get();
    uint64_t decoded = 0;
    while (decoded < original_size) {
        if (bit_pos >= payload_bits.size()) { std::fprintf(stderr, "Flujo truncado\n"); return false; }
        uint8_t b = payload_bits[bit_pos++];
        cur = (b == 0) ? cur->left.get() : cur->right.get();
        if (!cur) { std::fprintf(stderr, "Árbol inválido en decodificación\n"); return false; }
        if (cur->isLeaf()) {
            out.put(static_cast<char>(cur->character));
            ++decoded;
            cur = root.get();
        }
    }
    return true;
}

}