/**
 * 
 * ACNHByaml.hpp
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
#include "Byaml.hpp"

class ACNHByaml : public Byaml {
private:
    u64 CalcOffset(u32& typeName, u32 fieldName);
    u64 GetSize(u32& typeName, u32 fieldName);
    u64 GetSize(u32& typeName, u32 fieldName, ByamlNode &node);
    ByamlNode GetNode(u32& typeName, u32 fieldName);

public:
    ACNHByaml(const char* filePath);
    ACNHByaml(u8* inBuffer, u64 bufSize, bool manageMem = false);
    ~ACNHByaml() override;

    u64 CalcOffsets(u32 fieldType, const std::vector<u32>& fieldNames);
    u64 GetSize(u32 fieldType, const std::vector<u32>& fieldNames);
    ByamlNode GetNode(u32 fieldType, const std::vector<u32>& fieldNames);
    
};