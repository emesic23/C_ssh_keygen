#include "rsa.h"
#include "bigint.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void rsa_init(rsa_context *ctx) {
    big_init(&ctx->N);
    big_init(&ctx->E);
    big_init(&ctx->D);
    big_init(&ctx->P);
    big_init(&ctx->Q);
    big_init(&ctx->DP);
    big_init(&ctx->DQ);
    big_init(&ctx->QP);
    big_init(&ctx->RN);
    big_init(&ctx->RP);
    big_init(&ctx->RQ);
}

void rsa_free(rsa_context *ctx) {
    free(ctx->N.data);
    free(ctx->E.data);
    free(ctx->D.data);
    free(ctx->P.data);
    free(ctx->Q.data);
    free(ctx->DP.data);
    free(ctx->DQ.data);
    free(ctx->QP.data);
    free(ctx->RN.data);
    free(ctx->RP.data);
    free(ctx->RQ.data);
    free(ctx);
}

int rsa_gen_key(rsa_context *ctx, size_t nbits, big_uint exponent) {
    bigint* one = big_create_nonzero(1);
    big_set_nonzero(&ctx->E, exponent);
    bigint *upper_1 = malloc(sizeof(bigint));
    big_init(upper_1);
    big_uint *data;
    if ((nbits / 2 - 100) % 64 == 0) {
        data = malloc(((nbits / 2 - 100) / 64) * sizeof(big_uint));
        upper_1->num_limbs = ((nbits / 2 - 100) / 64);
        memset(data, 0, ((nbits / 2 - 100) / 64) * sizeof(big_uint));
        data[0] = 1 << 63;
    }
    else {
        data = malloc((((nbits / 2 - 100) / 64) + 1) * sizeof(big_uint));
        upper_1->num_limbs = ((nbits / 2 - 100) / 64) + 1;
        memset(data, 0, (((nbits / 2 - 100) / 64) + 1) * sizeof(big_uint));
        data[0] = 1 << ((nbits / 2 - 100) % 64)  - 1;
    }
    free(upper_1->data);
    upper_1->data = data;

    bigint *upper_2 = malloc(sizeof(bigint));
    big_init(upper_2);
    big_uint *data_u;
    if ((nbits / 2) % 64 == 0) {
        data_u = malloc(((nbits / 2) / 64) * sizeof(big_uint));
        upper_2->num_limbs = ((nbits / 2) / 64);
        memset(data, 0, ((nbits / 2) / 64) * sizeof(big_uint));
        data[0] = 1 << 63;
    }
    else {
        data_u = malloc((((nbits / 2) / 64) + 1) * sizeof(big_uint));
        upper_2->num_limbs = ((nbits / 2) / 64) + 1;
        memset(data, 0, (((nbits / 2) / 64) + 1) * sizeof(big_uint));
        data[0] = 1 << ((nbits / 2) % 64) - 1;
    }
    free(upper_2->data);
    upper_2->data = data_u;


    while(true){
        bigint* P = malloc(sizeof(bigint));
        big_init(P);
        while (!big_is_prime(P)) {
            big_gen_prime(P, nbits / 2);
        }
        bigint* Q = malloc(sizeof(bigint));
        big_init(Q);
        while (!big_is_prime(Q)) {
            big_gen_prime(Q, nbits / 2);
        }

        bigint* temp = malloc(sizeof(bigint));
        big_init(temp);
        big_sub(temp, P, Q);
        temp->signum = 1;

        if (big_cmp(temp, upper_1) <= 0) {
            continue;
        }

        bigint* DP = malloc(sizeof(bigint));
        big_init(DP);
        big_copy(DP, P);
        big_sub(DP, DP, one);

        bigint* DQ = malloc(sizeof(bigint));
        big_init(DQ);
        big_copy(DQ, Q);
        big_sub(DQ, DQ, one);

        bigint* DPDQ = malloc(sizeof(bigint));
        big_init(DPDQ);
        big_mul(DPDQ, DP, DQ);

        bigint* gcdeDPDQ = malloc(sizeof(bigint));
        big_init(gcdeDPDQ);
        big_gcd(gcdeDPDQ, exponent, DPDQ);
        if(big_cmp(gcdeDPDQ, one) != 0){
            continue;
        }

        //LCM(a, b) = (a x b) / GCD(a,b)
        bigint* lcm = malloc(sizeof(bigint));
        big_init(lcm);

        bigint* gcdDPDQ = malloc(sizeof(bigint));
        big_init(gcdDPDQ);
        big_gcd(gcdDPDQ, DP, DQ);

        bigint *tempr = big_create_nonzero(1);
        big_div(lcm, tempr, DPDQ, gcdDPDQ);
        big_free(tempr);

        bigint* D = malloc(sizeof(bigint));
        big_init(D);
        D = big_inv_mod(D, exponent, lcm);
        if (big_cmp(D, upper_2) <= 0) {
            continue;
        }

        bigint* N = malloc(sizeof(bigint));
        big_init(N);
        big_mul(N, P, Q);

        big_copy(&ctx->N, N);
        big_copy(&ctx->D, D);
        big_copy(&ctx->P, P);
        big_copy(&ctx->Q, Q);
        big_copy(&ctx->DP, DP);
        big_copy(&ctx->DP, DQ);
        big_free(lcm);
    }
}

int rsa_write_public_key(const rsa_context *ctx, FILE *file) {
    uint8_t* buff = malloc(ctx->len);
    fprintf(file, "%d", 7);
    fprintf(file, "%s", "ssh-rsa");
    fprintf(file, "%d", 8);
    big_write_binary(&ctx->E, buff, ctx->len);
    fwrite(buff, 1, 8, file);
    fprintf(file, "%lu", big_size(&ctx->N));
    big_write_binary(&ctx->N, buff, ctx->len);
    fwrite(buff, 1, ctx->len, file);
    return 0;
}
