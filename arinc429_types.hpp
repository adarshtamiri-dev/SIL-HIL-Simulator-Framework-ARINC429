// arinc429_types.hpp
#pragma once

#include <cstdint>
#include <iostream>

// Sign/Status Matrix (Bits 30-31)
enum class SSM : uint8_t {
    FailureWarning  = 0b00,
    NoComputedData  = 0b01,
    FunctionalTest  = 0b10,
    NormalOperation = 0b11
};

// Struct holding unpacked ARINC 429 fields
struct Arinc429Word {
    uint8_t  label;       // Bits 1-8 (Reversed to standard octal)
    uint8_t  sdi;         // Bits 9-10
    uint32_t rawData;     // Bits 11-29 (19-bit payload)
    SSM      ssm;         // Bits 30-31
    bool     parityBit;   // Bit 32
    bool     isValid;     // True if Odd Parity passes
};

class ArincUtils {
public:
    // Reverse 8 bits of label (Bit 1 <-> Bit 8, Bit 2 <-> Bit 7, etc.)
    static uint8_t reverseBits8(uint8_t b) {
        b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
        b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
        b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
        return b;
    }

    // Verify ARINC 429 Odd Parity Standard across all 32 bits
    static bool verifyOddParity(uint32_t word) {
        uint32_t count = 0;
        uint32_t temp = word;
        while (temp) {
            count += temp & 1;
            temp >>= 1;
        }
        return (count % 2) != 0; // Must be odd
    }
};