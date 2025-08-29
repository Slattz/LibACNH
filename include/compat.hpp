/**
 *
 * compat.hpp
 *
 * Copyright (c) 2021-2025, Slattz.
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

/// Flags a function as (always) inline.
#define ALWAYS_INLINE __attribute__((always_inline)) inline

/// Flags a function as consteval in >= C++20, else constexpr in >= C++14, else just always inline
/// Flags a function as constexpr in >= C++14, else just always inline
#if __cplusplus > 201703L //gcc 10.2 uses 201709 for C++20 instead of 202002
    #define LIBACNH_CONSTEVAL ALWAYS_INLINE consteval
    #define LIBACNH_CONSTEXPR ALWAYS_INLINE constexpr

#elif __cplusplus >= 201402L //C++14 and above
    #define LIBACNH_CONSTEVAL ALWAYS_INLINE constexpr
    #define LIBACNH_CONSTEXPR ALWAYS_INLINE constexpr

#else
    #define LIBACNH_CONSTEVAL ALWAYS_INLINE
    #define LIBACNH_CONSTEXPR ALWAYS_INLINE
#endif

/// Packs a struct so that it won't include padding bytes.
#ifndef PACKED
#define PACKED     __attribute__((packed))
#endif

/// Marks a function as not returning, for the purposes of compiler optimization.
#ifndef NORETURN
#define NORETURN   __attribute__((noreturn))
#endif
