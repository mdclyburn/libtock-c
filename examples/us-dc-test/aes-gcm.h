#include <stdint.h>

int aes128_gcm_decrypt_64bytes(uint8_t *data, const uint8_t *tag, const uint8_t *key, const uint8_t *iv);
