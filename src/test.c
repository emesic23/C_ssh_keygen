#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bigint.h"
#include <string.h>
#include <assert.h>

char* see_bigint(bigint* X){
    char *buff = malloc(4096);
    size_t *why = malloc(sizeof(size_t));
    big_write_string(X, buff, 4096, why);
    free(why);
    return buff;
}

void run_test(char* x, char* y, int (*func)(bigint*, bigint*, const bigint*), char* check){
    bigint *X = malloc(sizeof(bigint));
    big_init(X);
    big_read_string(X, x);

    big_print(X);
    char *x_buff = see_bigint(X);
    assert(!strcmp(x_buff, x));
    free(x_buff);

    bigint *Y = malloc(sizeof(bigint));
    big_init(Y);
    big_read_string(Y, y);


    char *y_buff = see_bigint(Y);
    assert(!strcmp(y_buff, y));
    free(y_buff);

    int result = (*func)(X, X, Y);
    char *buff = see_bigint(X);
    printf("WHAT:%s THE FUCK:%s\n", buff, check);
    if (check[0] != '0'){
        assert(!strcmp(buff, check));
    }
    // assert(!strcmp(buff, check));
    free(buff);
    big_free_all(X);
    big_free_all(Y);
}

void run_test_binary(char* x, int (*func)(const bigint*, uint8_t*, size_t), char* check){
    bigint *X = malloc(sizeof(bigint));
    big_init(X);
    big_read_string(X, x);

    big_print(X);
    char *x_buff = see_bigint(X);
    printf("x_buff: %s\n", x_buff);
    printf("x?: %llu\n", X->data[0]);
    assert(!strcmp(x_buff, x));
    free(x_buff);

    size_t olen = (big_bitlen(X) / 8 + (big_bitlen(X) % 8 != 0 ? 1:0));
    printf("BITLEN: %lu\n", big_bitlen(X));
    uint8_t *buff = malloc((olen + 1) * sizeof(uint8_t));
    buff[olen] = '\0';
    printf("OLEN: %lu\n", olen);

    int result = (*func)(X, buff, olen);
    printf("WHAT:%s THE FUCK:%s\n", buff, check);
    printf("%x\n", buff[0]);
    printf("%x\n", buff[1]);
    assert(!strcmp(buff, check));
    // assert(!strcmp(buff, check));
    free(buff);
    big_free_all(X);
}


void run_testcmp(char* x, char* y, int (*func)(const bigint*, const bigint*), char* check){
    bigint *X = malloc(sizeof(bigint));
    big_init(X);
    big_read_string(X, x);

    bigint *Y = malloc(sizeof(bigint));
    big_init(Y);
    big_read_string(Y, y);

    int result = (*func)(X, Y);
    char str[3];
    
    sprintf(str, "%d", result);
    assert(!strcmp(str, check));
    big_free_all(X);
    big_free_all(Y);
}

void run_testdiv(char* x, char* y, int (*func)(bigint*, bigint*, const bigint*, const bigint*), char* checkq, char* checkr){
    bigint *X = malloc(sizeof(bigint));
    big_init(X);
    big_read_string(X, x);
    
    bigint *Y = malloc(sizeof(bigint));
    big_init(Y);
    big_read_string(Y, y);

    bigint *Q = malloc(sizeof(bigint));
    big_init(Q);

    bigint *R = malloc(sizeof(bigint));
    big_init(R);

    int result = (*func)(Q, R, X, Y);
    char *buff_Q = see_bigint(Q);
    printf("WHATQ: %s THE FUCKQ: %s\n", buff_Q, checkq);
    big_print(Q);
    assert(!strcmp(buff_Q, checkq));

    char *buff_R = see_bigint(R);
    printf("WHATR: %s THE FUCKR: %s\n", buff_R, checkr);
    big_print(R);
    if (checkr[0] != '0'){
        assert(!strcmp(buff_R, checkr));
    }

    free(buff_Q);
    free(buff_R);
    big_free_all(X);
    big_free_all(Y);
    big_free_all(Q);
    big_free_all(R);
}

void run_testmont(char* a, char* e, char* m, int (*func)(bigint*, const bigint*, const bigint*, const bigint*, bigint*), char* check){
    bigint *A = malloc(sizeof(bigint));
    big_init(A);
    big_read_string(A, a);
    
    bigint *E = malloc(sizeof(bigint));
    big_init(E);
    big_read_string(E, e);

    bigint *M = malloc(sizeof(bigint));
    big_init(M);
    big_read_string(M, m);

    // bigint *_RR = malloc(sizeof(bigint));
    // big_init(_RR);

    bigint *X = malloc(sizeof(bigint));
    big_init(X);

    int result = (*func)(X, A, E, M, NULL);
    char *buff = see_bigint(X);
    printf("WHAT: %s THE FUCKQ: %s\n", buff, check);
    big_print(X);
    assert(!strcmp(buff, check));

    free(buff);
    big_free_all(X);
    big_free_all(A);
    big_free_all(E);
    big_free_all(M);
    // big_free(_RR);
}

void test_wrap_sequential(char* input_fname, char* ref_fname, int (*func)(bigint*, bigint*, const bigint*)){
    FILE* inputs = fopen(input_fname, "r");
    if (inputs == NULL){
        exit(EXIT_FAILURE);
    }

    FILE* ref = fopen(ref_fname, "r");
    if (ref == NULL){
        exit(EXIT_FAILURE);
    }

    char* inputline = NULL;
    size_t inputlen = 0;

    char* refline = NULL;
    size_t reflen = 0;

    int flag = 1;
    char* prev = NULL;
    while(getline(&inputline, &inputlen, inputs) != -1){
        inputline[strcspn(inputline, "\n")] = 0;
        if (flag == 1){
            prev = malloc(inputlen-1);
            prev = strcpy(prev, inputline);
            flag = 0;
            continue;
        }

        printf("input1:%s input2:%s\n", prev, inputline);
        getline(&refline, &reflen, ref);
        refline[strcspn(refline, "\n")] = 0;
        run_test(prev, inputline, func, refline);

        free(prev);
        prev = malloc(inputlen-1);
        prev = strcpy(prev, inputline);
    }
    flag = 1;
    free(prev);
    free(inputline);
    free(refline);
    fclose(ref);
}

void test_wrap_single(char* input_fname, char* ref_fname, int (*func)(const bigint, uint8_t, size_t)){
    FILE* inputs = fopen(input_fname, "r");
    if (inputs == NULL){
        exit(EXIT_FAILURE);
    }

    FILE* ref = fopen(ref_fname, "r");
    if (ref == NULL){
        exit(EXIT_FAILURE);
    }

    char* inputline = NULL;
    size_t inputlen = 0;

    char* refline = NULL;
    size_t reflen = 0;

    while(getline(&inputline, &inputlen, inputs) != -1){
        inputline[strcspn(inputline, "\n")] = 0;

        printf("input1:%s\n", inputline);
        getline(&refline, &reflen, ref);
        refline[strcspn(refline, "\n")] = 0;
        run_test_binary(inputline, func, refline);
    }
    free(inputline);
    free(refline);
    fclose(ref);
}

void test_wrap_cmp(char* input_fname, char* ref_fname, int (*func)(bigint*, bigint*)){
    FILE* inputs = fopen(input_fname, "r");
    if (inputs == NULL){
        exit(EXIT_FAILURE);
    }

    FILE* ref = fopen(ref_fname, "r");
    if (ref == NULL){
        exit(EXIT_FAILURE);
    }

    char* inputline = NULL;
    size_t inputlen = 0;

    char* refline = NULL;
    size_t reflen = 0;

    int flag = 1;
    char* prev = NULL;
    while(getline(&inputline, &inputlen, inputs) != -1){
        inputline[strcspn(inputline, "\n")] = 0;
        if (flag == 1){
            prev = malloc(inputlen-1);
            prev = strcpy(prev, inputline);
            flag = 0;
            continue;
        }

        printf("input1:%s input2:%s\n", prev, inputline);
        getline(&refline, &reflen, ref);
        refline[strcspn(refline, "\n")] = 0;
        run_testcmp(prev, inputline, func, refline);

        free(prev);
        prev = malloc(inputlen-1);
        prev = strcpy(prev, inputline);
    }
    flag = 1;
    free(prev);
    free(inputline);
    free(refline);
    fclose(ref);
}

void test_wrap_div(char* input_fname, char* ref_fname, int (*func)(bigint*, bigint*, const bigint*, const bigint*)){
    FILE* inputs = fopen(input_fname, "r");
    if (inputs == NULL){
        exit(EXIT_FAILURE);
    }

    FILE* ref = fopen(ref_fname, "r");
    if (ref == NULL){
        exit(EXIT_FAILURE);
    }

    char* inputline = NULL;
    size_t inputlen = 0;

    char* refqline = NULL;
    size_t refqlen = 0;

    char* refrline = NULL;
    size_t refrlen = 0;

    int flag = 1;
    char* prev = NULL;
    while(getline(&inputline, &inputlen, inputs) != -1){
        inputline[strcspn(inputline, "\n")] = 0;
        if (flag == 1){
            prev = malloc(inputlen-1);
            prev = strcpy(prev, inputline);
            flag = 0;
            continue;
        }
        getline(&refqline, &refqlen, ref);
        getline(&refrline, &refrlen, ref);
        refqline[strcspn(refqline, "\n")] = 0;
        refrline[strcspn(refrline, "\n")] = 0;
        // printf("%s, %s", prev, line);
        run_testdiv(prev, inputline, func, refqline, refrline);

        free(prev);
        flag = 1;
    }
    flag = 1;
    free(inputline);
    free(refqline);
    free(refrline);
    fclose(ref);
}

void test_wrap_mont(char* input_fname, char* ref_fname, int (*func)(bigint*, const bigint*, const bigint*, const bigint*, bigint*)){
    FILE* inputs = fopen(input_fname, "r");
    if (inputs == NULL){
        exit(EXIT_FAILURE);
    }

    FILE* ref = fopen(ref_fname, "r");
    if (ref == NULL){
        exit(EXIT_FAILURE);
    }

    char* inputline = NULL;
    size_t inputlen = 0;

    char* refline = NULL;
    size_t reflen = 0;

    int flag = 1;
    char* A = NULL;
    char* E = NULL;
    while(getline(&inputline, &inputlen, inputs) != -1){
        inputline[strcspn(inputline, "\n")] = 0;
        if (flag == 1){
            A = malloc(inputlen-1);
            A = strcpy(A, inputline);
            flag = 2;
            continue;
        }
        if (flag == 2){
            E = malloc(inputlen-1);
            E = strcpy(E, inputline);
            flag = 0;
            continue;
        }
        getline(&refline, &reflen, ref);
        refline[strcspn(refline, "\n")] = 0;
        run_testmont(A, E, inputline, func, refline);
        free(A);
        free(E);
        flag = 1;
    }
    flag = 1;
    free(A);
    free(inputline);
    free(refline);
    fclose(ref);
}

void test(){
    // printf("STARTING BINARY==============================\n");
    // test_wrap_single("nums_binary.txt", "ref_binary.txt", &big_write_binary);
    // printf("DONE BINARY\n");

    printf("STARTING ADD==============================\n");
    test_wrap_sequential("nums.txt", "ref_add.txt", &big_add);
    printf("DONE ADD\n");

    printf("STARTING SUB==============================\n");
    test_wrap_sequential("nums.txt", "ref_sub.txt", &big_sub);
    printf("DONE SUB==============================\n");

    printf("STARTING MUL==============================\n");
    test_wrap_sequential("nums.txt", "ref_mul.txt", &big_mul);
    printf("DONE MUL==========================\n");

    printf("STARTING DIV==============================\n");
    test_wrap_div("nums_div.txt", "ref_div.txt", &big_div);
    printf("DONE DIV==============================\n");

    printf("STARTING CMP==============================\n");
    test_wrap_cmp("nums.txt", "ref_cmp.txt", &big_cmp);
    printf("DONE CMP==============================\n");

    printf("STARTING GCD==============================\n");
    test_wrap_sequential("nums.txt", "ref_gcd.txt", &big_gcd);
    printf("DONE GCD==============================\n");

    printf("STARTING INVMOD\n");
    test_wrap_sequential("nums.txt", "ref_modinv.txt", &big_inv_mod);
    printf("DONE INVMOD\n");

    // printf("STARTING MONT\n");
    // test_wrap_mont("nums_mont.txt", "ref_mont.txt", &big_exp_mod);
    // printf("DONE MONT\n");

    printf("ALL TESTS PASS!!! :)\n");
}

int main(){
    test();
    return 0;
}