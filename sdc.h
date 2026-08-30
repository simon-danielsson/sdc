/*

SDC (an abbreviation of "Simon Danielsson's C library")

A header-only C library with utilities frequently needed across my various C
projects. Implemented in ANSI C (C89).

Repository     https://github.com/simon-danielsson/sdc
Author         Simon Danielsson
Contact        contact@simondanielsson.se
License        MIT

See the end of this file for more information.

*/

#ifndef SDC_H_INCLUDE
#define SDC_H_INCLUDE

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#endif /* SDC_H_INCLUDE */

#ifdef SDC_IMPLEMENTATION

#define _SDC_internal static

/* MATH = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =  */

/* Source: https://en.wikipedia.org/wiki/Feature_scaling */
/* returns new rescaled value */
int SDC_math_min_max_rescale_value(const int old_value, const int old_min,
                                   const int old_max, const int new_min,
                                   const int new_max) {
  double new_value = ((old_value - old_min) / (double)(old_max - old_min));
  new_value *= (new_max - new_min);
  return (new_value + new_min);
}

/* Source: https://en.wikipedia.org/wiki/Feature_scaling */
/* returns new rescaled value */
double SDC_math_min_max_rescale_value_double(const double old_value,
                                             const double old_min,
                                             const double old_max,
                                             const double new_min,
                                             const double new_max) {
  double new_value = ((old_value - old_min) / (old_max - old_min));
  new_value *= (new_max - new_min);
  return (new_value + new_min);
}

/* Source: https://stackoverflow.com/a/41871699 */
double SDC_math_floor(double num) {
  long n;
  double d;
  if (num >= (double)LONG_MAX || num <= (double)LONG_MIN || num != num) {
    return num;
  }
  n = (long)num;
  d = (double)n;
  if (d == num || num >= 0)
    return d;
  else
    return d - 1;
}

/* Source: https://stackoverflow.com/a/16659263 */
double SDC_math_clamp(double d, double min, double max) {
  const double t = d < min ? min : d;
  return t > max ? max : t;
}

/* Get the character width of an integer. */
/* Useful when working with CLI/TUI applications. */
size_t SDC_math_char_width_of_int(int i) {
  size_t len;
  if (i == INT_MIN)
    return 11;
  if (i < 0)
    i = -i;

  len = 1;
  for (; i >= 10; i /= 10)
    len++;
  return len;
}

/* RAND = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =  */

/* Source: https://en.wikipedia.org/wiki/Xorshift#xoroshiro */
_SDC_internal uint32_t _SDC_xorshift32(void) {
  uint32_t state = time(NULL) % (uint32_t)rand();
  uint32_t x = state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return state = x;
}

/* Returns either 0 or 1. */
int SDC_rand_bool(void) { return _SDC_xorshift32() % 2; }
_SDC_internal const uint32_t _SDC_rand_range_old_max = 1000 * 64;

/* Pseudo RNG: quick and dirty XORshift followed by min-max rescaling. */
int SDC_rand_range(const int floor, const int ceiling) {
  uint32_t rand = _SDC_xorshift32() % _SDC_rand_range_old_max;
  return SDC_math_min_max_rescale_value((int)rand, 0, _SDC_rand_range_old_max,
                                        floor, ceiling);
}

/* Pseudo RNG: quick and dirty XORshift followed by min-max rescaling. */
double SDC_rand_range_double(const double floor, const double ceiling) {
  uint32_t rand = _SDC_xorshift32() % _SDC_rand_range_old_max;
  return SDC_math_min_max_rescale_value_double(
      (int)rand, 0, _SDC_rand_range_old_max, floor, ceiling);
}

/* STRINGS = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/* Returns NULL if failure */
char *SDC_str_dup(const char *s) {
  char *out;
  if (!s)
    return NULL;
  out = malloc(strlen(s) + 1);
  if (!out) {
    return NULL;
  }
  strcpy(out, s);
  return out;
}

/* Append a single char to a string in-place. */
void SDC_str_append_char(char *s, char c) {
  int len = strlen(s);
  s[len] = c;
  s[len + 1] = '\0';
}

/* Trims whitespace, newlines etc. at beginning and end of string in-place. */
void SDC_str_trim(char *s) {
  char *start = s, *end;
  size_t len;
  while (*start && isspace((unsigned char)*start))
    start++;
  if (*start == '\0') {
    s[0] = '\0';
    return;
  }
  end = start + strlen(start) - 1;
  while (end > start && isspace((unsigned char)*end))
    end--;
  len = (size_t)(end - start) + 1;
  memmove(s, start, len);
  s[len] = '\0';
}

/* Returns 1 if true. */
int SDC_str_starts_with(const char *str, const char *starts_with) {
  size_t len_str = strlen(str), len_word = strlen(starts_with);
  if (len_word > len_str)
    return 0;
  return strncmp(str, starts_with, len_word) == 0;
}

/* Remove first N chars from string. */
void SDC_str_remove_first_n(char *c, int n) {
  int len = strlen(c);
  if (n >= len) {
    c[0] = '\0';
    return;
  }
  memmove(c, c + n, len - n + 1);
}

/* Returns 1 if str only contains whitespace. */
int SDC_str_is_empty(const char *s) {
  int i;
  for (i = 0; s[i] != '\0'; i++) {
    if (isalpha(s[i])) {
      return 0;
    }
  }
  return 1;
}

/* IO = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =  */

#define _SDC_io_read_buffer_kb 2048 * 1000

/* Reads file into provided buffer; stack-allocated (max 2MB). Silently stops
  reading into buffer without throwing an error if stack-limit is reached.
  Returns 1 on failure. */
int SDC_io_read_entire_file(char **buffer, const char *path) {
  int c, len = 0;
  char tmp[_SDC_io_read_buffer_kb];
  FILE *f = fopen(path, "rb");
  if (!f)
    return 1;
  while ((c = fgetc(f)) != EOF) {
    if (len >= _SDC_io_read_buffer_kb)
      break;
    tmp[len++] = c;
  }
  tmp[len] = '\0';
  *buffer = tmp;
  fclose(f);
  return 0;
}

/* Reads user input into buffer with a simple prompt, finish with newline. */
/* If you don't want a prompt header, provide NULL. */
void SDC_io_prompt(char **buffer, const char *prompt_header) {
  char tmp[96] = {0};
  int c_count = 0, ch;
  if (prompt_header != NULL)
    printf("%s\n", prompt_header);
  printf("> ");
  while ((ch = getchar()) != EOF) {
    if (ch == '\n') {
      tmp[c_count] = '\0';
      SDC_str_trim(tmp);
      break;
    }
    tmp[c_count] = ch;
    c_count++;
  }
  *buffer = tmp;
}

/* DYNAMIC ARRAY = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

typedef enum {
  SDC_DA_STR,
  SDC_DA_UINT8,
  SDC_DA_INT,
  SDC_DA_LONG,
  SDC_DA_DOUBLE
} SDC_da_item_kind;

typedef struct {
  SDC_da_item_kind k;
  union {
    char *s;
    int i;
    uint8_t u8;
    long ll;
    double d;
  } v;
} SDC_da_item;

typedef struct {
  SDC_da_item *items;
  size_t count, capacity;
} SDC_da;

#define _SDC_DA_INIT_CAP 8

void SDC_da_init(SDC_da *da) {
  da->capacity = _SDC_DA_INIT_CAP;
  da->count = 0;
  da->items = malloc(sizeof(SDC_da_item) * da->capacity);
}

/* Shortens the vector, keeping the first N items and removing the rest. Returns
   1 if index out of bounds.*/
int SDC_da_truncate(SDC_da *da, const size_t idx) {
  if (idx > da->count || idx < 0 || da->count == 0)
    return 1;
  da->count = idx;
  return 0;
}

void SDC_da_free(SDC_da *da) { free(da->items); }

/* Adds new item to the back, returns 1 if failure. */
int SDC_da_push(SDC_da *da, const SDC_da_item item) {
  if (da->count == da->capacity) {
    size_t new_capacity = da->capacity * 2;
    void *new_items = realloc(da->items, new_capacity * sizeof(SDC_da_item));
    if (new_items == NULL)
      return 1;
    da->items = new_items;
    da->capacity = new_capacity;
  }
  da->items[da->count] = item;
  da->count++;
  return 0;
}

/* Pops last item from array and returns its value in 'out' (if you don't need
   the popped item, provide NULL), returns 1 if failure. */
int SDC_da_pop(SDC_da *da, SDC_da_item *out) {
  if (da->count == 0)
    return 1;
  da->count--;
  if (out != NULL)
    *out = da->items[da->count];
  return 0;
}

/* Returns a copy of the dynamic array as a static stack-allocated C array,
   i.e 'SDC_da_item array[len]'. Returns 1 on failure. */
int SDC_da_copy_to_stack(SDC_da *da, SDC_da_item new_array[],
                         size_t new_array_len) {
  size_t i;
  if (da->count == 0 || da->count != new_array_len)
    return 1;
  for (i = 0; i < da->count; i++)
    new_array[i] = da->items[i];
  return 0;
}

/* Removes item at idx, shifting all items after it to the left. Returns 1 at
   failure. */
int SDC_da_remove(SDC_da *da, const size_t idx) {
  size_t i;
  SDC_da_item next;
  if (idx > da->count || idx < 0 || da->count == 0)
    return 1;
  for (i = idx - 1; i < da->count; i++) {
    if (i + 1 < da->count) {
      next = da->items[i + 1];
      da->items[i] = next;
    }
  }
  da->count--;
  return 0;
}

/* Sugar function for retrieving current length of array. */
size_t SDC_da_len(SDC_da *da) { return da->count; }

/* Inserts new item at idx, shifting all items after it to the right. */
int SDC_da_insert(SDC_da *da, const SDC_da_item item, const size_t idx) {
  size_t i;
  SDC_da_item prev;
  if (idx > da->count || idx < 0 || da->count == 0)
    return 1;
  if (da->count == da->capacity) {
    size_t new_capacity = da->capacity * 2;
    void *new_items = realloc(da->items, new_capacity * sizeof(SDC_da_item));
    if (new_items == NULL)
      return 1;
    da->items = new_items;
    da->capacity = new_capacity;
  }

  for (i = da->count; i >= idx; i--) {
    if (i >= idx) {
      prev = da->items[i - 1];
      da->items[i] = prev;
    }
  }
  da->items[idx - 1] = item;

  da->count++;

  return 0;
}

/* HASH TABLE = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =  */

typedef struct _SDC_BucketNode _SDC_BucketNode;
struct _SDC_BucketNode {
  char *key;
  void *value;
  _SDC_BucketNode *next;
};

_SDC_internal _SDC_BucketNode *
_SDC_BucketNode_new(const char *key, const void *value, size_t value_size) {
  _SDC_BucketNode *n = (_SDC_BucketNode *)malloc(sizeof(_SDC_BucketNode));
  n->key = SDC_str_dup(key);
  n->value = malloc(value_size);
  memcpy(n->value, value, value_size);

  n->next = NULL;
  return n;
}

typedef struct {
  _SDC_BucketNode *n;
} _SDC_Bucket;

#define _SDC_HashTable_len 200

typedef struct {
  _SDC_Bucket buckets[_SDC_HashTable_len];
  size_t buckets_len;
} SDC_HashTable;

int SDC_HashTable_init(SDC_HashTable *ht) {
  size_t i;
  ht->buckets_len = _SDC_HashTable_len;
  for (i = 0; i < ht->buckets_len; i++) {
    ht->buckets[i].n =
        (_SDC_BucketNode *)_SDC_BucketNode_new("@__HEAD__@", 0, 0);
    if (!ht->buckets[i].n)
      return 1;
  }
  return 0;
}

_SDC_internal void _SDC_Bucket_insert(_SDC_Bucket *b, const char *key,
                                      void *data, size_t value_size) {
  /*TODO: insert should double as a replace*/
  _SDC_BucketNode *bn = _SDC_BucketNode_new(key, data, value_size);
  _SDC_BucketNode *last;
  if (b->n == NULL) {
    b->n = bn;
    return;
  }
  last = b->n;
  while (last->next != NULL)
    last = last->next;
  last->next = bn;
}

_SDC_internal int _SDC_Bucket_free(_SDC_Bucket *b) {
  _SDC_BucketNode *current = b->n;

  while (current != NULL) {
    _SDC_BucketNode *next = current->next;

    free(current->value);
    free(current->key);
    free(current);
    current = next;
  }

  b->n = NULL;
  return 0;
}

int SDC_HashTable_free(SDC_HashTable *ht) {
  size_t i;
  for (i = 0; i < ht->buckets_len; i++) {
    _SDC_Bucket_free(&ht->buckets[i]);
  }
  return 0;
}

#define _SDC_FNV_offset_basis 0xcbf29ce484222325
#define _SDC_FNV_prime 0x100000001b3

_SDC_internal unsigned long _SDC_fnv_hash(const char *ip,
                                          const size_t array_len) {
  size_t i, ip_len = strlen(ip);
  unsigned char byte_of_data;
  unsigned long hash = _SDC_FNV_offset_basis;
  for (i = 0; i < ip_len; i++) {
    byte_of_data = (unsigned)ip[i];
    hash *= _SDC_FNV_prime;
    hash ^= (unsigned long)byte_of_data;
  }
  return hash % array_len;
}

/* returns 0 if false, 1 if true */
int SDC_HashTable_contains_key(SDC_HashTable *ht, const char *key) {
  unsigned long idx = _SDC_fnv_hash(key, ht->buckets_len);
  _SDC_BucketNode *current;
  {
    if (ht->buckets[idx].n == NULL)
      return 0;
    current = ht->buckets[idx].n;
    while (current != NULL) {
      if (strcmp(current->key, key) == 0)
        return 1;
      current = current->next;
    }
    return 0;
  }
}

/* returns 1 on failure */
int SDC_HashTable_insert(SDC_HashTable *ht, const char *key, const void *value,
                         size_t value_size) {
  unsigned long idx = _SDC_fnv_hash(key, ht->buckets_len);
  _SDC_BucketNode *bn, *last;
  if (SDC_HashTable_contains_key(ht, key))
    return 1;
  {
    /*TODO: insert should double as a replace*/
    bn = _SDC_BucketNode_new(key, value, value_size);
    if (ht->buckets[idx].n == NULL) {
      ht->buckets[idx].n = bn;
      return 0;
    }
    last = ht->buckets[idx].n;
    while (last->next != NULL)
      last = last->next;
    last->next = bn;
  }
  return 0;
}

/* returns NULL if not found */
void *SDC_HashTable_get_value_by_key(SDC_HashTable *ht, const char *key) {
  unsigned long idx = _SDC_fnv_hash(key, ht->buckets_len);
  _SDC_BucketNode *current;
  {
    if (ht->buckets[idx].n == NULL)
      return NULL;
    current = ht->buckets[idx].n;
    while (current != NULL) {
      if (strcmp(current->key, key) == 0)
        return current->value;
      current = current->next;
    }
    return NULL;
  }
}

/* returns 1 on failure */
int SDC_HashTable_remove(SDC_HashTable *ht, const char *key) {
  unsigned long idx = _SDC_fnv_hash(key, ht->buckets_len);
  _SDC_BucketNode *current, *prev;
  {
    if (ht->buckets[idx].n == NULL)
      return 1;

    current = ht->buckets[idx].n;
    prev = NULL;

    while (current != NULL) {
      if (strcmp(current->key, key) == 0) {
        if (prev == NULL) {
          ht->buckets[idx].n = current->next;
        } else {
          prev->next = current->next;
        }
        free(current->value);
        free(current->key);
        free(current);
        return 0;
      }
      prev = current;
      current = current->next;
    }

    return 1;
  }
}

/* Modify/replace value by key */
int SDC_HashTable_modify(SDC_HashTable *ht, const char *key,
                         const void *new_value, size_t new_value_size) {
  unsigned long idx = _SDC_fnv_hash(key, ht->buckets_len);
  _SDC_BucketNode *current = ht->buckets[idx].n;

  while (current != NULL) {
    if (strcmp(current->key, key) == 0) {
      memcpy(current->value, new_value, new_value_size);
      return 0;
    }
    current = current->next;
  }

  return 1;
}

#endif /* SDC_IMPLEMENTATION */

/*

-------------------------------------------------------------------------------
Revision history:

    2026-08-30  IO, strings, rand and math
    ----------
                * Math
                    - New function for min-max range scaling.
                    - New function for clamping.
                    - New function for flooring.
                    - New function to get the char width of an integer.
                * Rand
                    - New function to get random bool.
                    - New function to get random range.
                * IO
                    - New function to read entire file.
                    - New function for simple user prompt.
                * Strings
                    - New function to check if string effectively empty.
                    - New function for appending a char to the end of a string.

    2026-08-29  Initial commit
    ----------
                * Hashtable
                    - Owns the memory of both keys and values.
                    - Hash function implemented with Fowler–Noll–Vo.
                * Dynamic array
                * Various string functions
                    - Func to dup. string.
                    - Func to trim beg. and end of string in-place.
                    - Func to check if a str starts with another.

-------------------------------------------------------------------------------
References:

https://en.wikipedia.org/wiki/Hash_table
https://www.w3schools.com/dsa/dsa_theory_hashtables.php
https://doc.rust-lang.org/std/collections/struct.HashMap.html
https://www.masaischool.com/blog/understanding-hashmap-data-structure-with-examples/
https://www.geeksforgeeks.org/dsa/singly-linked-list-tutorial/
https://www.w3schools.com/dsa/dsa_algo_linkedlists_operations.php
https://en.wikipedia.org/wiki/Non-cryptographic_hash_function
https://stackoverflow.com/questions/15821123/removing-elements-from-an-array-in-c
https://doc.rust-lang.org/std/vec/struct.Vec.html#method.remove
https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
https://www.geeksforgeeks.org/c/bitwise-operators-in-c-cpp/
https://en.wikipedia.org/wiki/Xorshift#xoroshiro
https://en.wikipedia.org/wiki/Feature_scaling

-------------------------------------------------------------------------------
License:

Copyright © 2026 Simon Danielsson

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files, to deal in the Software
without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
