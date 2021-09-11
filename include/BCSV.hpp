/**
 * 
 * BCSV.hpp
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

// Copyright (C) 2021 Slattz
// 
// This file is part of LibACNH.
// 
// LibACNH is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// LibACNH is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with LibACNH.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "types.hpp"
#include <vector>
#include <map>

enum class ColumnType : u8 {
    UInt8,
    UInt16,
    UInt32,
    Float,
    String
};

struct BCSVField {
    ColumnType type;

    union {
        u8 UInt8;
        u16 UInt16;
        u32 UInt32;
        float Float;
        const char* String;
    };

    void Print() const;
};

typedef std::map<u32, BCSVField> BCSVRow;
typedef std::vector<BCSVRow> BCSVData;

class BCSV {
protected:
    inline void InValidate(const char* message) {
        this->isValid = false;
        this->errorMessage = message;
    }

    ALWAYS_INLINE u16 ReadU16(void* address) const {
        u16 val = *(u16 *)(address);
        return val;
    }

    ALWAYS_INLINE u32 ReadU32(void* address) const {
        u32 val = *(u32 *)(address);
        return val;
    }

    ALWAYS_INLINE float ReadFloat(void* address) const {
        float val = *(float*)(address);
        return val;
    }

    ALWAYS_INLINE bool IsFloat(u32 val) const {
        if (val == 0) return false; //Assume 0 is u32

        float fVal = ReadFloat(&val); //Converts hex representation to actual float val, e.g. 0x3F800000 == 1.0f
        return fVal >= std::numeric_limits<float>::min() && fVal <= std::numeric_limits<float>::max();
    }

    void Init();
    void Parse();

    u8 *data = nullptr;
    u64 dataSize = 0;
    const char *errorMessage = "No Error";
    bool autoManageMem = false;
    bool isValid = true;

    u32 numRows = 0;
    u32 rowSize = 0;
    u16 numColumns = 0;
    u8 version = 0xFF;
    bool jpEnumFlag = false;
    u32 startPos = 0;

    BCSVData csvData;

public:
    BCSV(const char* filePath);
    BCSV(u8* inBuffer, u64 bufSize, bool manageMem = false);
    virtual ~BCSV();
    bool IsValid() const;
    const char* GetErrorMessage() const;
    void Print() const;

    bool GetAll(BCSVData& outData) const;
    bool GetColumnByHash(std::vector<BCSVField>& outCols, u32 columnHash) const;
    u32 GetRowCount() const;
    u32 GetColCount() const;
    bool GetRow(BCSVRow& outRow, u32 index) const;
};