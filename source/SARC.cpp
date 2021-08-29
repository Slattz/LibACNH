/**
 * 
 * SARC.cpp
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

#include "SARC.hpp"
#include <cstdio>
#include <cstring>

static const constexpr u8 SFATNodeSize = 0x10;

SARC::SARC() {
    
}

SARC::SARC(const char* filePath) {
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

SARC::SARC(u8* inBuffer, u64 bufSize, bool manageMem) : data(inBuffer), dataSize(bufSize), autoManageMem(manageMem) {
    if (inBuffer == nullptr || bufSize < 0x1) {
        InValidate("Invalid file buffer");
        return;
    }

    this->Init();
}

SARC::~SARC() {
    if (autoManageMem) {
        delete[] this->data;
        autoManageMem = false;
    }
}

void SARC::Init() { //Checks based on ACNH 1.5.0
    if (data[0] != 'S' || data[1] != 'A' || data[2] != 'R' || data[3] != 'C') { //Magic 
        InValidate("Invalid SARC Magic"); 
        return;
    }

    if (data[6] == 0xFF && data[7] == 0xFE) { //Little Endian BOM
        bigEndian = false;
    }

    else if (data[6] == 0xFE && data[7] == 0xFF) { //Big Endian BOM
        bigEndian = true;
        InValidate("Big Endian Unsupported"); //TODO: Technically supported but not tested
        return;
    }

    else {
        InValidate("Invalid BOM");
        return;
    }

    u16 sarcHeaderSize = ReadU16(data + 4);
    if (sarcHeaderSize != 0x14) { //SARC Header Size
        InValidate("Invalid SARC Header Size"); 
        return;
    }

    this->dataOffset = ReadU32(data + 0xC);

    u16 version = ReadU16(data+0x10);
    if (version != 0x100) {
        InValidate("Invalid Version");
        return;
    }

    /* SFAT - File Attribute Table */
    if (data[sarcHeaderSize] != 'S' || data[sarcHeaderSize+1] != 'F' ||
        data[sarcHeaderSize+2] != 'A' || data[sarcHeaderSize+3] != 'T') { //SFAT Magic
        InValidate("Invalid SFAT Magic"); 
        return;
    }

    u16 sfatHeaderSize = ReadU16(data + sarcHeaderSize + 4);
    if (sfatHeaderSize != 0xC) { //SFAT Header Size
        InValidate("Invalid SFAT Header Size"); 
        return;
    }

    this->nodeAddress = sarcHeaderSize + sfatHeaderSize;
    this->nodeCount = ReadU16(data + sarcHeaderSize + 6);
    if ((nodeCount >> 0xE) != 0) {
        InValidate("Too many files"); 
        return;
    }

    /* SFNT - File Name Table */
    u16 sfntHeaderOffset = nodeAddress + (SFATNodeSize*nodeCount);
    if (data[sfntHeaderOffset] != 'S' || data[sfntHeaderOffset+1] != 'F' ||
        data[sfntHeaderOffset+2] != 'N' || data[sfntHeaderOffset+3] != 'T') { //SFNT Magic
        InValidate("Invalid SFNT Magic"); 
        return;
    }

    u16 sfntHeaderSize = ReadU16(data + sfntHeaderOffset + 4);
    if (sfntHeaderSize != 0x8) { //SFNT Header Size
        InValidate("Invalid SFNT Header Size");
        return;
    }
    this->nameTableAddress = sfntHeaderOffset + sfntHeaderSize;
    if (nameTableAddress > dataOffset) {
        InValidate("Data Offset Before Name Table Offset"); 
        return;
    }

    this->Parse();
}

bool SARC::IsValid() const {
    return isValid;
}

const char* SARC::GetErrorMessage() const {
    return errorMessage;
}

void SARC::Parse() {
    fileInfo.clear();
    
    u32 offset = this->nodeAddress;
    for (u16 i = 0; i < this->nodeCount; i++, offset += SFATNodeSize) {
        //u32 nameHash = ReadU32(data+offset);
        u32 fileAttributes = ReadU32(data+offset+0x4);
        if (fileAttributes == 0) { //Unnamed Files
            continue;
        }

        //u8 flags = fileAttributes >> 24;
        u32 nameOffset = fileAttributes & 0xffffff;
        u32 dataBegin = ReadU32(data+offset+0x8);
        u32 dataEnd = ReadU32(data+offset+0xC);

        u32 nameTableOffset = this->nameTableAddress + (4*nameOffset);
        if (nameTableOffset > this->dataOffset) {
            continue;
        }

        SARCFileInfo info; 
        info.name = reinterpret_cast<const char*>(data + nameTableOffset);
        info.dataAddress = data+dataOffset+dataBegin;
        info.dataSize = dataEnd - dataBegin;
        fileInfo.push_back(info);
    }
}

void SARC::Print() {
    for (auto& i : fileInfo) {
        printf("%s\n", i.name);
    }
}

bool SARC::GetFile(const char *outFile, const char *sarcFilePath) {
    static const constexpr u64 BLOCK_SIZE = 0x200000;

    if (!this->isValid || !outFile || !sarcFilePath)
        return false;

    SARCFileInfo fInfo; fInfo.name = nullptr;
    for (auto& info : fileInfo) {
        if (strcmp(info.name, sarcFilePath) == 0) {
            fInfo = info;
            break;
        }
    }

    if (fInfo.name == nullptr) {
        return false;
    }

    FILE* file = fopen(outFile, "w");
    if (file == NULL) {
        return false;
    }

    u8* buffer = new u8[BLOCK_SIZE];
    if (!buffer) {
        return false;
    }

    for (u64 offset = 0, blockSize = BLOCK_SIZE; offset < fInfo.dataSize; offset += blockSize) {
        if (blockSize > (fInfo.dataSize - offset))
            blockSize = (fInfo.dataSize - offset);
        
        size_t written = fwrite(fInfo.dataAddress+offset, sizeof(u8), blockSize, file);
        if (written != blockSize) {
            fclose(file);
            delete[] buffer;
            return false;
        }
    }

    fclose(file);
    delete[] buffer;
    return true;
}
