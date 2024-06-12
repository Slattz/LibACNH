/**
 * 
 * ACNHByaml.cpp
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

#include "ACNHByaml.hpp"
#include <cstring>

ACNHByaml::ACNHByaml(const char* filePath) : Byaml(filePath) {

}

ACNHByaml::ACNHByaml(u8* inBuffer, u64 bufSize, bool manageMem) : Byaml(inBuffer, bufSize, manageMem) {

}

ACNHByaml::~ACNHByaml() {
    if (autoManageMem) {
        delete[] this->data;
        autoManageMem = false;
    }
}

//Path: Array -> Hash -> Array -> Hash
u64 ACNHByaml::CalcOffset(u32& typeName, u32 fieldName) {
    u32 typeUInt = 0;
    u64 res = 0;

    if (!IsTypeArray(parentNode.type) || !IsValid())
        return 0;
    
    for (u64 i = 0; i < parentNode.size; i++) {
        ByamlNode arrayNode = (*parentNode.array)[i];
        
        if (IsTypeHash(arrayNode.type)) {
            for (auto elem : (*arrayNode.hash)) {
                if (strcmp(elem.first, "5c65d8b5") == 0 && elem.second.type == NodeType::UInt) { //5c65d8b5 == "TypeName"
                    typeUInt = elem.second.UInt;
                }
                
                else if (typeUInt == typeName && strcmp(elem.first, "a2fb4a94") == 0 && IsTypeArray(elem.second.type)) { //a2fb4a94 == "Members"
                    ByamlNode secElem = elem.second;
                    
                    for (u64 j = 0; j < secElem.size; j++) {
                        ByamlNode arrayNode2 = (*secElem.array)[j];
                        
                        if (IsTypeHash(arrayNode2.type)) {
                            bool found = false;
                            for (auto elem2 : (*arrayNode2.hash)) {
                                if (strcmp(elem2.first, "25efa387") == 0 && elem2.second.type == NodeType::UInt) { //25efa387 == "Name"
                                    found = (fieldName == elem2.second.UInt);
                                }
                                
                                else if (found && strcmp(elem2.first, "5c65d8b5") == 0 && elem2.second.type == NodeType::UInt) { //5c65d8b5 == "TypeName"
                                    typeName = elem2.second.UInt;
                                }
                                
                                else if (found && strcmp(elem2.first, "a7bb2e42") == 0 && elem2.second.type == NodeType::Int64) { //a7bb2e42 == "Offset"
                                    res = (u64)elem2.second.Int64;
                                }

                                if (found && typeName != typeUInt && res != 0) //Assume offset found, break early
                                    break;
                            }
                            
                            if (found) {
                                return res;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

//Path: Array -> Hash -> Array -> Hash
u64 ACNHByaml::GetSize(u32& typeName, u32 fieldName) {
    u32 typeUInt = 0;
    u64 res = 0;

    if (!IsTypeArray(parentNode.type) || !IsValid())
        return 0;
    
    for (u64 i = 0; i < parentNode.size; i++) {
        ByamlNode arrayNode = (*parentNode.array)[i];
        
        if (IsTypeHash(arrayNode.type)) {
            for (auto elem : (*arrayNode.hash)) {
                if (strcmp(elem.first, "5c65d8b5") == 0 && elem.second.type == NodeType::UInt) { //5c65d8b5 == "TypeName"
                    typeUInt = elem.second.UInt;
                }
                else if (typeUInt == typeName && strcmp(elem.first, "a2fb4a94") == 0 && IsTypeArray(elem.second.type)) { //a2fb4a94 == "Members"
                    ByamlNode secElem = elem.second;
                    
                    for (u64 j = 0; j < secElem.size; j++) {
                        ByamlNode arrayNode2 = (*secElem.array)[j];
                        
                        if (IsTypeHash(arrayNode2.type)) {
                            bool found = false;
                            for (auto elem2 : (*arrayNode2.hash)) {
                                if (strcmp(elem2.first, "25efa387") == 0 && elem2.second.type == NodeType::UInt) { //25efa387 == "Name"
                                    found = (fieldName == elem2.second.UInt);
                                }
                                
                                else if (found && strcmp(elem2.first, "5c65d8b5") == 0 && elem2.second.type == NodeType::UInt) { //5c65d8b5 == "TypeName"
                                     typeName = elem2.second.UInt;
                                }

                                else if (found && strcmp(elem2.first, "b4a58247") == 0 && elem2.second.type == NodeType::Int64) { //b4a58247 == "Size"
                                    res = (u64)elem2.second.Int64;
                                }

                                if (found && typeName != typeUInt && res != 0) //Assume size found, break early
                                    break;
                            }
                            
                            if (found) {
                                return res;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}


//Path: Array -> Hash -> Array -> Hash
ByamlNode ACNHByaml::GetNode(u32& typeName, u32 fieldName) {
    u32 typeUInt = 0;
    ByamlNode res = ByamlNode();

    if (!IsTypeArray(parentNode.type) || !IsValid())
        return ByamlNode();
    
    for (u64 i = 0; i < parentNode.size; i++) {
        ByamlNode arrayNode = (*parentNode.array)[i];
        
        if (IsTypeHash(arrayNode.type)) {
            for (auto elem : (*arrayNode.hash)) {
                if (strcmp(elem.first, "5c65d8b5") == 0 && elem.second.type == NodeType::UInt) { //5c65d8b5 == "TypeName"
                    typeUInt = elem.second.UInt;
                }
                else if (typeUInt == typeName && strcmp(elem.first, "a2fb4a94") == 0 && IsTypeArray(elem.second.type)) { //a2fb4a94 == "Members"
                    ByamlNode secElem = elem.second;
                    
                    for (u64 j = 0; j < secElem.size; j++) {
                        ByamlNode arrayNode2 = (*secElem.array)[j];
                        
                        if (IsTypeHash(arrayNode2.type)) {
                            bool found = false;
                            res = arrayNode2;
                            for (auto elem2 : (*arrayNode2.hash)) {
                                if (strcmp(elem2.first, "25efa387") == 0 && elem2.second.type == NodeType::UInt) { //25efa387 == "Name"
                                    found = (fieldName == elem2.second.UInt);
                                }
                                else if (found && strcmp(elem2.first, "5c65d8b5") == 0 && elem2.second.type == NodeType::UInt) { //5c65d8b5 == "TypeName"
                                     typeName = elem2.second.UInt;
                                }

                                if (found && typeName != typeUInt) //Assume size found, break early
                                    break;
                            }
                            
                            if (found) {
                                return res;
                            }
                        }
                    }
                }
            }
        }
    }

    return ByamlNode();
}


u64 ACNHByaml::CalcOffsets(u32 fieldType, const std::vector<u32>& fieldNames) {
    u64 offset = 0;
    for (u32 name : fieldNames) {
        offset += CalcOffset(fieldType, name);
    }
    return offset;
}

u64 ACNHByaml::GetSize(u32 fieldType, const std::vector<u32>& fieldNames) {
    u64 size = 0;
    for (u32 name : fieldNames) {
        size = GetSize(fieldType, name);
    }
    return size;
}

ByamlNode ACNHByaml::GetNode(u32 fieldType, const std::vector<u32>& fieldNames) {
    ByamlNode node = ByamlNode();
    for (u32 name : fieldNames) {
        node = GetNode(fieldType, name);
    }
    return node;
}