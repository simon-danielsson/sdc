#include "main.h"
#define SDC_IMPLEMENTATION
#include "../sdc.h"

int main(void) {

    /*

       for (i = 0; i <= 10; i++) {
       printf("%d ", SDC_rand_range(1, 10));
       }

       return 0;
       */

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

    SDC_HashTable ht = {0};
    if (SDC_HashTable_init(&ht) != 0) {
        return 1;
    }

    SDC_HashTable_insert(&ht, "car",
            &(SDC_TYPE){.kind = SDC_STR, .as.str = "volvo"});
    SDC_HashTable_insert(&ht, "sports_car",
            &(SDC_TYPE){.kind = SDC_STR, .as.str = "ferrari"});

    SDC_HashTable_remove(&ht, "car");

    if (SDC_HashTable_contains_key(&ht, "car")) {
        printf("table contains key car\n");
    } else {
        printf("table does not contain key car\n");
    }

    if (SDC_HashTable_contains_key(&ht, "sports_car")) {
        printf("table contains key sports_car\n");
    }

    SDC_HashTable_modify(&ht, "sports_car",
            &(SDC_TYPE){.kind = SDC_STR, .as.str = "cool bugatti"});

    SDC_TYPE car = SDC_HashTable_get_value_by_key(&ht, "sports_car");

    if (car.kind == SDC_NULL)
        printf("car was not found!\n");
    else
        printf("car: %s\n", car.as.str);

    car = SDC_HashTable_get_value_by_key(&ht, "car");
    if (car.kind == SDC_NULL)
        printf("car was not found!\n");
    else
        printf("car: %s\n", car.as.str);

    SDC_HashTable_free(&ht);

    return 0;
}
