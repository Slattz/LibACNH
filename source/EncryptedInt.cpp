/**
 * 
 * EncryptedInt.cpp
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

#include "EncryptedInt.hpp"
#include "SeadRandom.hpp"

static const constexpr u32 EncryptConstant = 0x80E32B11;
static const constexpr u8 ShiftBase = 3;

//ACNH Encrypted Ints implementation, based on ACNH 1.4.2 and NHSE
EncryptedInt::EncryptedInt(const u32& decVal) {
    this->Set(decVal);
}

EncryptedInt::EncryptedInt(const EncVal& encVal) {
    this->Set(encVal);
}

EncryptedInt::~EncryptedInt() {

}

EncVal EncryptedInt::Set(const u32& decVal) {
    auto seadRand = sead::Random();

    encryptedVal.adjust = (seadRand.GetU32() >> 16);
    u64 val = (static_cast<u64>(seadRand.GetU32()) * static_cast<u64>(0x1B)) >> 32;
    encryptedVal.shiftval = static_cast<u8>(val);

    encryptedVal.checksum = 0;
    encryptedVal.amount = 0;
    this->Encrypt(decVal, val);
    return encryptedVal;
}

u32 EncryptedInt::Set(const EncVal& encVal) {
    encryptedVal = encVal;
    decryptedVal = Decrypt(encVal);
    return decryptedVal;
}

u64 EncryptedInt::GetEnc() {
    return encryptedVal.val;
}

u32 EncryptedInt::GetDec() {
    return decryptedVal;
}

ALWAYS_INLINE u8 CalcChecksum(u32 value) {
    u32 checksum = value + (value >> 16) + (value >> 24) + (value >> 8);
    return (u8)(checksum-0x2D);
}

EncVal EncryptedInt::Encrypt(const u32& decVal, u64 val) {
    val = static_cast<u64>(((decVal + encryptedVal.adjust) - EncryptConstant)) << (val+ShiftBase);
    encryptedVal.amount = (u32)((val>>32)+val);

    encryptedVal.checksum = CalcChecksum(encryptedVal.amount);
    return encryptedVal;
}

u32 EncryptedInt::Decrypt(const EncVal& encVal) {
    encryptedVal = encVal;
    if ((encVal.val == 0) || CalcChecksum(encVal.amount) != encVal.checksum) { //game checks EncVal parts individually, only checking val is necessary
        decryptedVal = 0;
        return decryptedVal;
    }

    u64 val = static_cast<u64>(encVal.amount) << static_cast<u64>((32-ShiftBase) - encVal.shiftval);
    u32 decryptedVal = ((EncryptConstant - encVal.adjust) + static_cast<u32>(val)) + static_cast<u32>(val >> 32);
    return decryptedVal;
}
