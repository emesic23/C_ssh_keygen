#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "rsa.h"

#define STRLEN(s) (sizeof(s) / sizeof(*s) - 1)

#define BITS 2048
#define EXPONENT 65537

#define PUB_SUFFIX ".pub"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s RSA_FILE\n", argv[0]);
        return 1;
    }

    rsa_context rsa;
    rsa_init(&rsa);
    int result = rsa_gen_key(&rsa, BITS, EXPONENT);
    if (result != 0) {
        fprintf(stderr, "Key generation failed with error code: %d\n", result);
        rsa_free(&rsa);
        return 2;
    }

    char *private_filename = argv[1];
    FILE *private_file = fopen(private_filename, "w");
    if (private_file == NULL || rsa_write_private_key(&rsa, private_file)) {
        perror("Failed to write private key");
        return 2;
    }
    fclose(private_file);
    if (chmod(private_filename, S_IRUSR | S_IWUSR | S_IXUSR)) {
        perror("Failed to set private key permissions");
        return 2;
    }

    char public_filename[strlen(private_filename) + STRLEN(PUB_SUFFIX) + 1];
    strcpy(public_filename, private_filename);
    strcat(public_filename, PUB_SUFFIX);
    FILE *public_file = fopen(public_filename, "w");
    if (public_file == NULL || rsa_write_public_key(&rsa, public_file)) {
        perror("Failed to write public key");
        return 2;
    }
    fclose(public_file);

    rsa_free(&rsa);
}
