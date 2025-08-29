/**
 *
 * MurmurHash3.hpp
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

#pragma once
#include "types.hpp"
#include <cstring>

namespace MurmurHash3 {
    namespace {
        #define STRTOU32(str, offset) (str[offset] | str[offset+1] << 8 | str[offset+2] << 16 | str[offset+3] << 24)

        LIBACNH_CONSTEXPR u32 rotateRight(u32 x, s8 r) { //EXTR (aka ROR) instruction in ARMv8
            return (x >> r) | (x << (32 - r));
        }

        LIBACNH_CONSTEXPR u32 Murmur32_Scramble(u32 k) {
            k = (k * 0x16A88000) | ((k * 0xCC9E2D51) >> 17);
            k *= 0x1B873593;
            return k;
        }
    }

    u32 Calc(u8* data, u32 offset, u32 size, u32 seed = 0); //ACNH 1.4.2 code: 0x7100036380
    u32 Update(u8* data, u32 hashOffset, u32 readOffset, u32 readSize);
    u32 Verify(u8* data, u32 hashOffset, u32 readOffset, u32 readSize);

#if __cplusplus > 201703L
    LIBACNH_CONSTEVAL u32 Calc_CEval(const char* str, u32 offset = 0, u32 seed = 0) {
        u32 size = strlen(str) - offset;
        u32 checksum = seed;
        const u32 nBlocks = (size / 4);

        if (size >= 4) { //Hash blocks, sizes of 4
            for (u32 i = 0; i < nBlocks; i++) {
                u32 val = STRTOU32(str, offset + i*4);
                checksum ^= Murmur32_Scramble(val);
                checksum = rotateRight(checksum, 19);
                checksum = (checksum * 5) + 0xE6546B64;
            }
        }

        if (size % 4) {
            const char* remainder = str + offset + (nBlocks*4);
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
#else
    ALWAYS_INLINE u32 Calc_CEval(const char* str, u32 offset = 0, u32 seed = 0) {
        return MurmurHash3::Calc((u8*)str+offset, 0, strlen(str) - offset, seed);
    }


#endif
}
