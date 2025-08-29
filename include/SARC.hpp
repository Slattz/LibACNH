/**
 *
 * SARC.hpp
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
#include <vector>

struct SARCFileInfo {
    const char* name;
    u8* dataAddress;
    u32 dataSize;
};

class SARC
{
protected:
    inline void InValidate(const char *message)
    {
        this->isValid = false;
        this->errorMessage = message;
    }

    ALWAYS_INLINE u16 ReadU16(void *address) const
    {
        u16 val = *(u16 *)(address);
        if (bigEndian)
            return __builtin_bswap16(val);
        return val;
    }

    ALWAYS_INLINE u32 ReadU32(void *address) const
    {
        u32 val = *(u32 *)(address);
        if (bigEndian)
            return __builtin_bswap32(val);
        return val;
    }

    void Init();
    void Parse();

    u8 *data = nullptr;
    u64 dataSize = 0;
    const char *errorMessage = "No Error";
    bool autoManageMem = false;

    bool bigEndian = false;
    bool isValid = true;
    u32 dataOffset = 0;
    u32 nameTableAddress = 0;
    u32 nodeAddress = 0;
    u16 nodeCount = 0;

    std::vector<SARCFileInfo> fileInfo;

public:
    SARC();
    SARC(const char *filePath);
    SARC(u8 *inBuffer, u64 bufSize, bool manageMem = false);
    virtual ~SARC();
    bool IsValid() const;
    const char *GetErrorMessage() const;

    void Print();
    bool GetFile(const char* outFile, const char* sarcFilePath);
};
