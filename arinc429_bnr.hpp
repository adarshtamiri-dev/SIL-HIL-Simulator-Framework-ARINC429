// arinc429_bnr.hpp
#pragma once

#include <cstdint>

class BnrConverter {
public:
    static double toPhysicalValue(uint32_t rawData19Bit, int numBits, double maxRange, bool isSigned) {
        // Shift data to align left within the 19-bit field
        uint32_t shifted = rawData19Bit >> (19 - numBits);

        if (isSigned) {
            uint32_t signBit = 1 << (numBits - 1);
            if (shifted & signBit) {
                // Negative value: handle Two's Complement
                int32_t signedVal = shifted - (1 << numBits);
                return (signedVal / static_cast<double>(1 << (numBits - 1))) * maxRange;
            } else {
                return (shifted / static_cast<double>(1 << (numBits - 1))) * maxRange;
            }
        } else {
            // Unsigned value calculation
            return (shifted / static_cast<double>((1 << numBits) - 1)) * maxRange;
        }
    }
};