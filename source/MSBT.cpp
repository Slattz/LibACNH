/**
 * 
 * MSBT.cpp
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

#include "MSBT.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <locale>
#include <codecvt>

MSBT::MSBT(const char* filePath) {
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

MSBT::MSBT(u8* inBuffer, u64 bufSize, bool manageMem) : data(inBuffer), dataSize(bufSize), autoManageMem(manageMem) {
    if (inBuffer == nullptr || bufSize < 0x1) {
        InValidate("Invalid file buffer");
        return;
    }

    this->Init();
}

MSBT::~MSBT() {
    if (autoManageMem) {
        delete[] this->data;
        autoManageMem = false;
    }
}

void MSBT::Init() {
    if (strncmp((const char*)data, "MsgStdBn", 8)) {
        InValidate("Invalid MSBT Magic");
        return;
    }

    if (data[8] == 0xFF && data[9] == 0xFE) {
        bigEndian = false;
    }

    else if (data[8] == 0xFE && data[9] == 0xFF) {
        bigEndian = true;
    }

    else {
        InValidate("Invalid MSBT BOM");
        return;
    }

    encoding = (MSBTEncoding)data[0xC];
    sectionCount = ReadU16(data+0xE);
    if (!sectionCount) {
        InValidate("No MSBT Sections");
        return;
    }

    this->Parse();
}

bool MSBT::IsValid() const {
    return isValid;
}

const char* MSBT::GetErrorMessage() const {
    return errorMessage;
}

void MSBT::ParseTXT2(u32 dataPos, u32 sectionSize) {
    u32 entryCount = ReadU32(data+dataPos);
    u32 offsetPos = dataPos+4;

    Texts.clear();
    for (u32 i = 0; i < entryCount; i++, offsetPos+=sizeof(u32)) {
        u32 offset = ReadU32(data+offsetPos);
        u32 strStart = dataPos + offset;
        u32 strEnd = (i+1 < entryCount) ? dataPos + ReadU32(data+offsetPos+4) : dataPos + sectionSize;
        Texts.push_back({data+strStart, strEnd-strStart, i, this->encoding});
    }
}

bool compareIndex(MSBTString& s1, MSBTString& s2) {
    return (s1.index < s2.index);
}

void MSBT::ParseLBL1(u32 dataPos) {
    u32 entryCount = ReadU32(data+dataPos);
    u32 offsetPos = dataPos+4;
    
    Labels.clear();
    for (u32 i = 0; i < entryCount; i++, offsetPos+=8) {
        u32 numLabels = ReadU32(data+offsetPos);
        u32 offset = ReadU32(data+offsetPos+4);

        u32 entryOffset = dataPos + offset;
        for (u32 j = 0; j < numLabels; j++) {
            u8 size = data[entryOffset];
            u32 index = ReadU32(data+entryOffset+1+size);
            Labels.push_back({data+entryOffset+1, size, index, Encoding_UTF8});
            entryOffset += sizeof(u8) + size + sizeof(u32); // size byte + string + index u32
        }
    }

    std::sort(Labels.begin(), Labels.end(), compareIndex); //sort by Index
}

#define SECTION_HEADER_SIZE 0x10
void MSBT::Parse() {
    u32 pos = 0x20;

    for (u16 i = 0; i < sectionCount; i++) {
        u32 sectionSize = ReadU32(data+pos+4);
        if (strncmp((const char*)data+pos, "TXT2", 4) == 0) {
            ParseTXT2(pos+SECTION_HEADER_SIZE, sectionSize);
        }

        else if (strncmp((const char*)data+pos, "LBL1", 4) == 0) {
            ParseLBL1(pos+SECTION_HEADER_SIZE);
        }

        pos += sectionSize + SECTION_HEADER_SIZE; //also skips over other sections
        pos = AlignUp(pos, 16);
    }
}

void MSBT::Print() {
    std::string textStr, labelStr;
    for (size_t i = 0; i < Texts.size(); i++) {
        const MSBTString& text = Texts[i];
        const MSBTString& label = Labels[i];

        if (label.encoding == Encoding_UTF8) {
            labelStr = std::string((const char*)label.stringBytes, label.stringSize);
        }

        else {
            std::u16string str((const char16_t*)(label.stringBytes), label.stringSize);
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
            labelStr = convert.to_bytes(str);
        }

        if (text.encoding == Encoding_UTF8) {
            textStr = std::string((const char *)text.stringBytes, text.stringSize);
        }

        else {
            std::u16string str((const char16_t*)(text.stringBytes), text.stringSize);
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
            textStr = convert.to_bytes(str);
        }

        printf("%s - %s\n", labelStr.c_str(), textStr.c_str());
    }
}
