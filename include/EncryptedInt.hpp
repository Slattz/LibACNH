/**
 *
 * EncryptedInt.hpp
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

union EncVal {
    u64 val = 0;
    PACKED_ANON_STRUCT (
        u32 amount;
        u16 adjust;
        u8 shiftval;
        u8 checksum;
    );
};

class EncryptedInt {
public:
    EncryptedInt(const u32& decVal);
    EncryptedInt(const EncVal& encVal);
    ~EncryptedInt();

    EncVal Set(const u32& decVal);
    u32 Set(const EncVal& encVal);

    u64 GetEnc();
    u32 GetDec();

private:
    u32 decryptedVal;
    EncVal encryptedVal;

    EncVal Encrypt(const u32& decVal, u64 val);
    u32 Decrypt(const EncVal& encVal);
};
