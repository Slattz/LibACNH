/**
 * 
 * SaveCrypto.cpp
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

#include "types.hpp"
#include "SeadRandom.hpp"
#include "SaveCrypto.hpp"
#ifdef __SWITCH__
#include <switch.h>
#else
#include "aes.h"
#endif

#define AES_BLOCK_LENGTH 0x10 //128 bits

void SaveCrypto::RegenHeaderCrypto(GSaveVersion& header, const u32 seed) {
    sead::Random rand = sead::Random(seed);
    for (u32 i = 0; i < HEADER_CRYPTO_SIZE; i++)
        header.headerCrypto[i] = rand.GetU32();
}

ALWAYS_INLINE void GetParam(u8* outParam, const u32 data[], const int index) {
    sead::Random rand = sead::Random(data[data[index] & 0x7F]);
    u32 rngRoll = (data[data[index + 1] & 0x7F] & 0xF) + 1;

    for (u32 i = 0; i < rngRoll; i++)
        rand.GetU64();

    for (u32 i = 0; i < AES_BLOCK_LENGTH; i++)
        outParam[i] = (u8)(rand.GetU32() >> 24);
}

void SaveCrypto::Crypt(const GSaveVersion& header, u8* encryptedSave, const u32 saveSize) {
    u8 key[AES_BLOCK_LENGTH] = {0};
    u8 counter[AES_BLOCK_LENGTH] = {0};
    GetParam(key, header.headerCrypto, 0); //Get AES Key
    GetParam(counter, header.headerCrypto, 2); // Get AES CTR

#ifdef __SWITCH__ //if Switch, use specific AES hardware
    Aes128CtrContext ctx;
    aes128CtrContextCreate(&ctx, key, counter);
    aes128CtrCrypt(&ctx, encryptedSave, encryptedSave, saveSize); //Crypt In-Place
#else //software fallback for other platforms
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, counter);
    AES_CTR_xcrypt_buffer(&ctx, encryptedSave, saveSize); //Crypt In-Place
#endif
}
