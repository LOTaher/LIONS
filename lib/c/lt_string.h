/*  lt_strings.h - Single file library for my strings implementation
    Copyright (C) 2026 splatte.dev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef LT_STRING_H
#define LT_STRING_H

#include <string.h>

#include "lt_arena.h"
#include "lt_base.h"

typedef struct {
  u8 *str;
  u64 length;
} string8;

#define str8(s) \
    (string8){(u8 *)(s), sizeof(s) - 1}
#define str8_fmt(s) \
    (int)(s).length, (s).str

string8 str8_substring(string8 str, u64 start, u64 end);
string8 str8_cstring(char *str);
string8 str8_copy(string8 str, arena *arena);
string8 str8_concat(string8 str1, string8 str2, arena *arena);
b8      str8_compare(string8 str1, string8 str2);
b8      str8_contains(string8 str, string8 substr);

#endif // LT_STRINGS_H
