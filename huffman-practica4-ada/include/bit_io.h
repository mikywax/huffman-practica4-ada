#pragma once
#include <cstdint>
#include <vector>

class BitWriter {
    std::vector<uint8_t> buf;
    uint8_t cur = 0;
    int nbits = 0;
    uint8_t valid_last = 8;
public:
    void writeBit(uint8_t b) {
        cur = static_cast<uint8_t>((cur << 1) | (b & 1u));
        if (++nbits == 8) { buf.push_back(cur); cur = 0; nbits = 0; }
    }
    void writeBits(uint32_t value, int n) {
        for (int i = n - 1; i >= 0; --i) writeBit((value >> i) & 1u);
    }
    void flush() {
        if (nbits > 0) {
            valid_last = static_cast<uint8_t>(nbits);
            cur = static_cast<uint8_t>(cur << (8 - nbits));
            buf.push_back(cur);
            cur = 0; nbits = 0;
        }
    }
    const std::vector<uint8_t>& data() const { return buf; }
    uint8_t lastValidBits() const { return valid_last; }
};

class BitReader {
    const std::vector<uint8_t>& buf;
    size_t pos = 0;
    int bit = 7;
    uint64_t remaining;
public:
    BitReader(const std::vector<uint8_t>& b, uint64_t total_bits)
        : buf(b), remaining(total_bits) {}
    bool readBit(uint8_t& out) {
        if (remaining == 0) return false;
        out = static_cast<uint8_t>((buf[pos] >> bit) & 1u);
        if (--bit < 0) { bit = 7; ++pos; }
        --remaining;
        return true;
    }
};