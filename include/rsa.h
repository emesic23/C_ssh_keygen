/**
 * \file rsa.h
 *
 * \brief This file provides an API for the RSA public-key cryptosystem.
 *
 * The RSA public-key cryptosystem is defined in <em>Public-Key
 * Cryptography Standards (PKCS) #1 v1.5: RSA Encryption</em>
 * and <em>Public-Key Cryptography Standards (PKCS) #1 v2.1:
 * RSA Cryptography Specifications</em>.
 *
 */

#ifndef RSA_H
#define RSA_H

#include <stdio.h>

#include "bigint.h"

#define RSA_KEY_WRITE_ERROR 1

/**
 * \brief   The RSA context structure.
 */
typedef struct
{
    int ver;               /*!<  Always 0.*/
    size_t len;            /*!<  The size of \p N in Bytes. */

    bigint N;              /*!<  The public modulus. */
    bigint E;              /*!<  The public exponent. */

    bigint D;              /*!<  The private exponent. */
    bigint P;              /*!<  The first prime factor. */
    bigint Q;              /*!<  The second prime factor. */

    bigint DP;             /*!<  <code>D % (P - 1)</code>. */
    bigint DQ;             /*!<  <code>D % (Q - 1)</code>. */
    bigint QP;             /*!<  <code>(1 / Q) % P</code>. */

    bigint RN;             /*!<  cached <code>R^2 mod N</code>. */

    bigint RP;             /*!<  cached <code>R^2 mod P</code>. */
    bigint RQ;             /*!<  cached <code>R^2 mod Q</code>. */

}
rsa_context;

void rsa_init(rsa_context *ctx);
void rsa_free(rsa_context *ctx);

int rsa_gen_key(rsa_context *ctx, size_t nbits, big_uint exponent);

/**
 * \brief                  Write the private key to the given file
 *                         in the DER ASN.1 format
 * 
 * \return                 0 if successful, RSA_KEY_WRITE_ERROR otherwise.
 */
int rsa_write_private_key(const rsa_context *ctx, FILE *file);

/**
 * \brief                  Write the public key to the given file.
 *                         See notes for format.
 * 
 * \note                   Follow RFC4716 Section 3.4; which refers to
 *                         the "ssh-rsa" section of RFC4253 Section 6.6.
 * 
 * \return                 0 if successful, RSA_KEY_WRITE_ERROR otherwise.
 */
int rsa_write_public_key(const rsa_context *ctx, FILE *file);

#endif /* RSA_H */
