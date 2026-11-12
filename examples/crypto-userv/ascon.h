#pragma once

#include <stdint.h>

#define ASCON_KEY_SIZE 16
#define ASCON_NONCE_SIZE 16
#define ASCON_TAG_SIZE 16

/** Encrypt the plaintext buffer.

Encrypts the provided plaintext buffer with the provided key,
placing its ciphertext in the output buffer.

### Arguments
* `key` - The key to use for encryption.
* `nonce` - The nonce to use for encryption.
* `plaintext` - The plaintext buffer to encrypt.
* `plaintext_len` - The length of the plaintext buffer.
* `aad` - The associated data to use for encryption.
* `aad_len` - The length of the associated data.
* `ciphertext` - The output buffer for the ciphertext.
* `ciphertext_len` - The length of the output buffer.
*/
int32_t ascon_encrypt(const uint8_t *key, const uint8_t *nonce,
                      const uint8_t *plaintext, uint32_t plaintext_len,
                      const uint8_t *aad, uint32_t aad_len, uint8_t *ciphertext,
                      uint32_t *ciphertext_len);

int32_t ascon_decrypt(const uint8_t *key, const uint8_t *nonce,
                      const uint8_t *ciphertext, uint32_t ciphertext_len,
                      const uint8_t *aad, uint32_t aad_len, uint8_t *plaintext,
                      uint32_t *plaintext_len);