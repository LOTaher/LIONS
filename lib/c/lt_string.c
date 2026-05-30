#include "./lt_string.h"

string8 str8_substring(string8 str, u64 start, u64 end)
{
  end = MIN(end, str.length);
  start = MIN(start, end);

  return (string8){str.str + start, end - start};
}

string8 str8_cstring(char *str) { return (string8){(u8 *)str, strlen(str)}; }

b8 str8_compare(string8 str1, string8 str2)
{
  if (str1.length != str2.length)
    return -1;

  return memcmp(str1.str, str2.str, str1.length) == 0 ? 1 : -1;
}

b8 str8_contains(string8 str, string8 substr)
{
  if (substr.length > str.length)
    return -1;

  for (u64 i = 0; i < str.length; i++) {
    string8 sub = str8_substring(str, i, substr.length + i);

    if (str8_compare(sub, substr))
      return 1;
  }

  return -1;
}

string8 str8_copy(string8 str, arena *arena)
{
  u8 *bytes = (u8 *)arena_push(arena, str.length);

  memcpy(bytes, &str, str.length);

  return (string8){bytes, str.length};
}

string8 str8_concat(string8 str1, string8 str2, arena *arena)
{
  u8 *bytes = (u8 *)arena_push(arena, str1.length + str2.length);

  memcpy(bytes, str1.str, str1.length);
  memcpy(bytes + str1.length, str2.str, str2.length);

  return (string8){bytes, str1.length + str2.length};
}
