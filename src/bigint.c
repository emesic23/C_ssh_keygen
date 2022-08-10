#include "bigint.h"
#include "utils.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// WE"RE DOING LEFT MOST IS MOST SIGNIFICANT BIT

void big_init(bigint *X) {
    X->signum = 1;
    X->num_limbs = 0;
    X->data = malloc(1);
}

void big_free(bigint *X) {
    free(X->data);
}

void big_free_all(bigint *X){
    big_free(X);
    free(X);
}

int big_copy(bigint *X, const bigint *Y) {
    X->signum = Y->signum;
    X->num_limbs = Y->num_limbs;
    free(X->data);
    X->data = malloc(Y->num_limbs * sizeof(big_uint));
    for (size_t i = 0; i < Y->num_limbs; i++) {
        X->data[i] = Y->data[i];
    }
    if (X->data){
        return 0;
    }
    return ERR_BIGINT_ALLOC_FAILED;
}

size_t big_bitlen(const bigint *X) {
    size_t pos = 63;
    while ((X->data[0] & ((big_uint) 1 << pos)) == 0) {
        pos--;
    }
    pos += 1;
    pos += (X->num_limbs - 1) * 64;
    return pos;
}

size_t big_size(const bigint *E) {
    return big_bitlen(E) / 8 + (big_bitlen(E) % 8 != 0 ? 1:0);
    // return E->num_limbs * 8;
}

int big_set_nonzero(bigint *X, big_uint limb) {
    free(X->data);
    X->num_limbs = 1;
    X->data = malloc(X->num_limbs * sizeof(big_uint));
    X->data[0] = limb; 
    if (X->data){
        return 0;
    }
    return ERR_BIGINT_ALLOC_FAILED;
}

bigint *big_create_nonzero(big_uint limb) {
    bigint *R = malloc(sizeof(bigint));
    big_init(R);
    big_set_nonzero(R, limb);
    return R;
}

int big_read_string(bigint *X, const char *s) {
    char *subset_s; 
    printf("I'm here\n");
    if (s[0] == '-') {
        subset_s = malloc(strlen(s) * sizeof(char));
        strcpy(subset_s, &s[1]);
        X->signum = -1;
    }
    else {
        subset_s = malloc((strlen(s) + 1) * sizeof(char));
        strcpy(subset_s, s);
        X->signum = 1;
    }
    printf("I'm there\n");
    size_t diff = (strlen(subset_s) % 16 != 0) ? 16 - (strlen(subset_s) % 16) : 0;
    char *padding = calloc(strlen(subset_s) + diff + 1, sizeof(char));
    for (size_t i = 0; i < diff; i++) {
        padding[i] = '0';
    }
    printf("I'm there2\n");
    
    padding[diff] = '\0';
    strcat(padding, subset_s);
    size_t num_limbs = (strlen(padding) / 16);
    big_uint *data = malloc(num_limbs * sizeof(big_uint));
    char *chunk = malloc(17);
    chunk[16] = '\0';
    size_t curr_index = 0;
    for (size_t i = 0; i < strlen(padding); i++) {
        if (i % 16 == 0 && i != 0) {
            big_uint limb = hex_to_dec(chunk);
            data[curr_index] = limb;
            curr_index++;
        }
        chunk[i % 16] = padding[i];
        if (!(padding[i] >= '0' && padding[i] <='9') && !(padding[i] >= 'A' && padding[i] <='F') && !(padding[i] >= 'a' && padding[i] <='f')) {
            free(padding);
            free(chunk);
            free(data);
            free(subset_s);
            return ERR_BIGINT_INVALID_CHARACTER;
        } 
    }
    big_uint limb = hex_to_dec(chunk);
    data[curr_index] = limb;
    free(X->data);
    X->num_limbs = num_limbs;
    X->data = data;
    free(padding);
    free(chunk);
    free(subset_s);

    if (X->data) {
        return 0;
    }
    return ERR_BIGINT_ALLOC_FAILED;
}

int big_write_string(const bigint *X,
                     char *buf, size_t buflen, size_t *olen) {
    char* chunk = malloc(sizeof(char) * 17);
    chunk[16] = '\0';
    buf[0] = '\0';
    size_t count = 1;
    if (X->signum == -1) {
        strcat(buf, "-");
        count++;
    }
    for (size_t i = 0; i < X->num_limbs; i++){
        sprintf(chunk, "%lX", X->data[i]); //Not sure if this does null terminator
        if (i != 0) {
            size_t diff = 16 - strlen(chunk);
            if (diff > 0) {
                for (size_t j = 16; j > diff - 1; j--) {
                    chunk[j] = chunk[j - diff];
                }
                for (size_t j = 0; j < diff; j++) {
                    chunk[j] = '0';
                }
            }
        }
        count += strlen(chunk);
        if (count < buflen) {
            strcat(buf, chunk);
        }
    }
    *olen = count;
    free(chunk);
    if (count >= buflen) {
        return ERR_BIGINT_BUFFER_TOO_SMALL;
    }
    return 0; //Handle errors, might want to clarify at code review how/what.
}

int big_read_binary(bigint *X, const uint8_t *buf, size_t buflen) {
    size_t padding = 0;
    if (buflen % 8 != 0) {
        padding = 8 - (buflen % 8);
    }
    uint8_t *data = malloc((buflen + padding) * sizeof(uint8_t));
    memset((void *) data, 0, padding);
    memcpy((void *) (((void *) data) + padding), buf, buflen);
}

int big_write_binary(const bigint *X, uint8_t *buf, size_t buflen) {
    size_t buf_loc = 0;
    size_t max_byte = big_size(X);
    printf("BUFLEN:%lu\n", buflen);
    printf("max_byte:%lu\n", max_byte);
    printf("curr: %llu\n", X->num_limbs);
    size_t diff = buflen - (max_byte);
    if (max_byte > buflen) {
        printf("what\n");
        return ERR_BIGINT_BUFFER_TOO_SMALL;
    }
    
    for(size_t i = 0; i < diff; i++) {
        printf("buffloc: %lu\n", diff);
        buf[buf_loc] = 0;
        buf_loc++;
        if (buf_loc >= buflen) {
            printf("fuck me\n");
            return ERR_BIGINT_BUFFER_TOO_SMALL;
        }
    }

    size_t max_bit = big_bitlen(X);
    for(size_t i = 0; i < X->num_limbs; i++){
        uint64_t cur = X->data[i];
        printf("currrrr: %llu\n", cur);
        size_t j_start;
        if (i == 0) {
            size_t ref_bit_loc = max_bit % 64;
            j_start = ref_bit_loc / 8;
            
        }
        else {j_start = 7;}
        printf("j_start: %lu\n", j_start);
        for (int j = j_start; j >=0; j--) {
            uint8_t cur_char = (uint8_t) (cur >> (j * 8));
            printf("buf_loccc: %lu\n", buf_loc);
            buf[buf_loc] = cur_char;
            buf_loc++;
            if (buf_loc >= buflen) {
                return ERR_BIGINT_BUFFER_TOO_SMALL;
            }
        }
    }
    return 0;
}

big_uint add_1x1(big_udbl a, big_udbl b, big_udbl c, big_uint* chunks, size_t idx){
    big_udbl carry = 0;
    big_udbl s1 = (a + b + c);
    carry = s1 >> 64;
    big_uint s2 = 0;
    if (carry){
        s2 = s1 - (big_udbl) UINT64_MAX - 1;
    }
    else{
        s2 = s1;
    }
    // big_uint s2 = s1 > (big_udbl) UINT64_MAX ? s1 - (big_udbl) UINT64_MAX : s1;
    chunks[idx] = s2;
    return carry;
}

int big_add(bigint *X, const bigint *A, const bigint *B) {
    if (A->signum == -1 && B->signum == 1) {
        bigint *C = malloc(sizeof(bigint));
        big_init(C);
        big_copy(C, A);
        C->signum = 1;
        big_sub(X, B, C);
        big_free_all(C);
        return 0;
    }
    if (B->signum == -1 && A->signum == 1) {
        bigint *C = malloc(sizeof(bigint));
        big_init(C);
        big_copy(C, B);
        C->signum = 1;
        big_sub(X, A, C);
        big_free_all(C);
        return 0;
    }

    if (A->signum == -1 && B->signum == -1) {
        X->signum = -1;
    }
    else {
        X-> signum = 1;
    }
    size_t num_iters = A->num_limbs > B->num_limbs ? A->num_limbs : B->num_limbs;
    
    big_uint* chunks = malloc((num_iters) * sizeof(big_uint));
    big_udbl c = 0;
    for (size_t i = 0; i < num_iters; i++){
        big_udbl a = i < A->num_limbs ? A->data[A->num_limbs - 1 - i] : 0;
        big_udbl b = i < B->num_limbs ? B->data[B->num_limbs - 1 - i] : 0;
        c = add_1x1(a, b, c, chunks, i);
    }
    if (c > 0){
        num_iters += 1;
        chunks = (big_uint *)realloc(chunks, (num_iters) * sizeof(big_uint));
        chunks[num_iters - 1] = c;
    }
    reverse_biguint(chunks, num_iters);
    free(X->data);
    X->data = chunks;
    X->num_limbs = num_iters;

    if (X->data) {
        return 0;
    }

    return ERR_BIGINT_ALLOC_FAILED;
}


big_uint sub_1x1(big_uint a, big_uint b, big_sint c, big_uint* chunks, size_t idx){
    big_uint carry = 0;
    big_uint s = 0;
    carry = (big_sdbl)a < (big_sdbl)((big_sdbl)b - c) ? 1 : 0;
    if (carry == 1){
        s = (big_uint) ((big_udbl) UINT64_MAX + a - b - c + 1) ;
    }
    else{
        s = a - b - c;
    }
    chunks[idx] = s;
    return carry;
}

//6427788810959985422885155 + 85968058272638546416180

//6341820752687346876468975 THIS IS PROPER SUB
//6341820752687346876468975 THIS IS SUB
//6513756869232623969301335 THIS IS ADD
int big_sub(bigint *X, const bigint *A, const bigint *B) {
    if (big_cmp(A, B) == 0) {    
        big_copy(X, &BIG_ZERO);
        return 0;
    }
    if (A->signum == 1 && B->signum == -1) {
        bigint *C = malloc(sizeof(bigint));
        big_init(C);
        big_copy(C, B);
        C->signum = 1;
        big_add(X, A, C);
        big_free_all(C);
        return 0;
    }
    if (B->signum == 1 && A->signum == -1) {
        bigint *C = malloc(sizeof(bigint));
        big_init(C);
        big_copy(C, B);
        C->signum = -1;
        big_add(X, A, C);
        big_free_all(C);
        return 0;
    }
    if (A->signum == -1 && B->signum == -1) {
        bigint *C = malloc(sizeof(bigint));
        big_init(C);
        big_copy(C, A);
        C->signum = 1;
        bigint *D = malloc(sizeof(bigint));
        big_init(D);
        big_copy(D, B);
        D->signum = 1;
        big_sub(X, D, C);
        big_free_all(C);
        big_free_all(D);
        return 0;
    }
    if (big_cmp_mag(A, B) < 0) {
        big_sub(X, B, A);
        X->signum *= -1;
        return 0;
    }
    size_t num_iters = A->num_limbs > B->num_limbs ? A->num_limbs : B->num_limbs;
    // might be allocating for more than needed, could cause problems down the line.
    big_uint* chunks = malloc((num_iters) * sizeof(big_uint));
    big_sint c = 0;
    for (size_t i = 0; i < num_iters; i++){
        big_udbl a = i < A->num_limbs ? A->data[A->num_limbs - 1 - i] : 0;
        big_udbl b = i < B->num_limbs ? B->data[B->num_limbs - 1 - i] : 0;
        c = sub_1x1(a, b, c, chunks, i);
    }
    reverse_biguint(chunks, num_iters);
    // This needs to change in some way...
    if (chunks[0] == 0){
        num_iters--;
        big_uint *new_chunks = malloc((num_iters) * sizeof(big_uint));;
        for (size_t i = 0; i < num_iters; i++) {
            new_chunks[i] = chunks[i + 1];
        }
        free(chunks);
        chunks = new_chunks;
    }
    free(X->data);
    X->signum = 1;
    X->data = chunks;
    X->num_limbs = num_iters;
    if (X->data) {
        return 0;
    }

    return ERR_BIGINT_ALLOC_FAILED;
}

int big_cmp_mag(const bigint *x, const bigint *y){
    if (x->num_limbs < y->num_limbs){
        return -1;
    }
    else if (y->num_limbs < x->num_limbs){
        return 1;
    }
    else{
        for (size_t i = 0; i < x->num_limbs; i++){
            if (x->data[i] < y->data[i]){
                return -1;
            }
            if (y->data[i] < x->data[i]){
                return 1;
            }
        }
    }
    return 0;
}

int big_cmp(const bigint *x, const bigint *y) {
    // is the num limbs if statement true? if x < y, the limbs are also smaller?
    if (x->signum < y->signum){
        return -1;
    }
    else if (y->signum < x->signum){
        return 1;
    }
    else if (x->num_limbs < y->num_limbs){
        return -1;
    }
    else if (y->num_limbs < x->num_limbs){
        return 1;
    }
    else if (x->signum == -1){
        return -1 * big_cmp_mag(x, y);
    }
    else if (x->signum == 1){
        return big_cmp_mag(x, y);
    }
    return 0;
}

//60CABF3B094BFFF7A2B1EBEE1EF561A351FB311C --expected
//60c8f11cfe15dc4742c422381ef561a351fb311c --ours flipped uv
//1d0b3deaf5884470000 --ours right uv
//60cabf3b94bfff7a2b1ebee1ef561a351fb311c
//60cabf3b94bfff7a2b1ebee1ef561a351fb311c

void mul1x1(big_uint w, big_uint x, big_uint y, big_uint c, big_uint* uv){
    big_udbl cur = (big_udbl) w + (big_udbl) x * (big_udbl) y + (big_udbl) c;
    uv[0] = cur >> 64;
    uv[1] = cur;
}

int big_mul(bigint *X, const bigint *A, const bigint *B) {
    if (big_cmp(A, &BIG_ZERO) == 0 || big_cmp(B, &BIG_ZERO) == 0) {
        big_copy(X, &BIG_ZERO);
        return 0;
    }
    size_t n = A->num_limbs - 1;
    size_t t = B->num_limbs - 1;
    big_uint* data = malloc((n + t + 2) * sizeof(big_uint));
    for (size_t i = 0; i <= n + t + 1; i++) {
        data[i] = 0;
    }
    big_uint* uv = malloc(2 * sizeof(big_uint));
    for (size_t i = 0; i <= t; i++) {
        big_uint c = 0;
        for (size_t j = 0; j <= n; j++) {
            // printf("THIS IS A: %d\n", A->signum);
            // big_print(A);
            mul1x1(data[i + j], A->data[(A->num_limbs - 1 - j)], B->data[(B->num_limbs - 1 - i)], c, uv);
            data[i + j] = uv[1];
            c = uv[0];
        }
        data[i + n + 1] = uv[0];
    }
    free(uv);
    reverse_biguint(data, (n + t + 2));
    size_t num_limbs;
    if (data[0] == 0) {
        num_limbs = n + t + 1;
        big_uint *new_data = malloc((n + t + 1) * sizeof(big_uint));;
        for (size_t i = 0; i < n + t + 1; i++) {
            new_data[i] = data[i + 1];
        }
        free(data);
        // new_data[0] <<= 4;
        data = new_data;
    }
    else {
        num_limbs = n + t + 2;
    }
    X->num_limbs = num_limbs;
    free(X->data);
    X->data = data;
    X->signum = A->signum * B->signum;
    if (X->data) {
        return 0;
    }

    return ERR_BIGINT_ALLOC_FAILED;
}


int big_div(bigint *Q, bigint *R, const bigint *A, const bigint *B) {

    if (big_cmp(B, &BIG_ZERO) == 0) {
        return ERR_BIGINT_DIVISION_BY_ZERO;
    }

    if (big_cmp(A, B) == 0) {
        big_set_nonzero(Q, 1);
        big_copy(R, &BIG_ZERO);
        return 0;
    }
    
    if (big_cmp(A, &BIG_ZERO) == 0) {
        big_copy(Q, &BIG_ZERO);
        big_copy(R, &BIG_ZERO);
        return 0;
    }

    if (big_cmp(A, B) < 0) {
        printf("is happening\n");
        big_copy(Q, &BIG_ZERO);
        big_copy(R, A);
        return 0;
    }
    printf("fuck me\n");

    big_uint lambda = (big_uint) 1 << 63;

    bigint *X = malloc(sizeof(bigint));
    big_init(X);
    big_copy(X, A);

    bigint *Y = malloc(sizeof(bigint));
    big_init(Y);
    big_copy(Y, B);

    size_t pow_2 = 1;
    size_t pow_2_n = 0;
    if (B->data[0] < lambda) {
        lambda = lambda / B->data[0];
        while (pow_2 < lambda) {
            pow_2 *= 2;
            pow_2_n++;
        }
        bigint *L = malloc(sizeof(bigint));
        big_init(L);
        big_set_nonzero(L, pow_2);

        big_mul(X, L, X);

        big_mul(Y, L, Y);
        big_free_all(L);
    }

    // Step 1
    size_t n = X->num_limbs - 1;
    size_t t = Y->num_limbs - 1;
    big_uint* data_q = malloc((n + 1 - t) * sizeof(big_uint));
    size_t qlimbs = n + 1 - t;
    big_uint* data_r = malloc((t + 1) * sizeof(big_uint));
    for (size_t i = 0; i < qlimbs; i++) {
        data_q[i] = 0;
    }

    // This is y * b ^(n - t)
    bigint *T = malloc(sizeof(bigint));
    big_init(T);
    big_copy(T, Y);
    T->num_limbs += n - t;
    T->data = (big_uint *)realloc(T->data, (T->num_limbs) * sizeof(big_uint));
    for (size_t i = t + 1; i < T->num_limbs; i++) {
        T->data[i] = 0;
    }

    //Step 2
    while (big_cmp(X, T) >= 0) {
        data_q[qlimbs - 1 - (n - t)] = data_q[qlimbs - 1 - (n - t)] + 1;
        big_sub(X, X, T);
    }

    //Step 3
    for (size_t i = n; i > t; i--) {
        //Step 3.1
        if (X->data[X->num_limbs - 1 - i] == Y->data[0]) {
            data_q[qlimbs - 1 - (i - t - 1)] = UINT64_MAX;
        }
        else {
            big_udbl c = ((big_udbl)X->data[X->num_limbs - 1 - i]) << 64;
            c = c | ((big_udbl) X->data[X->num_limbs - 1 - (i - 1)]);
            c /= (big_udbl) Y->data[Y->num_limbs - 1 - t];
            if (c > (big_udbl) UINT64_MAX) {
                c = (big_udbl) UINT64_MAX;
            }
            data_q[qlimbs - 1 - (i - t - 1)] = (big_uint) c;
        }

        bigint *D = malloc(sizeof(bigint));
        big_init(D);
        D->num_limbs = 3;
        D->data = (big_uint *)realloc(D->data, (D->num_limbs) * sizeof(big_uint));
        D->data[2] = (int) (i-2) < X->num_limbs ? X->data[X->num_limbs - 1 - (i - 2)] : 0;
        D->data[1] = (int) (i-1) < X->num_limbs ? X->data[X->num_limbs - 1 - (i - 1)] : 0;
        D->data[0] = X->data[X->num_limbs - 1 - i];
        
        bigint *E = malloc(sizeof(bigint));
        big_init(E);
        E->num_limbs = 2;
        E->data = (big_uint *)realloc(E->data, (E->num_limbs) * sizeof(big_uint));
        E->data[1] = ((int) t - 1 < 0) ? 0 : Y->data[Y->num_limbs - 1 - (t - 1)];
        E->data[0] = Y->data[Y->num_limbs - 1 - t];

        bigint *F = malloc(sizeof(bigint));
        big_init(F);
        big_set_nonzero(F, data_q[qlimbs - 1 - (i-t-1)]);
        
        big_mul(F, E, F);
        //Step 3.2
        while(big_cmp(F, D) == 1){
            data_q[qlimbs - 1 - (i - t - 1)] = data_q[qlimbs - 1 - (i - t - 1)] - 1;

            big_set_nonzero(F, data_q[qlimbs - 1 - (i-t-1)]);
            
            big_mul(F, E, F);
        }
        big_free_all(D);
        big_free_all(E);
        big_free_all(F);
        
        big_copy(T, Y);
        T->num_limbs += i - t - 1;
        T->data = (big_uint *)realloc(T->data, (T->num_limbs) * sizeof(big_uint));
        for (size_t j = t+1; j < T->num_limbs; j++) {
            T->data[j] = 0;
        }
        

        bigint *R = malloc(sizeof(bigint));
        big_init(R);
        big_set_nonzero(R, data_q[qlimbs - 1 - (i-t-1)]);
        big_mul(R, R, T);
        big_sub(X, X, R);
        big_free_all(R);

        if (X->signum == -1) {
            big_add(X, X, T);
            data_q[qlimbs - 1 - (i - t - 1)] = data_q[qlimbs - 1 - (i - t - 1)] - 1;
        }
        
    }
    big_copy(R, X);
    // if(R->data[0] != 0){
    //     bigint *L = malloc(sizeof(bigint));
    //     big_init(L);
    //     big_set_nonzero(L, lambda);
    //     big_div(R, tempr, R, L);
    // }
    if (big_cmp(R, &BIG_ZERO) != 0) {
        for (size_t i = 0; i < pow_2_n; i++){
            bigint *tempr = malloc(sizeof(bigint));
            big_init(tempr);
            big_div_by_2(R, tempr, R);
            big_free_all(tempr);
        }
    }
    
    Q->num_limbs = (n - t + 1);
    while (data_q[0] == 0) {
        Q->num_limbs--;
        for (size_t i = 0; i < Q->num_limbs; i++) {
            data_q[i] = data_q[i + 1];
        }
    }
    free(Q->data);
    Q->data = data_q;
    big_free_all(X);
    big_free_all(T);
    big_free_all(Y);


    if (Q->data && R->data) {
        return 0;
    }

    return ERR_BIGINT_ALLOC_FAILED;
}

int big_div_by_2(bigint *Q, bigint *R, const bigint *A) {
    if(A->num_limbs == 0){
        big_copy(Q, A);
        return 0;
    }
    big_uint* data_q = malloc(A->num_limbs * sizeof(big_uint));
    big_uint* data_r = malloc(sizeof(big_uint));
    for (size_t i = 0; i < A->num_limbs; i++) {
        data_q[i] = A->data[i];
    }
    data_r[0] = 0;

    size_t prev_carry = 0;
    size_t carry = 0;

    for (size_t i = 0; i < A->num_limbs; i++) {
        carry = 0;
        if (data_q[i] % 2 != 0) {
            data_q[i]--;
            carry = 1;
        }
        if (prev_carry) {
            big_udbl cur = (big_udbl) 1 << 64;
            cur = cur | (big_udbl) data_q[i];
            cur /= 2;
            data_q[i] = (big_uint) cur;
        }
        else {
            data_q[i] /= 2;
        }
        prev_carry = carry;
    }
    
    if (prev_carry) {
        data_r[0] = 1;
    }

    Q->num_limbs = A->num_limbs;
    if (data_q[0] == 0) {
        Q->num_limbs = A->num_limbs - 1;
        for (size_t i = 0; i < Q->num_limbs; i++) {
            data_q[i] = data_q[i + 1];
        }
        if (Q->num_limbs == 0) {
            data_q = realloc(data_q, 0);
        }
        else {
            data_q = realloc(data_q, Q->num_limbs * sizeof(big_uint));
        }
    }
    free(Q->data);
    Q->data = data_q;

    
    R->num_limbs = 1;
    if (data_r[0] == 0) {
        R->num_limbs--;
        free(data_r);
        data_r = malloc(1);
        R->num_limbs = 0;
    }    
    free(R->data);
    R->data = data_r;

    return 0;
}

int big_gcd(bigint *G, const bigint *A, const bigint *B) {
    bigint *temp_g = malloc(sizeof(bigint));
    big_init(temp_g);
    big_set_nonzero(temp_g, 1);
    int flag = 1;

    int A0 = A->num_limbs == 0;
    int B0 = B->num_limbs == 0;
    if (A0 && !B0) {
        big_copy(temp_g, B);
        flag = 0;
    }
    else if (A0 && !B0) {
        big_copy(temp_g, A);
        flag = 0;
    }
    else if (A0 && B0){
        big_copy(temp_g, &BIG_ZERO);
        flag = 0;
    }
    if (B->data[0] == 0 && A->data[0] != 0){
        big_copy(temp_g, A);
        flag = 0;
    }
    if (A->data[0] == 0 && B->data[0] != 0){
        big_copy(temp_g, B);
        flag = 0;
    }

    bigint *X = malloc(sizeof(bigint));
    big_init(X);
    big_copy(X, A);

    bigint *Y = malloc(sizeof(bigint));
    big_init(Y);
    big_copy(Y, B);
    
    bigint *two = malloc(sizeof(bigint));
    big_init(two);
    big_set_nonzero(two, 2);
    if (flag){
        while(X->data[X->num_limbs-1] % 2 == 0 && Y->data[Y->num_limbs-1]%2 == 0) {
        bigint *tempr = malloc(sizeof(bigint));
        big_init(tempr);
        big_div_by_2(X, tempr, X);
        big_div_by_2(Y, tempr, Y);
        big_mul(temp_g, temp_g, two);
        big_free_all(tempr);
        }
    
        while(big_cmp(X, &BIG_ZERO) != 0) {
            bigint *tempr = malloc(sizeof(bigint));
            big_init(tempr);
            while(big_cmp(X, &BIG_ZERO) != 0 && X->data[X->num_limbs-1] % 2 == 0) {
                big_div_by_2(X, tempr, X); 
            }
            while(big_cmp(Y, &BIG_ZERO) != 0 && Y->data[Y->num_limbs-1] % 2 == 0) {
                big_div_by_2(Y, tempr, Y); 
            }
            bigint *T = malloc(sizeof(bigint));
            big_init(T);
            big_sub(T, X, Y);
            T->signum = 1;
            if (big_cmp(T, &BIG_ZERO) != 0) {
                big_div_by_2(T, tempr, T);
            }
            if (big_cmp(X, Y) >= 0) {
                big_copy(X, T);
            }
            else {
                big_copy(Y, T);
            }
            big_free_all(T);
            big_free_all(tempr);
        }
        big_mul(temp_g, temp_g, Y);
    }

    big_free_all(X);
    big_free_all(Y);
    big_free_all(two);
    big_copy(G, temp_g);
    big_free_all(temp_g);


    if (G->data) {
        return 0;
    }
    return ERR_BIGINT_ALLOC_FAILED;
}

int big_inv_mod(bigint *X, const bigint *A, const bigint *N) {

    bigint *one = big_create_nonzero(1);
    if (big_cmp(N, one) <= 0) {
        big_free_all(one);
        return ERR_BIGINT_BAD_INPUT_DATA;
    }

    bigint *T = big_create_nonzero(1);
    big_copy(T, X);

    // GDC fucked
    big_gcd(T, A, N);
    printf("THIS IS T:\n");
    big_print(T);
    if (big_cmp(T, one) != 0) {
        printf("SHITTER\n");
        big_free_all(one);
        return ERR_BIGINT_BAD_INPUT_DATA;
    }

    if(N->data[0] == 0){
        big_copy(T, &BIG_ZERO);
        big_free_all(one);
        return ERR_BIGINT_BAD_INPUT_DATA;
    }
    if(A->data[0] == 0){
        big_free_all(one);
        big_copy(T, &BIG_ZERO);
        return 0;
    }
    printf("Args print\n");
    big_print(A);
    big_print(N);
    //Step 1
    bigint *temp_g = malloc(sizeof(bigint));
    big_init(temp_g);
    big_set_nonzero(temp_g, 1);

    bigint *Z = malloc(sizeof(bigint));
    big_init(Z);
    big_copy(Z, N);

    bigint *Y = malloc(sizeof(bigint));
    big_init(Y);
    big_copy(Y, A);
    
    bigint *two = malloc(sizeof(bigint));
    big_init(two);
    big_set_nonzero(two, 2);
    //Step 2
    while(Z->data[Z->num_limbs-1] % 2 == 0 && Y->data[Y->num_limbs-1]%2 == 0) {
        bigint *tempr = malloc(sizeof(bigint));
        big_init(tempr);
        big_div_by_2(Z, tempr, Z);
        big_div_by_2(Y, tempr, Y);
        big_mul(temp_g, temp_g, two);
        big_free_all(tempr);
    } 

    bigint *U = malloc(sizeof(bigint));
    big_init(U);
    big_copy(U, Z);

    bigint *V = malloc(sizeof(bigint));
    big_init(V);
    big_copy(V, Y);

    bigint *E = malloc(sizeof(bigint));
    big_init(E);
    big_set_nonzero(E, 1);

    bigint *B = malloc(sizeof(bigint));
    big_init(B);
    big_copy(B, &BIG_ZERO);

    bigint *C = malloc(sizeof(bigint));
    big_init(C);
    big_copy(C, &BIG_ZERO);

    bigint *D = malloc(sizeof(bigint));
    big_init(D);
    big_set_nonzero(D, 1);

    bigint *tempr = malloc(sizeof(bigint));
    big_init(tempr);
    size_t flag = 1;
    while (flag) {
        while(U->data[U->num_limbs-1] % 2 == 0) {
            big_div_by_2(U, tempr, U);
            if ((big_cmp(E, &BIG_ZERO) == 0 || E->data[E->num_limbs-1] % 2 == 0) && (big_cmp(B, &BIG_ZERO) == 0 || B->data[B->num_limbs-1] % 2 == 0)) {
                big_div_by_2(E, tempr, E);
                big_div_by_2(B, tempr, B);
            } 
            else{
                big_add(E, E, Y);
                big_div_by_2(E, tempr, E);
                big_sub(B, B, Z);
                big_div_by_2(B, tempr, B);
            }
        } 
        while(V->data[V->num_limbs-1] % 2 == 0) {
            big_div_by_2(V, tempr, V);
             
            if ((big_cmp(C, &BIG_ZERO) == 0 || C->data[C->num_limbs-1] % 2 == 0) && (big_cmp(D, &BIG_ZERO) == 0 || D->data[D->num_limbs-1] % 2 == 0)) {
                big_div_by_2(C, tempr, C);
                big_div_by_2(D, tempr, D);
            } 
            else{
                big_add(C, C, Y);
                big_div_by_2(C, tempr, C);
                big_sub(D, D, Z);
                big_div_by_2(D, tempr, D);
            }
        }

        if (big_cmp(U, V) >= 0){
            big_sub(U, U, V);
            big_sub(E, E, C);
            big_sub(B, B, D);
        }
        else {
            big_sub(V, V, U);
            big_sub(C, C, E);
            big_sub(D, D, B);
        }
        if (big_cmp(U, &BIG_ZERO) == 0) {
            flag = 0;
        } 
    }


    if(big_cmp(V, one) == 0) {
        if (big_cmp(D, &BIG_ZERO) > 0) {
            big_copy(T, D);
        }
        else {
            big_add(T, D, N);
        } 
    }
    else {
        big_free_all(one);
        big_free_all(temp_g);
        big_free_all(tempr);
        big_free_all(two);
        big_free_all(Z);
        big_free_all(Y);
        big_free_all(U);
        big_free_all(V);
        big_free_all(E);
        big_free_all(B);
        big_free_all(C);
        big_free_all(D);
        printf("FUCKER\n");
        return ERR_BIGINT_NOT_ACCEPTABLE;
    }

    big_copy(X, T);
    big_free(T);
    big_free_all(one);
    big_free_all(temp_g);
    big_free_all(tempr);
    big_free_all(two);
    big_free_all(Z);
    big_free_all(Y);
    big_free_all(U);
    big_free_all(V);
    big_free_all(E);
    big_free_all(B);
    big_free_all(C);
    big_free_all(D);
    
    if (X->data) {
        return 0;
    }
    return ERR_BIGINT_ALLOC_FAILED;
}

int big_mont(bigint *Q, const bigint *A, const bigint *B, const bigint *M, bigint *_MP) {
    bigint *MP = malloc(sizeof(bigint));
    big_init(MP);

    bigint *base = malloc(sizeof(bigint));
    big_init(base);
    base->num_limbs = 2;
    base->data = (big_uint *)realloc(base->data, (base->num_limbs) * sizeof(big_uint));
    base->data[1] = 0;
    base->data[0] = 1;

    if (_MP) {
        big_copy(MP, _MP);
    }
    else {
        big_inv_mod(MP, M, base);
        MP->signum *= -1;
    }
    
    size_t x_limbs = M->num_limbs + 2;
    big_uint *data_x = malloc((x_limbs) * sizeof(big_uint));
    for (size_t i = 0; i < x_limbs; i++) {
        data_x[i] = 0;
    }
    bigint* X = malloc(sizeof(bigint));
    big_init(X);
    X->num_limbs = x_limbs;
    free(X->data);
    X->data = data_x;

    bigint* U = malloc(sizeof(bigint));
    big_init(U);
    bigint* temp = malloc(sizeof(bigint));
    big_init(temp);
    bigint *B0 = big_create_nonzero(B->data[B->num_limbs - 1]);
    for (size_t i = 0; i < x_limbs; i++) {
        bigint *X0 = big_create_nonzero(X->data[x_limbs - 1]);
        bigint *AI = big_create_nonzero(A->data[A->num_limbs - 1 - i]);
        printf("FIRST BIG_MUL ARGS\n");
        big_print(B0);
        big_print(AI);
        big_mul(U, B0, AI);
        big_add(U, X0, U);
        printf("SECOND BIG_MUL\n");
        big_print(MP);
        big_print(U);
        big_mul(U, MP, U);
        printf("AFTER SECOND BIG MUL\n");
        big_print(U);
        big_print(base);
        
        big_div(temp, U, U, base);
        printf("AFTER SECOND BIGDIV\n");
        big_print(U);

        

        printf("THIRD BIG_MUL\n");
        big_print(U);
        big_print(M);
        big_mul(U, U, M);

        printf("FOURTH BIG_MUL\n");
        big_print(AI);
        big_print(B);
        big_mul(AI, AI, B);
        big_add(X, X, AI);
        big_add(X, X, U);
        big_div(X, temp, X, base);

        free(AI);
        free(X0);
    }
    free(temp);
    free(U);
    free(B0);

    if (big_cmp(X, M) >= 0) {
        big_sub(X, X, M);
    }

    big_copy(Q, X);

    big_free_all(MP);
    return 0;
}



size_t big_get_bit(bigint *E, size_t pos) {
    size_t limb = pos / 64;
    size_t idx = pos % 64;
    return (E->data[limb] && (1 << idx));
}

int big_exp_mod(bigint *Q, const bigint *A, const bigint *E, const bigint *N,
                bigint *_RR) {
    if (big_cmp(N, &BIG_ZERO) < 0 || big_cmp(E, &BIG_ZERO) < 0) {
        return ERR_BIGINT_BAD_INPUT_DATA;
    }
    if (N->data[N->num_limbs - 1] % 2 == 0) {
        return ERR_BIGINT_BAD_INPUT_DATA;
    }
    bigint *X = malloc(sizeof(bigint));
    big_init(X);

    bigint *R = malloc(sizeof(bigint));
    big_init(R);
    R->num_limbs = N->num_limbs + 1;
    free(R->data);
    R->data = malloc(R->num_limbs * sizeof(big_uint)) ;
    for (size_t i = 0; i < R->num_limbs; i++) {
        R->data[i] = 0;
    }
    R->data[0] = 1;

    bigint *RR = malloc(sizeof(bigint));
    big_init(RR);

    bigint *tempq = malloc(sizeof(bigint));
    big_init(tempq); 
    if (_RR) {
        big_copy(RR, _RR);
    }
    else {
        big_mul(RR, R, R);
        big_div(tempq, RR, RR, N);
    }

    printf("THIS IS R:\n");
    big_print(R);
    printf("THIS IS N:\n");
    big_print(N);

    printf("THIS IS COMPARISON: %d\n", big_cmp(R, N));

    big_div(tempq, R, R, N);
    free(tempq);

    bigint *NP = malloc(sizeof(bigint));
    big_init(NP);

    bigint *base = malloc(sizeof(bigint));
    big_init(base);
    base->num_limbs = 2;
    base->data = (big_uint *)realloc(base->data, (base->num_limbs) * sizeof(big_uint));
    base->data[1] = 0;
    base->data[0] = 1;
    big_inv_mod(NP, N, base);
    NP->signum *= -1;

    printf("ABOUT TO DO BIG MONT X============\n");
    bigint *x_tilde = malloc(sizeof(bigint));
    big_init(x_tilde); 
    big_mont(x_tilde, A, RR, N, NP);
    big_copy(X, R);
    printf("FUCKING HELL============X\n");

    printf("ABOUT TO DO BIG MONT============\n");
    size_t t = big_size(E);
    for (int i = t; i >= 0; i--) {
        big_mont(X, X, X, N, NP);
        size_t ei = big_get_bit(E, i);
        if (ei == 1) {
            big_mont(X, X, x_tilde, N, NP);
        }
    }
    printf("FUCKING HELL============\n");
    bigint *one = big_create_nonzero(1);
    big_mont(X, X, one, N, NP);
    free(one);
    free(base);
    free(R);
    free(RR);
    free(NP);

    big_copy(Q, X);
    big_free_all(X);


    if (Q->data) {
        return 0;
    }
    return ERR_BIGINT_ALLOC_FAILED;
}

uint64_t rand_64() {
    uint64_t ret = 0;
    size_t shift = sizeof(RAND_MAX) * 8;
    for (size_t i = 0; i < (64 / shift); i++) {
        ret = ret << shift;
        ret = ret | rand();
    }
    return ret;
}

bool big_miller(const bigint* N){
    bigint* one = big_create_nonzero(1);
    bigint* two = big_create_nonzero(2);
    bigint* temp = malloc(sizeof(bigint));
    big_init(temp);
    big_sub(temp, N, one);

    bigint* n1 = malloc(sizeof(bigint));
    big_init(n1);
    big_copy(n1, temp);
    
    bigint* t = malloc(sizeof(bigint));
    big_init(t);
    big_set_nonzero(t, 0);
    
    bigint* tempr = malloc(sizeof(bigint));
    big_init(tempr);
    while(temp->data[temp->num_limbs-1] % 2 == 0){
        big_div_by_2(temp, tempr, temp);
        big_add(t, t, one);
    }
    big_free_all(tempr);

    big_sub(t, t, one);
    for (size_t i = 0; i < 64; i++){
        uint64_t mask = (1 << big_bitlen(N)) - 1;
        uint64_t cur = rand_64();
        cur = cur | mask;
        bigint* curr = big_create_nonzero(cur);
        bigint* V = malloc(sizeof(bigint));
        big_init(V);
        big_exp_mod(V, curr, temp, N, NULL);
        if (big_cmp(V, one) != 0){
            bigint* I = big_create_nonzero(0);
            while (big_cmp(V, n1) != 0){
                if (big_cmp(I, t) == 0)
                {   
                    big_free_all(one);
                    big_free_all(two);
                    big_free_all(n1);
                    big_free_all(temp);
                    big_free_all(t);
                    return false;
                }
                else{
                    big_add(I, I, one);
                    big_exp_mod(V, V, two, N, NULL);
                }
            }
        }
        big_free_all(V);
        big_free_all(curr);
    }
    big_free_all(one);
    big_free_all(two);
    big_free_all(n1);
    big_free_all(temp);
    big_free_all(t);
    return true;
}

int big_is_prime(const bigint *X) {
    bigint* one = big_create_nonzero(1);
    bigint* three = big_create_nonzero(3);
    if (big_cmp(X, three) < 0){
        return 0;
    }      
    if (X->data[X->num_limbs-1] % 2 == 0){
        return false;
    }
    bigint *tempr = big_create_nonzero(1);
    bigint *tempq = big_create_nonzero(1);
    for(size_t i = 2; i < 50; i++){
        bigint* temp = big_create_nonzero(i);
        big_div(tempq, tempr, X, temp);
        if (big_cmp(tempr, &BIG_ZERO) == 0) {
            return false;
        }
        free(temp);
    }
    big_free_all(tempq);
    big_free_all(tempr);
    big_free_all(one);
    big_free_all(three);

    bool prime = big_miller(X);
    if (prime) {
        return 0;
    }
    else {
        return ERR_BIGINT_NOT_ACCEPTABLE;
    }
}



int big_gen_prime(bigint *X, size_t nbits) {
    if (nbits < 3) {
        return ERR_BIGINT_BAD_INPUT_DATA;
    }
    uint64_t mask = (1 << nbits) - 1;
    while (true) {
        big_free_all(X);
        big_init(X);
        X->num_limbs = nbits % 64;
        for (size_t i = 0; i < X->num_limbs; i++) {
            uint64_t cur = rand_64();
            if (i == 0) {
                cur = cur | mask;
            }
            if (i == X->num_limbs - 1) {
                cur = cur | 1;
            }
            X->data[i] = cur;
        }
        if (big_is_prime(X)) {
            return 0;
        }
    }
    if (X->data) {
        return 0;
    }
    return ERR_BIGINT_ALLOC_FAILED;
}

void big_print(bigint *X) {
    for (size_t i = 0; i < X->num_limbs; i++){
        printf("Limb %zu: %lX\n", i + 1, X->data[i]);
    }
}
