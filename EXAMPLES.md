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
- `SDC_HashTable_KV *SDC_HashTable_reduce_to_array(SDC_HashTable *ht, size_t *out_len)`
- `void SDC_HashTable_free(SDC_HashTable *ht)`
- `void SDC_HashTable_KV_array_free(SDC_HashTable_KV *array, const size_t array_len)`

### Code
``` c
#define SDC_IMPLEMENTATION
#include "../../sdc/sdc.h"

void err(const char *msg) {
    fprintf(stderr, "[ERROR] %s\n", msg);
    exit(1);
}

int words_sort(const void *a, const void *b) {
    SDC_HashTable_KV arg1 = *(const SDC_HashTable_KV *)a;
    SDC_HashTable_KV arg2 = *(const SDC_HashTable_KV *)b;

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
    SDC_HashTable_KV *array = SDC_HashTable_reduce_to_array(&ht, &array_len);

    if (array == NULL)
        err("Failed to reduce table");

    qsort(array, array_len, sizeof(*array), words_sort);

    for (size_t i = 0; i < array_len; i++)
        printf("%s: %d\n", array[i].key, array[i].val.as.i);

    SDC_HashTable_free(&ht);
    SDC_HashTable_KV_array_free(array, array_len);
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

