# Examples

## Number guessing game

### API
- `int SDC_rand_range(const int floor, const int ceiling)`
- `void SDC_io_prompt(char **buffer, const char *prompt_header)`

### Code
``` c
#define SDC_IMPLEMENTATION
#include "../../sdc/sdc.h"

int main(void) {
    int rand_num = SDC_rand_range(1, 10), guess_counter = 1;
    char *entry_buffer, *prompt = "Guess a number between 1 and 10!";
    for (;; guess_counter++) {
        SDC_io_prompt(&entry_buffer, prompt);
        if (atoi(entry_buffer) == rand_num)
            break;
        else if (atoi(entry_buffer) > rand_num)
            prompt = "Lower!";
        else
            prompt = "Higher!";
    }
    printf("You guessed correctly after %d guesses!\n", guess_counter);

    return 0;
}
```

## Check if string ends with suffix

### API
- `int SDC_str_ends_with(const char *s, const char *suffix)`
- `void SDC_str_remove_first_n(char *s, int n)`

### Code
``` c
int main(void) {
    char s[] = "usr/bin/ls";
    char *find = "ls";

    int idx = SDC_str_ends_with(s, find);
    if (idx != -1) {
        SDC_str_remove_first_n(s, idx);
        printf("found %s at index %d!\n", s, idx);
    } else {
        printf("did not find %s!\n", find);
    }
    return 0;
}
```

## Replace substrings of a string

### API
- `char *SDC_str_replace_substr(const char *s, const char *sub, const char *new_sub)`

### Code
``` c
#define SDC_IMPLEMENTATION
#include "../../sdc/sdc.h"

int main(void) {
    char *s = "This is a hello, and a goodbye. Another hello, or goodbye and "
        "hello at last.\n";
    printf("raw string: %s", s);

    char *new_s = SDC_str_replace_substr(s, "hello", "goodbye");
    printf("new string: %s", new_s);
    free(new_s);

    return 0;
}
```

## Split a string and sort its words alphabetically

### API
- `bool SDC_str_split_by_delim(SDC_da *da, const char *input, const size_t input_len, const char delim)`
- `void SDC_str_remove_special_chars(char *str)`
- `bool SDC_da_qsort(SDC_da *da, int (*comp)(const void *, const void *))`
- `void SDC_da_free(SDC_da *da)`

### Code
``` c
#define SDC_IMPLEMENTATION
#include "../../sdc/sdc.h"

int da_sort_alph(const void *a, const void *b) {
    SDC_TYPE arg1 = *(const SDC_TYPE *)a;
    SDC_TYPE arg2 = *(const SDC_TYPE *)b;

    if (tolower(arg1.as.str[0]) < tolower(arg2.as.str[0]))
        return -1;
    if (tolower(arg1.as.str[0]) > tolower(arg2.as.str[0]))
        return 1;
    return 0;
}

int main(void) {
    char *s = "This is a string that I want to split by whitespace.";
    SDC_da da = {0};
    SDC_str_split_by_delim(&da, s, strlen(s), ' ');

    SDC_da_qsort(&da, da_sort_alph);

    for (size_t i = 0; i < SDC_da_len(&da); i++) {
        SDC_str_remove_special_chars(da.items[i].as.str);
        printf("%s\n", da.items[i].as.str);
    }
    SDC_da_free(&da);

    return 0;
}
```

## Word frequency counter

### API
- `int SDC_HashTable_init(SDC_HashTable *ht)`
- `bool SDC_str_split_by_delim(SDC_da *da, const char *input, const size_t input_len, const char delim)`
- `void SDC_str_remove_special_chars(char *str)`
- `void SDC_str_lower(char *s)`
- `bool SDC_HashTable_insert(SDC_HashTable *ht, const char *key, SDC_TYPE *value)`
- `SDC_TYPE SDC_HashTable_get_value_by_key(SDC_HashTable *ht, const char *key)`
- `bool SDC_HashTable_modify(SDC_HashTable *ht, const char *key, const SDC_TYPE *new_value)`
- `void SDC_da_free(SDC_da *da)`
- `SDC_KV *SDC_HashTable_reduce_to_array(SDC_HashTable *ht, size_t *out_len)`
- `void SDC_HashTable_free(SDC_HashTable *ht)`
- `void SDC_KV_array_free(SDC_KV *array, const size_t array_len)`

### Code
``` c
#define SDC_IMPLEMENTATION
#include "../../sdc/sdc.h"

void err(const char *msg) {
    fprintf(stderr, "[ERROR] %s\n", msg);
    exit(1);
}

int words_sort(const void *a, const void *b) {
    SDC_KV arg1 = *(const SDC_KV *)a;
    SDC_KV arg2 = *(const SDC_KV *)b;

    if (arg1.val.as.i < arg2.val.as.i)
        return -1;
    if (arg1.val.as.i > arg2.val.as.i)
        return 1;
    return 0;
}

void count_occurences(const char *file_contents,
        const size_t file_contents_len) {
    SDC_HashTable ht = {0};
    SDC_HashTable_init(&ht);

    SDC_da da = {0};

    SDC_str_split_by_delim(&da, file_contents, file_contents_len, ' ');

    for (size_t i = 0; i < SDC_da_len(&da); i++) {
        char *word = da.items[i].as.str;
        SDC_str_remove_special_chars(word);
        SDC_str_lower(word);
        if (!SDC_HashTable_insert(&ht, word,
                    &(SDC_TYPE){.kind = SDC_INT, .as.i = 1})) {
            SDC_TYPE new_val = SDC_HashTable_get_value_by_key(&ht, word);
            if (!SDC_HashTable_modify(
                        &ht, word,
                        &(SDC_TYPE){.kind = SDC_INT, .as.i = new_val.as.i + 1})) {
                err("Occurence increment failure!");
            }
        }
    }
    SDC_da_free(&da);

    size_t array_len;
    SDC_KV *array = SDC_HashTable_reduce_to_array(&ht, &array_len);

    if (array == NULL)
        err("Failed to reduce table");

    qsort(array, array_len, sizeof(*array), words_sort);

    for (size_t i = 0; i < array_len; i++)
        printf("%s: %d\n", array[i].key, array[i].val.as.i);

    SDC_HashTable_free(&ht);
    SDC_KV_array_free(array, array_len);
}

int main(int argc, char *argv[]) {
    char *file_contents;

    if (argc < 2 || argv[1] == NULL) {
        err("No filepath was provided!");
    }

    SDC_io_read_entire_file(&file_contents, argv[1]);
    count_occurences(file_contents, strlen(file_contents));

    return 0;
}
```

## Configuration file parser

### API
- `int SDC_HashTable_init(SDC_HashTable *ht)`
- `bool SDC_str_split_by_delim(SDC_da *da, const char *input, const size_t input_len, const char delim)`
- `bool SDC_HashTable_insert(SDC_HashTable *ht, const char *key, SDC_TYPE *value)`
- `SDC_TYPE SDC_HashTable_get_value_by_key(SDC_HashTable *ht, const char *key)`
- `void SDC_da_free(SDC_da *da)`
- `void SDC_HashTable_free(SDC_HashTable *ht)`
- `bool SDC_io_read_entire_file(char *buffer, const char *path)`
- `bool SDC_str_starts_with(const char *str, const char *starts_with)`
- `bool SDC_str_is_empty(const char *s)`

### Code

``` c
#define SDC_IMPLEMENTATION
#include "../../sdc/sdc.h"
#include <locale.h>

bool parse_conf_file(SDC_HashTable *ht, char *conf_contents, const char *_,
        const char *syntax_comm) {
    if (!ht || !syntax_comm || !conf_contents)
        return false;
    SDC_da lines = {0};
    SDC_str_split_by_delim(&lines, conf_contents, strlen(conf_contents), '\n');

    setlocale(LC_ALL, "en_US.utf8");

    for (size_t i = 0; i < SDC_da_len(&lines); i++) {
        if (SDC_str_starts_with(lines.items[i].as.str, syntax_comm) ||
                SDC_str_is_empty(lines.items[i].as.str)) {
            continue;
        }
        char key[48], value[48], sep[4];
        sscanf(lines.items[i].as.str, "%s%s%s", key, sep, value);
        if (!SDC_HashTable_insert(ht, key,
                    &(SDC_TYPE){.as.str = value, .kind = SDC_STR})) {
            return false;
        };
    }

    SDC_da_free(&lines);

    return true;
}

int main(void) {
    char conf_contents[_SDC_io_read_buffer_kb];
    {
        SDC_io_read_entire_file(conf_contents, "settings.conf");
    }

    SDC_HashTable conf = {0};
    {
        SDC_HashTable_init(&conf);
        if (!parse_conf_file(&conf, conf_contents, "=", "#")) {
            printf("Error: failed to parse config file\n");
        }
    }

#define GET_VALUE(key)                                                         \
    {                                                                            \
        SDC_TYPE k = SDC_HashTable_get_value_by_key(&conf, (key));                 \
        if (k.kind != SDC_STR)                                                     \
        printf("Error: field '%s' either missing or malformed\n", (key));        \
        else                                                                       \
        printf("%-20s%-8s\n", (key), k.as.str);                                  \
    }

    GET_VALUE("host");
    GET_VALUE("port");
    GET_VALUE("debug");
    GET_VALUE("log_level");
    GET_VALUE("log_file");
    GET_VALUE("max_log_size_mb");

    SDC_HashTable_free(&conf);

    return 0;
}
```
