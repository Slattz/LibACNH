/**
 * 
 * SeadRandom.cpp
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

#include "SeadRandom.hpp"
#ifdef __SWITCH__
#include <switch.h>
#else
#include <time.h>
#endif

//sead::Random implementation, based on ACNH 1.4.2 and Splatoon 3.1.0
static const constexpr u32 RandomConstant = 0x6C078965;

namespace sead {

void Random::init(void) {
#ifdef __SWITCH__ //if Switch, implement exactly like ACNH 1.4.2
    u64 tick = svcGetSystemTick();
#else
    u64 tick = time(NULL);
#endif
    this->init(static_cast<u32>(tick)); //Uses lower u32 of system tick as seed for the randomiser
}

void Random::init(u32 seed) { //Allows specifying a specific seed for setting up the randomiser
    for (u32 i = 0; i < 4; i++) {
        context[i] = (u32)((RandomConstant * (seed ^ (seed >> 30))) + i + 1);
        seed = context[i];
    }
}

void Random::init(u32 seed1, u32 seed2, u32 seed3, u32 seed4) { //Allows setting up randomiser context directly
    if ((seed1 | seed2 | seed3 | seed4)) { //If each seed OR'd isn't 0 (i.e. at least one seed isn't 0)
        context[0] = seed1; context[1] = seed2;
        context[2] = seed3; context[3] = seed4;
    }

    else { // same as using a seed of 0 ( init(0) )
        context[0] = 1; context[1] = 0x6C078967;
        context[2] = 0x714ACB41; context[3] = 0x48077044;  
    }
}

#define XORSHFT11(val) (val ^ (val<<11))
u32 Random::GetU32() {
    u32 s0 = XORSHFT11(context[0]) ^ (XORSHFT11(context[0])>>8) ^ (context[3] ^ context[3]>>19);

    context[0] = context[1];
    context[1] = context[2];
    context[2] = context[3];
    return context[3] = s0;
}

u64 Random::GetU64() {
    u32 s0 = XORSHFT11(context[0]) ^ (XORSHFT11(context[0])>>8) ^ (context[3]);
    u32 s1 = XORSHFT11(context[1]) ^ (XORSHFT11(context[1])>>8) ^ (s0 ^ context[3]>>19);
    
    context[0] = context[2];
    context[1] = context[3];
    context[2] = (s0 ^ context[3]>>19);
    context[3] = (s1 ^ s0>>19);
    return ((u64)context[2] << 32) | context[3];
}

void Random::GetContext(u32& ctx0, u32& ctx1, u32& ctx2, u32& ctx3) const {
    ctx0 = context[0];
    ctx1 = context[1];
    ctx2 = context[2];
    ctx3 = context[3];
}

#undef XORSHFT11
} //namespace sead