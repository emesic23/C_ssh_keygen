/**
 * \file base64.h
 *
 * \brief RFC 1521 base64 encoding/decoding
 */
#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

#define ERR_BASE64_BUFFER_TOO_SMALL     -0x002A /**< Output buffer too small. */
#define ERR_BASE64_INVALID_CHARACTER    -0x002C /**< Invalid character in input. */

/**
 * \brief          Compute how large a buffer will be needed to base64-encode
 * 
 * \param buf_size Size of the buffer to be encoded
 * 
 * \return         Necessary length for the dest buffer
 */
size_t base64_size(size_t buf_size);

/**
 * \brief          Encode a buffer into base64 format
 *
 * \param dst      destination buffer
 * \param dlen     size of the destination buffer
 * \param olen     number of bytes written
 * \param src      source buffer
 * \param slen     amount of data to be encoded
 *
 * \return         0 if successful, or ERR_BASE64_BUFFER_TOO_SMALL.
 *                 *olen is always updated to reflect the amount
 *                 of data that has (or would have) been written.
 *                 If that length cannot be represented, then no data is
 *                 written to the buffer and *olen is set to the maximum
 *                 length representable as a size_t.
 *
 * \note           Call this function with dlen = 0 to obtain the
 *                 required buffer size in *olen
 */
int base64_encode(unsigned char *dst, size_t dlen, size_t *olen,
                  const unsigned char *src, size_t slen);

/**
 * \brief          Decode a base64-formatted buffer
 *
 * \param dst      destination buffer (can be NULL for checking size)
 * \param dlen     size of the destination buffer
 * \param olen     number of bytes written
 * \param src      source buffer
 * \param slen     amount of data to be decoded
 *
 * \return         0 if successful, ERR_BASE64_BUFFER_TOO_SMALL, or
 *                 ERR_BASE64_INVALID_CHARACTER if the input data is
 *                 not correct. *olen is always updated to reflect the amount
 *                 of data that has (or would have) been written.
 *
 * \note           Call this function with *dst = NULL or dlen = 0 to obtain
 *                 the required buffer size in *olen
 */
int base64_decode(unsigned char *dst, size_t dlen, size_t *olen,
                  const unsigned char *src, size_t slen);

#endif /* BASE64_H */
