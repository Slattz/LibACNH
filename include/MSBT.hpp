/**
 *
 * MSBT.hpp
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
#include <string>
#include <vector>
#include <map>

enum MSBTEncoding : u8 {
    Encoding_UTF8 = 0,
    Encoding_UTF16 = 1,
    Encoding_None
};

struct MSBTString {
    u8* stringBytes;
    u32 stringSize;
    u32 index;
    MSBTEncoding encoding;
};

class MSBT
{
protected:
    inline void InValidate(const char *message)
    {
        this->isValid = false;
        this->errorMessage = message;
    }

    ALWAYS_INLINE u32 AlignUp(u32 offset, s32 size) {
        return offset + ((size - (offset % size)) % size);
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
    void ParseTXT2(std::vector<MSBTString>& Texts, u32 dataPos, u32 sectionSize);
    void ParseLBL1(std::vector<MSBTString>& Labels, u32 dataPos);
    void Parse();

    u8* data = nullptr;
    u64 dataSize = 0;
    const char* errorMessage = "No Error";
    bool autoManageMem = false;

    bool bigEndian = false;
    bool isValid = true;

    MSBTEncoding encoding = Encoding_None;
    u16 sectionCount = 0;

    std::map<std::string, std::string> stringMap;

public:
    MSBT(const char *filePath);
    MSBT(u8 *inBuffer, u64 bufSize, bool manageMem = false);
    virtual ~MSBT();
    bool IsValid() const;
    const char *GetErrorMessage() const;

    bool GetAll(std::map<std::string, std::string>& stringMap);
    bool Get(const char* label, std::string& text);

    void Print();
};
