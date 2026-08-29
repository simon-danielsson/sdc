#include "main.h"
#define SDC_IMPLEMENTATION
#include "../sdc.h"

#define HT_INSERT_STR(ht, key, val)                                            \
    SDC_HashTable_insert((ht), (key), (void *)(val), strlen((val)) + 1);

/*
   To do:

   - linked list single
   - linked list double
   - arena allocator

   Done:

   - dynamic array
   - hashtable
   - logging

*/

int main(void) {
    SDC_HashTable ht = {0};
    void *car;
    double x = 40, min = 5, max = 20;
    printf("char len of x: %f\n", SDC_math_clamp(x, min, max));

    return 0;

    /*
     * char s1[] = " \t  Banana \t ";
     * char s2[] = "helloBanana";
     *
     * SDC_str_trim(s1);
     * printf("'%s'\n", s1);
     * SDC_str_remove_first_n(s2, 5);

     * if (SDC_str_starts_with(s1, s2)) {
     *     printf("true!\n");
     * } else {
     *     printf("false!\n");
     * }
     */

    if (SDC_HashTable_init(&ht) != 0) {
        return 1;
    }

    HT_INSERT_STR(&ht, "car", "volvo");
    HT_INSERT_STR(&ht, "sports_car", "ferrari");

    SDC_HashTable_remove(&ht, "car");

    if (SDC_HashTable_contains_key(&ht, "car")) {
        printf("table contains key car\n");
    } else {
    }

    if (SDC_HashTable_contains_key(&ht, "sports_car")) {
        printf("table contains key sports_car\n");
    }

    SDC_HashTable_modify(&ht, "sports_car", (void *)"cool bugatti",
            strlen("cool bugatti") + 1);

    car = SDC_HashTable_get_value_by_key(&ht, "sports_car");
    if (!car)
        printf("car was not found!\n");
    else
        printf("car: %s\n", (char *)car);

    car = SDC_HashTable_get_value_by_key(&ht, "car");
    if (!car)
        printf("car was not found!\n");
    else
        printf("car: %s\n", (char *)car);

    if (SDC_HashTable_free(&ht) != 0) {
        return 1;
    }
    return 0;

    return 0;
}
