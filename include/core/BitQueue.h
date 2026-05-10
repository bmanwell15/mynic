#ifndef BITQUEUE_H
#define BITQUEUE_H

#include <span>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdint>
#include <optional>

/** BitQueue Class
 * This class represents a stream of bits which can be popped from the stream to be interpreted.
 * 
 * BitQueue(std::vector<uint8_t> data)
 *    - Constructs a new queue given a stream of bytes.
 * 
 * bool empty()
 *    - Returns true if there are no more bits in the stream, false otherwise.
 * 
 * size_t size()
 *    - Returns the number of BITS (not Bytes) left in the stream.
 * 
 * uint64_t pop(size_t n)
 *    - Pops and returns the number of specified bits. If n is over 64, then only 64 bits will be returned.
 * 
 * uint64_t peet(size_t n)
 *    - Returns the next n bits of the stream without changing the stream itself. If n is over 64, then only 64 bits will be returned.
 * 
 * void rewind(size_t bits)
 *    - Restore the number of bits. For example, if 5 bits have been popped, then rewind(5) will restore the queue back to the original state.
 * 
 * size_t bitPot()
 *    - Returns the bit position of the stream.
 * 
 * PRIVATE METHODS
 * uint64_t readBits(size_t n)
 *    - Reads and returns the next n bits.
 */
class BitQueue {
public:
    BitQueue() : bitPos_(0) {}
    explicit BitQueue(std::vector<uint8_t> data)
        : owned_(std::move(data)),
          data_(owned_),
          bitPos_(0) {}

    bool empty() const {
        return bitPos_ >= data_.size() * 8;
    }

    size_t size() const {
        return data_.size() * 8 - bitPos_;
    }

    std::optional<uint64_t> pop(size_t n) {
        if (n > 64) n = 64;
        return readBits(n);
    }

    // uint64_t peek(size_t n) const {
    //     if (n > 64) n = 64;
    //     size_t savedPos = bitPos_;
    //     auto val = const_cast<BitQueue*>(this)->readBits(n);
    //     const_cast<BitQueue*>(this)->bitPos_ = savedPos;
    //     return val;
    // }

    void rewind(size_t bits) {
        if (bits > bitPos_) {
            bitPos_ = 0;
            return;
        }
        bitPos_ -= bits;
    }

    size_t bitPos() const {
        return bitPos_;
    }

    void setBitPos(size_t newBitPos) {
        bitPos_ = newBitPos;
    }

private:
    std::optional<uint64_t> readBits(size_t n) {
        if (n == 0 || n > 64 || bitPos_ + n > data_.size() * 8)
            return std::nullopt;

        uint64_t result = 0;
        size_t remaining = n;

        while (remaining > 0) {
            size_t byteIndex = bitPos_ / 8;
            size_t bitOffset = bitPos_ % 8;

            size_t bitsAvailable = 8 - bitOffset;
            size_t take = std::min(bitsAvailable, remaining);

            uint8_t byte = data_[byteIndex];
            uint8_t mask = ((1u << take) - 1) << (bitsAvailable - take);
            uint8_t bits = (byte & mask) >> (bitsAvailable - take);

            result = (result << take) | bits;

            bitPos_ += take;
            remaining -= take;
        }

        return result;
    }

    std::vector<uint8_t> owned_; // Stores a vector of the raw bytes.
    std::span<const uint8_t> data_; // Stores the raw bytes as a span object
    size_t bitPos_; // Stores the index of the next bit to be popped/peeked
};

#endif