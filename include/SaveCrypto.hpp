/**
 *
 * SaveCrypto.hpp
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

#define HEADER_CRYPTO_SIZE 0x80
struct GSaveVersion {
    u32 Major;
    u32 Minor;
    u32 SaveHeaderVersion; //Read as a u32 in the ACNH 1.4.2 (0x7101241ACC) and 1.0.0 (0x7100FD8274)
    u16 SaveCryptoVersion;
    u16 RomReleaseVersion; //Code checks if this is not 999 when code's is not 999; otherwise if code's is 999, then code checks if the value is 999; ctor defaults to 0xFFFF
    u8 _ReserveField[0x30];

    //Header Info of when save was created
    u32 Created_Major;
    u32 Created_Minor;
    u32 Created_SaveHeaderVersion;
    u16 Created_SaveCryptoVersion;
    u16 Created_RomReleaseVersion;
    u8 Padding2[0xB0];

    u32 headerCrypto[HEADER_CRYPTO_SIZE];
};

static_assert((sizeof(GSaveVersion) == 0x300), "GSaveVersion is not 0x300 in size!");

namespace SaveCrypto {
    void RegenHeaderCrypto(GSaveVersion& header);
    void RegenHeaderCrypto(GSaveVersion& header, const u32 seed);
    void Crypt(const GSaveVersion& header, u8* encryptedSave, const u32 saveSize);
}
