/**
 * 
 * MurmurHash3.hpp
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

namespace MurmurHash3 {
    u32 Calc(u8* data, u32 offset, u32 size, u32 seed = 0); //ACNH 1.4.2 code: 0x7100036380
    u32 Update(u8* data, u32 hashOffset, u32 readOffset, u32 readSize);
    u32 Verify(u8* data, u32 hashOffset, u32 readOffset, u32 readSize);
}
