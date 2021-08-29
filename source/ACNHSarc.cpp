/**
 * 
 * ACNHSarc.cpp
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

#include "ACNHSarc.hpp"
#include "zstd.h"
#include <cstdio>

ACNHSarc::ACNHSarc(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        InValidate("ACNHSarc: Failed to open file");
        return;
    }

    fseek(file, 0, SEEK_END);
    u64 dataSize = ftell(file);
    rewind(file);
    u8* srcData = new u8[dataSize];
    size_t res = fread(srcData, sizeof(u8), dataSize, file);
    fclose(file);

    if (res == dataSize) {
        u64 destSize = ZSTD_getFrameContentSize(srcData, dataSize);
        this->data = new u8[destSize];
        this->autoManageMem = true;

        size_t retSize = ZSTD_decompress(this->data, destSize, srcData, dataSize);
        if (!ZSTD_isError(retSize)) {
            this->Init();
        }
        else {
            InValidate("ACNHSarc: ZSTD Error");
        }
    }
    
    delete[] srcData;
}

ACNHSarc::~ACNHSarc() {
    if (autoManageMem) {
        delete[] this->data;
        autoManageMem = false;
    }
}
