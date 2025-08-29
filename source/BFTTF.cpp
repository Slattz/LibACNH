/**
 *
 * BFTTF.cpp
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

#include "BFTTF.hpp"
#include <cstdio>
#include "types.hpp"

bool BFTTF::Decrypt(const char* outFilePath, const char* inFilePath) {
    if (!outFilePath || !inFilePath)
        return false;

    FILE* inFile = fopen(inFilePath, "r");
    if (inFile == NULL) {
        return false;
    }

    fseek(inFile, 0, SEEK_END);
    u64 dataSize = ftell(inFile);
    rewind(inFile);

    u32 magic = 0;
    fread(&magic, sizeof(u32), 1, inFile);

    u32 decKey;
    switch (magic) {
    case 0x1A879BD9: //Windows
        decKey = 0xA6018502U;
        break;
    case 0x1E1AF836: //Switch
        decKey = 0x49621806U;
        break;
    case 0xC1DE68F3: //Wii U
        decKey = 0x8CF2DCD9U;
        break;
    default:
        decKey = 0;
        break;
    }

    if (decKey == 0) {
        fclose(inFile);
        return false;
    }

    u32 fontSize = 0;
    fread(&fontSize, sizeof(u32), 1, inFile);
    fontSize = __builtin_bswap32(fontSize);

    fontSize ^= decKey;

    if (fontSize == 0 || fontSize > dataSize) {
        fclose(inFile);
        return false;
    }

    FILE* outFile = fopen(outFilePath, "w");
    if (outFile == NULL) {
        fclose(inFile);
        return false;
    }

    for (u32 i = 0; i < fontSize && i < dataSize; i+=sizeof(u32)) {
        u32 value = 0;
        fread(&value, sizeof(u32), 1, inFile);
        value = __builtin_bswap32(value);
        value ^= decKey;
        value = __builtin_bswap32(value);
        fwrite(&value, sizeof(u32), 1, outFile);
    }

    fclose(inFile);
    fclose(outFile);
    return true;
}
