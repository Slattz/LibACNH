/**
 * 
 * MurmurHash3.cpp
 * 
 * Copyright (c) 2021-2021, Slattz.
 * 
 * This file is part of LibACNH (https://github.com/Slattz/LibACNH).
 * 
 * LibACNH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * LibACNH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with LibACNH.  If not, see <https://www.gnu.org/licenses/>
 */

#include "MurmurHash3.hpp"

#define READU32(addr) *(vu32 *)(addr)
#define WRITEU32(addr, data) *(vu32 *)(addr) = data

//MurmurHash3 implementation, based on ACNH 1.4.2
ALWAYS_INLINE uint32_t rotateRight(uint32_t x, int8_t r) { //EXTR (aka ROR) instruction in ARMv8
    return (x >> r) | (x << (32 - r));
}

ALWAYS_INLINE u32 Murmur32_Scramble(u32 k) {
    k = (k * 0x16A88000) | ((k * 0xCC9E2D51) >> 17);
    k *= 0x1B873593;
    return k;
}

u32 MurmurHash3::Calc(u8* data, u32 offset, u32 size, u32 seed) {
    u32 checksum = seed;
    const u32 nBlocks = (size / 4);

    if (size >= 4) { //Hash blocks, sizes of 4
        for (u32 i = 0; i < nBlocks; i++) {
            u32 val = READU32(data+offset);
            checksum ^= Murmur32_Scramble(val);
            checksum = rotateRight(checksum, 19);
            checksum = (checksum * 5) + 0xE6546B64;
            offset += 4;
        }
    }
    
    if (size % 4) {
        const u8* remainder = data + (nBlocks*4);
        u32 val = 0;

        switch(size & 3) { //Hash remaining bytes as size isn't always aligned by 4
            case 3: val ^= remainder[2] << 16; [[fallthrough]];
            case 2: val ^= remainder[1] << 8; [[fallthrough]];
            case 1: val ^= remainder[0]; 
                    checksum ^= Murmur32_Scramble(val);
            default: break;
        };
    }

    checksum ^= size;
    checksum ^= checksum >> 16;
    checksum *= 0x85EBCA6B;
    checksum ^= checksum >> 13;
    checksum *= 0xC2B2AE35;
    checksum ^= checksum >> 16;
    return checksum;
}

u32 MurmurHash3::Update(u8* data, u32 hashOffset, u32 readOffset, u32 readSize) {
    u32 newHash = MurmurHash3::Calc(data, readOffset, readSize);
    WRITEU32(data+hashOffset, newHash);
    return newHash;
}

u32 MurmurHash3::Verify(u8* data, u32 hashOffset, u32 readOffset, u32 readSize) {
    return MurmurHash3::Calc(data, readOffset, readSize) == READU32(data + hashOffset);
}