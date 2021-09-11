/**
 * 
 * Byaml.cpp
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

#include "Byaml.hpp"
#include <cstdio>
#include <cstring>

Byaml::Byaml(const char* filePath) {
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

Byaml::Byaml(u8* inBuffer, u64 bufSize, bool manageMem) : data(inBuffer), dataSize(bufSize), autoManageMem(manageMem) {
    if (inBuffer == nullptr || bufSize < 0x10) {
        InValidate("Invalid file buffer");
        return;
    }

    this->Init();
}

Byaml::~Byaml() {
    if (autoManageMem) {
        delete[] this->data;
        autoManageMem = false;
    }
}

void Byaml::Init() {
    if (data[0] == 'Y' && data[1] == 'B') { //Little Endian
        bigEndian = false;
    }

    else if (data[0] == 'B' && data[1] == 'Y') { //Big Endian
        bigEndian = true;
        InValidate("Big Endian Unsupported"); //TODO: Technically supported but not tested
        return;
    }

    else {
        InValidate("Invalid Magic");
        return;
    }

    u16 version = ReadU16(data+2);
    if (version < 1 || version > 4) {
        InValidate("Invalid Version");
        return;
    }

    if (!this->Parse()) {
        InValidate("Failed to Parse");
    }
}

bool Byaml::IsValid() const {
    return isValid;
}

const char* Byaml::GetErrorMessage() const {
    return errorMessage;
}

bool Byaml::ToYaml(const char* filePath) const {
    (void)(filePath);
    if (!IsValid())
        return false;

    return true;
}

Byaml* Byaml::FromYaml(const char* filePath) {
    (void)(filePath);
    return nullptr;
}

bool Byaml::ParseTable(u32 offset, std::vector<const char*>& table) {
    NodeType node = (NodeType)data[offset];
    if (!IsTypeStringTable(node)) {
        return false;
    }
    
    table.clear();
    u32 size = ReadU24(data+(offset+1));

    for (u32 i = 0; i < size; i++) {
        u32 stringOffset = offset + ReadU32(data+(offset+4+(i*4)));
        const char* strFromTable = reinterpret_cast<const char*>(data+stringOffset);
        table.push_back(strFromTable);
    }
    return true;
}

bool Byaml::Parse() {
    u32 dataoffset = ReadU32(data+0xC);
    NodeType node = (NodeType)ReadU8(data+dataoffset);
    if (!IsTypeContainer(node)) {
        return false;
    }

    u32 hashKeyTableOffset = ReadU32(data+0x4);
    if (hashKeyTableOffset != 0) {
        ParseTable(hashKeyTableOffset, this->hashTable);
    }

    u32 stringTableOffset = ReadU32(data+0x8);
    if (stringTableOffset != 0) {
        ParseTable(stringTableOffset, this->stringTable);
    }

    parentNode = ParseNode(node, 0xC);
    if (parentNode.type == NodeType::Null) {
        return false;
    }
    return true;
}

ByamlNode Byaml::ParseNode(NodeType node, u32 offset) {
    switch (node) {
        case NodeType::String:
            return ParseStringNode(offset);

        case NodeType::Binary:
            return ParseBinaryNode(ReadU32(data+offset));

        case NodeType::Array:
            return ParseArrayNode(ReadU32(data+offset));

        case NodeType::Hash:
            return ParseHashNode(ReadU32(data+offset));

        case NodeType::Bool:
            return ParseBoolNode(offset);

        case NodeType::Int:
            return ParseIntNode(offset);

        case NodeType::Float:
            return ParseFloatNode(offset);

        case NodeType::UInt:
            return ParseUIntNode(offset);

        case NodeType::Int64:
            return ParseInt64Node(ReadU32(data+offset));

        case NodeType::UInt64:
            return ParseUInt64Node(ReadU32(data+offset));

        case NodeType::Double:
            return ParseDoubleNode(ReadU32(data+offset));

        case NodeType::Null:
            return ParseNullNode();

        default:
            return ParseNullNode();
    }
}

ByamlNode Byaml::ParseStringNode(u32 offset) {
    ByamlNode node;
    node.type = NodeType::String;
    node.string = stringTable[ReadU32(data+offset)];
    node.size = strlen(node.string);
    //printf("Parsing String Node: %s\n", node.string);
    return node;
}

ByamlNode Byaml::ParseBinaryNode(u32 offset) {
    //printf("Parsing Binary Node\n");
    ByamlNode node;
    node.type = NodeType::Binary;
    node.size = ReadU32(data+offset);
    node.bytes = this->data+offset+4;
    return node;
}

ByamlNode Byaml::ParseArrayNode(u32 offset) {
    //printf("Parsing Array Node\n");
    ByamlNode node;
    node.type = NodeType::Array;
    node.array = new std::vector<ByamlNode>();
    node.size = ReadU24(data+offset +1 );
    u32 entryOffset = offset + AlignUp((s32)node.size, 4) + 4;
    for (u64 i = 0; i < node.size; i++) {
        u8 nodeType = data[offset+4+i];
        node.array->push_back(ParseNode((NodeType)nodeType, entryOffset + (4*i)));
    }
    return node;
}

ByamlNode Byaml::ParseHashNode(u32 offset) {
    //printf("Parsing Hash Node\n");
    ByamlNode node;
    node.type = NodeType::Hash;
    node.hash = new std::map<const char*, ByamlNode>();
    node.size = ReadU24(data+offset +1);
    for (u64 i = 0; i < node.size; i++) {
        u32 entryOffset = offset + 4 + (i*8);
        u32 stringIdx = ReadU24(data+entryOffset);
        const char* hashName = hashTable[stringIdx];

        u8 nodeType = data[entryOffset+3];
        (*node.hash)[hashName] = ParseNode((NodeType)nodeType, entryOffset+4);
    }
    
    return node;
}

ByamlNode Byaml::ParseBoolNode(u32 offset) {
    ByamlNode node;
    node.type = NodeType::Bool;
    node.Bool = ReadU32(data+offset) != 0;
    //printf("Parsing Bool Node: %d\n", node.Bool);
    return node;
}

ByamlNode Byaml::ParseIntNode(u32 offset) {
    ByamlNode node;
    node.type = NodeType::Int;
    node.Int = ReadS32(data+offset);
    //printf("Parsing Int Node: %d\n", node.Int);
    return node;
}

ByamlNode Byaml::ParseFloatNode(u32 offset) {
    ByamlNode node;
    node.type = NodeType::Float;
    node.Float = ReadFloat(data+offset);
    //printf("Parsing Float Node: %f\n", node.Float);
    return node;
}

ByamlNode Byaml::ParseUIntNode(u32 offset) {
    ByamlNode node;
    node.type = NodeType::UInt;
    node.UInt = ReadU32(data+offset);
    //printf("Parsing UInt Node: %d\n", node.UInt);
    return node;
}

ByamlNode Byaml::ParseInt64Node(u32 offset) {
    ByamlNode node;
    node.type = NodeType::Int64;
    node.Int64 = ReadS64(data+offset);
    //printf("Parsing Int64 Node: %ld\n", node.Int64);
    return node;
}

ByamlNode Byaml::ParseUInt64Node(u32 offset) {
    ByamlNode node;
    node.type = NodeType::UInt64;
    node.UInt64 = ReadU64(data+offset);
    //printf("Parsing UInt64 Node: %ld\n", node.UInt64);
    return node;
}

ByamlNode Byaml::ParseDoubleNode(u32 offset) {
    ByamlNode node;
    node.type = NodeType::Double;
    node.Double = ReadDouble(data+offset);
    //printf("Parsing Double Node: %f\n", node.Double);
    return node;
}

ByamlNode Byaml::ParseNullNode() {
    //printf("Parsing Null Node\n");
    ByamlNode node;
    node.type = NodeType::Null;
    node.size = 0;
    return node;
}

bool Byaml::ToString(std::string& outStr) const {
    if (IsValid()) {
        Print(this->parentNode, outStr, 0);
        return true;
    }

    else {
        outStr = "Not Valid!";
        return false;
    }
}

void Byaml::printTabs(u32 indent, std::string& outString, NodeType type) const {
    outString += "├─";
    for (u32 i = 0; i < indent; i++)
        outString += "──";

    switch (type) {
        case NodeType::String:
            outString += " String: ";
            break;

        case NodeType::Binary:
            outString += " Binary: "; 
            break;

        case NodeType::Array:
            outString += " Array:\n";
            break;

        case NodeType::Hash:
            outString += " Hash: "; 
            break;

        case NodeType::Bool:
            outString += " Bool: "; 
            break;

        case NodeType::Int:
            outString += " Int: ";
            break;

        case NodeType::Float:
            outString += " Float: "; 
            break;

        case NodeType::UInt:
            outString += " UInt: "; 
            break;

        case NodeType::Int64:
            outString += " Int64: "; 
            break;

        case NodeType::UInt64:
            outString += " UInt64: "; 
            break;

        case NodeType::Double:
            outString += " Double: "; 
            break;

        case NodeType::Null:
            outString += ' '; 
            break;

        default:
            break;
    }
}

void Byaml::Print(const ByamlNode& node, std::string& outString, u32 indent) const {
    switch (node.type) {
        case NodeType::String:
            printTabs(indent, outString, NodeType::String);
            (outString += node.string) += '\n';
            break;

        case NodeType::Binary:
            {
                if (node.size > 0) {
                    printTabs(indent, outString, NodeType::Binary);
                    char byteStr[4] = {0};
                    for (u64 i = 0; i < node.size; i++) {
                        snprintf(byteStr, 3, "%02X", node.bytes[i]%0xFF);
                        outString += byteStr;
                    }
                    outString += '\n';
                }
            }
            break;

        case NodeType::Array:
            {
                for (u64 i = 0; i < node.size; i++) {
                    printTabs(indent, outString, NodeType::Array);
                    Print((*node.array)[i], outString, indent+1);
                }
            }
            break;

        case NodeType::Hash:
            {
                for (auto elem : (*node.hash)) {
                    printTabs(indent, outString, NodeType::Hash);
                    (outString += elem.first) += '\n';
                    Print(elem.second, outString, indent+1);
                }
            }
            break;

        case NodeType::Bool:
            printTabs(indent, outString, NodeType::Bool);
            outString += (node.Bool == true ? "true\n" : "false\n");
            break;

        case NodeType::Int:
            printTabs(indent, outString, NodeType::Int);
            (outString += std::to_string(node.Int)) += '\n';
            break;

        case NodeType::Float:
            printTabs(indent, outString, NodeType::Float);
            (outString += std::to_string(node.Float)) += '\n';
            break;

        case NodeType::UInt:
            printTabs(indent, outString, NodeType::UInt);
            (outString += std::to_string(node.UInt)) += '\n';
            break;

        case NodeType::Int64:
            printTabs(indent, outString, NodeType::Int64);
            (outString += std::to_string(node.Int64)) += '\n';
            break;

        case NodeType::UInt64:
            printTabs(indent, outString, NodeType::UInt64);
            (outString += std::to_string(node.UInt64)) += '\n';
            break;

        case NodeType::Double:
            printTabs(indent, outString, NodeType::Double);
            (outString += std::to_string(node.Double)) += '\n';
            break;

        case NodeType::Null:
            printTabs(indent, outString, NodeType::Null);
            outString += "NULL\n";
            break;

        default:
            //printf("UNKNOWN: 0x%02X\n", (u8)node.type);
            break;
    }
}

void ByamlNode::Find(std::vector<ByamlNode>& outNodes, const char* keyName, bool found) {
    switch (this->type) {
        case NodeType::String: [[fallthrough]];
        case NodeType::Binary: [[fallthrough]];
        case NodeType::Bool: [[fallthrough]];
        case NodeType::Int: [[fallthrough]];
        case NodeType::Float: [[fallthrough]];
        case NodeType::UInt: [[fallthrough]];
        case NodeType::Int64: [[fallthrough]];
        case NodeType::UInt64: [[fallthrough]];
        case NodeType::Double: [[fallthrough]];
        case NodeType::Null: 
            {
                if (found)
                    outNodes.push_back(*this);
            }
            break;

        case NodeType::Array:
            {
                for (u64 i = 0; i < this->size; i++) {
                    ByamlNode node = (*(this->array))[i];
                    node.Find(outNodes, keyName, false);
                }
            }
            break;

        case NodeType::Hash:
            {
                for (auto elem : (*(this->hash))) {
                    elem.second.Find(outNodes, keyName, (strcmp(elem.first, keyName) == 0));
                }
            }
            break;

        default:
            break;
    }
}

std::vector<ByamlNode> ByamlNode::operator[](const char* keyName) {
    std::vector<ByamlNode> nodes;
    this->Find(nodes, keyName, false);
    return nodes;
}

std::vector<ByamlNode> Byaml::operator[](const char* keyName) {
    std::vector<ByamlNode> nodes;
    parentNode.Find(nodes, keyName, false);
    return nodes;
}