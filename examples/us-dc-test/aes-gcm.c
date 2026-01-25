#include <stdint.h>
#include <string.h>

// --- AES-128 Forward Core ---

static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

#define xtime(x) ((uint8_t)((x << 1) ^ (((x >> 7) & 1) * 0x1b)))

static void aes_encrypt_block(uint8_t *state, const uint8_t *round_keys) {
    uint8_t t, tm, tmp;
    for (int i = 0; i < 16; i++) state[i] ^= round_keys[i];

    for (int round = 1; round <= 10; round++) {
        for (int i = 0; i < 16; i++) state[i] = sbox[state[i]];

        t = state[1]; state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = t;
        t = state[2]; state[2] = state[10]; state[10] = t;
        t = state[6]; state[6] = state[14]; state[14] = t;
        t = state[15]; state[15] = state[11]; state[11] = state[7]; state[7] = state[3]; state[3] = t;

        if (round < 10) {
            for (int i = 0; i < 16; i += 4) {
                t = state[i];
                tmp = state[i] ^ state[i+1] ^ state[i+2] ^ state[i+3];
                tm = state[i] ^ state[i+1];   state[i]   ^= xtime(tm) ^ tmp;
                tm = state[i+1] ^ state[i+2]; state[i+1] ^= xtime(tm) ^ tmp;
                tm = state[i+2] ^ state[i+3]; state[i+2] ^= xtime(tm) ^ tmp;
                tm = state[i+3] ^ t;          state[i+3] ^= xtime(tm) ^ tmp;
            }
        }
        for (int i = 0; i < 16; i++) state[i] ^= round_keys[round * 16 + i];
    }
}

static void aes_key_expansion(const uint8_t *key, uint8_t *rkeys) {
    memcpy(rkeys, key, 16);
    uint8_t rcon = 1;
    for (int i = 16; i < 176; i += 4) {
        uint8_t temp[4];
        memcpy(temp, &rkeys[i - 4], 4);
        if (i % 16 == 0) {
            uint8_t k = temp[0]; temp[0] = sbox[temp[1]]; temp[1] = sbox[temp[2]];
            temp[2] = sbox[temp[3]]; temp[3] = sbox[k];
            temp[0] ^= rcon;
            rcon = xtime(rcon);
        }
        for (int j = 0; j < 4; j++) rkeys[i + j] = rkeys[i + j - 16] ^ temp[j];
    }
}

// --- GCM GHASH ($GF(2^{128})$) with 4-bit Tables ---

static void gcm_shift_right(uint8_t *v) {
    uint32_t b;
    for (int i = 15; i >= 0; i--) {
        b = v[i] & 1;
        v[i] >>= 1;
        if (i > 0 && (v[i-1] & 1)) v[i] |= 0x80;
    }
}

static void gcm_gf_mult(uint8_t *x, const uint8_t *h_table) {
    uint8_t z[16] = {0};
    for (int i = 0; i < 32; i++) {
        uint8_t nibble = (i % 2 == 0) ? (x[i/2] >> 4) : (x[i/2] & 0x0F);
        const uint8_t *h_ptr = &h_table[nibble * 16];
        for (int j = 0; j < 16; j++) z[j] ^= h_ptr[j];

        // This is a simplified "shift and reduction" for the table logic
        // For performance, pre-calculating the 16 entries of H * nibble is key.
    }
    memcpy(x, z, 16);
}

// Simplified multiplication for 64-byte bench (standard bit-by-bit for portability)
static void gcm_mult_simple(uint8_t *x, const uint8_t *y) {
    uint8_t z[16] = {0};
    uint8_t v[16];
    memcpy(v, y, 16);
    for (int i = 0; i < 128; i++) {
        if ((x[i / 8] >> (7 - (i % 8))) & 1) {
            for (int j = 0; j < 16; j++) z[j] ^= v[j];
        }
        uint8_t lsb = v[15] & 1;
        for (int j = 15; j > 0; j--) v[j] = (v[j] >> 1) | (v[j - 1] << 7);
        v[0] >>= 1;
        if (lsb) v[0] ^= 0xe1;
    }
    memcpy(x, z, 16);
}

// --- GCM Main ---

/**
 * Perform AES-128-GCM Decryption.
 * @param data Ciphertext input (will be decrypted in-place).
 * @param tag  The 16-byte authentication tag to verify.
 * @param key  16-byte key.
 * @param iv   12-byte IV/Nonce.
 * @return 0 if Tag is valid, -1 if invalid.
 */
int aes128_gcm_decrypt_64bytes(uint8_t *data, const uint8_t *tag, const uint8_t *key, const uint8_t *iv) {
    uint8_t rkeys[176];
    uint8_t h[16] = {0};
    uint8_t s[16] = {0};
    uint8_t j0[16] = {0};
    uint8_t counter[16];
    uint8_t keystream[16];

    aes_key_expansion(key, rkeys);

    // 1. Generate Hash Key H = E(K, 0)
    aes_encrypt_block(h, rkeys);

    // 2. Prepare J0 (Counter 0)
    memcpy(j0, iv, 12);
    j0[15] = 1;

    // 3. GHASH AD (assuming 0 for this bench)
    // 4. Decrypt and GHASH Ciphertext
    /* memcpy(counter, j0); */
    for (int i = 0; i < 4; i++) {
        // Increment Counter (32-bit)
        for (int j = 15; j >= 12; j--) if (++counter[j]) break;

        memcpy(keystream, counter, 16);
        aes_encrypt_block(keystream, rkeys);

        uint8_t *block = &data[i * 16];
        // GHASH the ciphertext *before* XORing to decrypt
        for (int j = 0; j < 16; j++) s[j] ^= block[j];
        gcm_mult_simple(s, h);

        // Decrypt
        for (int j = 0; j < 16; j++) block[j] ^= keystream[j];
    }

    // 5. Finalize Tag
    uint8_t len_block[16] = {0};
    // Length of AD (0) and Length of Data (64 bytes * 8 bits = 512 bits = 0x0200)
    len_block[14] = 0x02; len_block[15] = 0x00;
    for (int j = 0; j < 16; j++) s[j] ^= len_block[j];
    gcm_mult_simple(s, h);

    uint8_t t_mask[16];
    memcpy(t_mask, j0, 16);
    aes_encrypt_block(t_mask, rkeys);
    for (int j = 0; j < 16; j++) s[j] ^= t_mask[j];

    // Verify Tag
    return memcmp(s, tag, 16) == 0 ? 0 : -1;
}
