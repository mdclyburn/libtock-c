#include <stdint.h>

void aes128_cbc_mac_64bytes(const uint8_t* message, const uint8_t* key, uint8_t* mac_out);
