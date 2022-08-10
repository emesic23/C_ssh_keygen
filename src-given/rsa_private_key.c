#include "rsa.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "base64.h"

#define STRLEN(s) (sizeof(s) / sizeof(*s) - 1)

#define BIG_VERSION BIG_ZERO

#define SEQUENCE_TAG 0x30
#define INTEGER_TAG 0x02
#define PRIVATE_KEY_HEADER "-----BEGIN RSA PRIVATE KEY-----\n"
#define PRIVATE_KEY_FOOTER "\n-----END RSA PRIVATE KEY-----\n"
#define LINE_LENGTH 64

void write_asn_length(size_t length, uint8_t *buffer, size_t *index) {
    if (length < 0x80) {
        if (buffer != NULL) buffer[*index] = length;
        (*index)++;
    }
    else {
        bigint big_length;
        big_init(&big_length);
        int result = big_set_nonzero(&big_length, length);
        assert(result == 0);
        size_t length_length = big_size(&big_length);
        if (buffer != NULL) buffer[*index] = 0x80 | length_length;
        (*index)++;
        if (buffer != NULL) {
            result = big_write_binary(&big_length, &buffer[*index], length_length);
            assert(result == 0);
        }
        *index += length_length;
        big_free(&big_length);
    }
}

void write_asn_integer(const bigint *x, uint8_t *buffer, size_t *index) {
    size_t length = big_size(x);
    if (buffer != NULL) buffer[*index] = INTEGER_TAG;
    (*index)++;
    write_asn_length(length, buffer, index);
    if (buffer != NULL) {
        int result = big_write_binary(x, &buffer[*index], length);
        assert(result == 0);
    }
    *index += length;
}

int rsa_write_private_key(const rsa_context *rsa, FILE *file) {
    bigint version = BIG_VERSION;
    size_t sequence_length = 0;
    write_asn_integer(&version, NULL, &sequence_length);
    write_asn_integer(&rsa->N, NULL, &sequence_length);
    write_asn_integer(&rsa->E, NULL, &sequence_length);
    write_asn_integer(&rsa->D, NULL, &sequence_length);
    write_asn_integer(&rsa->P, NULL, &sequence_length);
    write_asn_integer(&rsa->Q, NULL, &sequence_length);
    write_asn_integer(&rsa->DP, NULL, &sequence_length);
    write_asn_integer(&rsa->DQ, NULL, &sequence_length);
    write_asn_integer(&rsa->QP, NULL, &sequence_length);
    size_t contents_length = 1;
    write_asn_length(sequence_length, NULL, &contents_length);
    uint8_t contents[contents_length + sequence_length];
    size_t contents_index = 0;
    contents[contents_index++] = SEQUENCE_TAG;
    write_asn_length(sequence_length, contents, &contents_index);
    write_asn_integer(&version, contents, &contents_index);
    write_asn_integer(&rsa->N, contents, &contents_index);
    write_asn_integer(&rsa->E, contents, &contents_index);
    write_asn_integer(&rsa->D, contents, &contents_index);
    write_asn_integer(&rsa->P, contents, &contents_index);
    write_asn_integer(&rsa->Q, contents, &contents_index);
    write_asn_integer(&rsa->DP, contents, &contents_index);
    write_asn_integer(&rsa->DQ, contents, &contents_index);
    write_asn_integer(&rsa->QP, contents, &contents_index);
    big_free(&version);

    uint8_t base64_encoded[base64_size(sizeof(contents))];
    size_t olen;
    int result = base64_encode(
        base64_encoded, sizeof(base64_encoded),
        &olen,
        contents, sizeof(contents)
    );
    assert(result == 0);
    size_t base64_length = sizeof(base64_encoded) - 1;
    if (fputs(PRIVATE_KEY_HEADER, file) < 0)
        return RSA_KEY_WRITE_ERROR;

    size_t full_base64_lines = base64_length / LINE_LENGTH;
    uint8_t *remaining_base64 = base64_encoded;
    for (size_t i = 0; i < full_base64_lines; i++) {
        if (
            fwrite(remaining_base64, 1, LINE_LENGTH, file) < LINE_LENGTH ||
            fputc('\n', file) < 0
        ) return RSA_KEY_WRITE_ERROR;

        remaining_base64 += LINE_LENGTH;
    }
    size_t remaining_length = base64_length % LINE_LENGTH;
    return
        !(fwrite(remaining_base64, 1, remaining_length, file) == remaining_length &&
        fputs(PRIVATE_KEY_FOOTER, file) >= 0);
}
