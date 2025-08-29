/**
 *
 * Byaml.hpp
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
#include <map>
#include <string>

enum class NodeType : u8 {
    String = 0xA0,
    Binary = 0xA1,
    Array = 0xC0,
    Hash = 0xC1,
    StringTable = 0xC2,
    BinaryTable = 0xC3,
    Bool = 0xD0,
    Int = 0xD1,
    Float = 0xD2,
    UInt = 0xD3,
    Int64 = 0xD4,
    UInt64 = 0xD5,
    Double = 0xD6,
    Null = 0xFF
};

struct ByamlNode {
    NodeType type;
    u64 size;

    union {
        void* raw;
        const char* string;
        u8* bytes;
        std::vector<ByamlNode>* array;
        std::map<const char*, ByamlNode>* hash;
        bool Bool;
        s32 Int;
        float Float;
        u32 UInt;
        s64 Int64;
        u64 UInt64;
        double Double;
    };

    void Find(std::vector<ByamlNode>& outNodes, const char* keyName, bool found = false);
    std::vector<ByamlNode> operator[](const char* keyName);
};

class Byaml {
protected:
    inline void InValidate(const char* message) {
        this->isValid = false;
        this->errorMessage = message;
    }

    inline s32 AlignUp(s32 offset, s32 size) {
        return offset + ((size - (offset % size)) % size);
    }

    inline bool IsTypeHash(NodeType type) const {
        return type == NodeType::Hash;
    }

    inline bool IsTypeArray(NodeType type) const {
        return type == NodeType::Array;
    }

    inline bool IsTypeContainer(NodeType type) const {
        return IsTypeHash(type) || IsTypeArray(type);
    }

    inline bool IsTypeStringTable(NodeType type) const {
        return type == NodeType::StringTable;
    }

    ALWAYS_INLINE u8 ReadU8(void* address) const {
        return *(u8*)(address);
    }

    ALWAYS_INLINE u16 ReadU16(void* address) const {
        u16 val = *(u16*)(address);
        if (bigEndian)
            return __builtin_bswap16(val);
        return val;
    }

    ALWAYS_INLINE u32 ReadU24(void* address) const {
        u32 val = (*(u32*)(address)) & 0x00FFFFFF;
        if (bigEndian)
            return (__builtin_bswap32(val) >> 8) & 0x00FFFFFF;
        return val;
    }

    ALWAYS_INLINE u32 ReadU32(void* address) const {
        u32 val = *(u32*)(address);
        if (bigEndian)
            return __builtin_bswap32(val);
        return val;
    }

    ALWAYS_INLINE s32 ReadS32(void* address) const {
        u32 val = *(u32*)(address);
        if (bigEndian)
            return static_cast<s32>(__builtin_bswap32(val));
        return static_cast<s32>(val);
    }

    ALWAYS_INLINE u64 ReadU64(void* address) const {
        u64 val = *(u64*)(address);
        if (bigEndian)
            return __builtin_bswap64(val);
        return val;
    }

    ALWAYS_INLINE s64 ReadS64(void* address) const {
        u64 val = *(u64*)(address);
        if (bigEndian)
            return static_cast<s64>(__builtin_bswap64(val));
        return static_cast<s64>(val);
    }

    ALWAYS_INLINE float ReadFloat(void* address) const {
        u32 val = *(u32*)(address);
        if (bigEndian)
            return static_cast<float>(__builtin_bswap32(val));
        return static_cast<float>(val);
    }

    ALWAYS_INLINE double ReadDouble(void* address) const {
        u64 val = *(u64*)(address);
        if (bigEndian)
            return static_cast<double>(__builtin_bswap64(val));
        return static_cast<double>(val);
    }

    ByamlNode ParseStringNode(u32 index);
    ByamlNode ParseBinaryNode(u32 offset);
    ByamlNode ParseArrayNode(u32 offset);
    ByamlNode ParseHashNode(u32 offset);
    ByamlNode ParseBoolNode(u32 offset);
    ByamlNode ParseIntNode(u32 offset);
    ByamlNode ParseFloatNode(u32 offset);
    ByamlNode ParseUIntNode(u32 offset);
    ByamlNode ParseInt64Node(u32 offset);
    ByamlNode ParseUInt64Node(u32 offset);
    ByamlNode ParseDoubleNode(u32 offset);
    ByamlNode ParseNullNode();

    void Init();

    ByamlNode ParseNode(NodeType node, u32 offset);
    bool ParseTable(u32 offset, std::vector<const char*>& table);
    bool Parse();

    void printTabs(u32 indent, std::string& outString, NodeType type) const;
    void Print(const ByamlNode& node, std::string& outString, u32 indent) const;

    u8* data = nullptr;
    u64 dataSize = 0;
    const char* errorMessage = "No Error";
    bool autoManageMem = false;

    bool bigEndian;
    bool isValid = true;

    ByamlNode parentNode;

    std::vector<const char*> stringTable;
    std::vector<const char*> hashTable;

public:
    Byaml(const char* filePath);
    Byaml(u8* inBuffer, u64 bufSize, bool manageMem = false);
    virtual ~Byaml();
    bool IsValid() const;
    const char* GetErrorMessage() const;

    bool ToString(std::string& outStr) const;
    bool ToYaml(const char* filePath) const;
    static Byaml* FromYaml(const char* filePath);

    std::vector<ByamlNode> operator[](const char* keyName);
};
