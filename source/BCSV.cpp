/**
 * 
 * BCSV.cpp
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

#include "BCSV.hpp"
#include <cstdio>
#include <cstring>

struct BCSVColumn {
    u32 hash;
    u32 offset;
};

BCSV::BCSV(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        InValidate("Failed to open file");
        return;
    }

    fseek(file, 0, SEEK_END);
    this->dataSize = ftell(file);
    rewind(file);
    this->data = new u8[dataSize];
    size_t res = fread(this->data, sizeof(u8), dataSize, file);
    if (res == dataSize) {
        autoManageMem = true;
        this->Init();
    } 
    else {
        InValidate("Failed to fully read file");
        delete[] data;
    }
    fclose(file);
}

BCSV::BCSV(u8* inBuffer, u64 bufSize, bool manageMem) : data(inBuffer), dataSize(bufSize), autoManageMem(manageMem) {
    if (inBuffer == nullptr || bufSize < 0xC) {
        InValidate("Invalid file buffer");
        return;
    }

    this->Init();
}

BCSV::~BCSV() {
    if (autoManageMem) {
        delete[] this->data;
        autoManageMem = false;
    }
}

void BCSV::Init() {
    this->numRows = ReadU32(data);
    this->rowSize = ReadU32(data+4);
    this->numColumns = ReadU16(data+8);
    this->version = data[0xA];
    this->jpEnumFlag = data[0xB] == 0;

    if (!numRows || !rowSize || !numColumns) {
        InValidate("Invalid numRows/rowSize/numColumns!");
        return;
    }

    if (version == 0) {
        startPos = 0xC;
    }

    else if (version == 1) {
        startPos = 0x1C;
        if (data[0xC] != 'V' || data[0xD] != 'S' || data[0xE] != 'C' || data[0xF] != 'B') { //Magic 
            InValidate("Invalid BCSV Magic"); 
            return;
        }
    }

    else {
        InValidate("Invalid Version!");
        return;
    }

    this->Parse();
}

bool BCSV::IsValid() const {
    return isValid;
}

const char* BCSV::GetErrorMessage() const {
    return errorMessage;
}

void BCSV::Parse() {
    if (!IsValid()) {
        return;
    }

    BCSVColumn cols[numColumns];

    u32 pos = this->startPos;
    for (u16 i = 0; i < numColumns; i++, pos+=8) {
        cols[i].hash = ReadU32(data + pos);
        cols[i].offset = ReadU32(data + pos + 4);
    }
    
    for (u32 i = 0; i < this->numRows; i++, pos += this->rowSize) {
        BCSVRow rowValues;

        for (u16 j = 0; j < this->numColumns; j++) {
            u32 size = this->rowSize - cols[j].offset;
            if (j < this->numColumns-1) {
                size = cols[j+1].offset - cols[j].offset;
            }

            BCSVField row; row.type = ColumnType::String;
            if (size == sizeof(u8)) {
                row.type = ColumnType::UInt8;
                row.UInt8 = data[pos+cols[j].offset];
            }

            else if (size == sizeof(u16)) {
                row.type = ColumnType::UInt16;
                row.UInt16 = ReadU16(data + pos+cols[j].offset);
            }

            else if (size == sizeof(u32)) {
                row.UInt32 = ReadU32(data + pos+cols[j].offset);
                if (IsFloat(row.UInt32)) {
                    row.type = ColumnType::Float;
                    row.Float = ReadFloat(&row.UInt32); //properly convert the hex representation to float
                }

                else {
                    row.type = ColumnType::UInt32;
                }
            }

            if (row.type == ColumnType::String) {
                row.String = reinterpret_cast<const char*>(data + pos+cols[j].offset);
            }

            rowValues[cols[j].hash] = row;
        }
        this->csvData.push_back(std::move(rowValues));
    }
}

bool BCSV::GetAll(BCSVData& outData) const {
    if (IsValid()) {
        outData = this->csvData;
        return true;
    }
    return false;
}

bool BCSV::GetColumnByHash(std::vector<BCSVField>& outCols, u32 columnHash) const {
    if (!IsValid())
        return false;

    outCols.clear();
    for (const auto& i : this->csvData) {
        if (i.find(columnHash) != i.end()) { //equivalent to C++20's 'contains'
            outCols.push_back(i.at(columnHash));
        }
    }
    return true;
}

u32 BCSV::GetRowCount() const {
    return IsValid() ? numRows : 0;
}

u32 BCSV::GetColCount() const {
    return IsValid() ? numColumns : 0;
}

bool BCSV::GetRow(BCSVRow& outRow, u32 index) const {
    if (!IsValid() || index > csvData.size())
        return false;

    outRow = csvData[index];
    return true;
}

void BCSVField::Print() const {
#ifdef DEBUG
    switch (type) {
        case ColumnType::UInt8:
            printf("%d ", UInt8);
            break;

        case ColumnType::UInt16:
            printf("%d ", UInt16);
            break;

        case ColumnType::UInt32:
            printf("%d ", UInt32);
            break;

        case ColumnType::Float:
            printf("%f ", Float);
            break;

        case ColumnType::String:
            printf("%s ", String);
            break;
            
        default:
            break;
    }
#endif
}

void BCSV::Print() const {
#ifdef DEBUG
    for (const auto& slot : csvData[0]) {
        printf("%08X ", slot.first);
    }

    printf("\n");
    for (const auto& i : this->csvData) {
        for (const auto& slot : i) {
            slot.second.Print();
        }
        printf("\n");
    }
#endif
}

