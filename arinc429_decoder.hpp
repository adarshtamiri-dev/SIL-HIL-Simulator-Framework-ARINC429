// arinc429_decoder.hpp
#pragma once

#include "arinc429_types.hpp"

class ArincDecoder {
public:
    static Arinc429Word decodeRawWord(uint32_t rawWord) {
        Arinc429Word w;

        // 1. Label (Bits 1-8)
        uint8_t rawLabel = rawWord & 0xFF;
        w.label = ArincUtils::reverseBits8(rawLabel);

        // 2. SDI (Bits 9-10)
        w.sdi = (rawWord >> 8) & 0x03;

        // 3. Raw 19-bit Data Field (Bits 11-29)
        w.rawData = (rawWord >> 10) & 0x7FFFF;

        // 4. SSM (Bits 30-31)
        w.ssm = static_cast<SSM>((rawWord >> 29) & 0x03);

        // 5. Parity Bit (Bit 32)
        w.parityBit = (rawWord >> 31) & 0x01;

        // 6. Check overall frame validity
        w.isValid = ArincUtils::verifyOddParity(rawWord);

        return w;
    }
};