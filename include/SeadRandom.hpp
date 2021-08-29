/**
 * 
 * SeadRandom.hpp
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

/**
 * sead::Random: Nintendo's implementation of Mersenne Twister (MT19937) 
 * Based on ACNH 1.4.2 and Splatoon 3.1.0
 */

namespace sead {

class Random {
public:
    Random() { init(); };
    Random(u32 seed) { init(seed); };
    Random(u32 seed1, u32 seed2, u32 seed3, u32 seed4) { init(seed1, seed2, seed3, seed4); };

    //init functions seperated to allow re-initialisation
    void init();
    void init(u32 seed);
    void init(u32 seed1, u32 seed2, u32 seed3, u32 seed4);

    u32 GetU32();
    u64 GetU64();
    void GetContext(u32& ctx0, u32& ctx1, u32& ctx2, u32& ctx3) const;

private:
    u32 context[4];
};

} //namespace sead