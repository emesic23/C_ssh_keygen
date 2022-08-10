#include "bigint.h"

#include <stdio.h>

FILE *get_dev_urandom(void) {
    static FILE *file;
    if (file == NULL) file = fopen("/dev/urandom", "r");
    return file;
}

int big_fill_random(bigint *x, size_t size) {
    FILE *random = get_dev_urandom();
    if (random == NULL) return ERR_BIGINT_ALLOC_FAILED;

    uint8_t buffer[size];
    size_t read = fread(buffer, 1, size, random);
    if (read != size) return ERR_BIGINT_ALLOC_FAILED;

    return big_read_binary(x, buffer, size);
}
