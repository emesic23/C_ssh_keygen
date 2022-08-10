#include "utils.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

uint64_t hex_to_dec(char *chunk) {
    uint64_t ret = 0;
    uint64_t base = 1;
    for (int i = strlen(chunk) - 1; i >= 0; i--) {
        size_t cur = 0;
        if (chunk[i] >= '0' && chunk[i] <='9') {
            cur = (chunk[i] - '0');
        } 
        else if (chunk[i] >= 'A' && chunk[i] <='F'){
            cur = (chunk[i] - 'A' + 10);
        } 
        else if (chunk[i] >= 'a' && chunk[i] <='f'){
            cur = (chunk[i] - 'a' + 10);
        } 
        ret += cur * base;
        base *= 16;
    }
    return ret;
}

void reverse_biguint(uint64_t* arr, size_t len){
    size_t start = 0;
    size_t end = len - 1;
    while (start < end) {
        uint64_t store = arr[start];
        arr[start] = arr[end];
        arr[end] = store;
        start++;
        end--;
    }
}