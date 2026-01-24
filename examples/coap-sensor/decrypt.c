#include <stdint.h>
#include <string.h>

int decrypt_packet_37b(const uint8_t *key, const uint8_t *iv, const uint8_t *buffer, uint8_t *output);

/* --- AES-128 Constants & Lookup Tables --- */

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

// Rcon is needed for Key Expansion
static const uint8_t rcon[11] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

/* --- AES Core Functions --- */

static void aes_key_expansion(const uint8_t *key, uint8_t *round_keys) {
    int i, j;
    uint8_t temp[4];

    // First round key is the key itself
    for (i = 0; i < 16; i++) round_keys[i] = key[i];

    for (i = 16; i < 176; i += 4) {
        // Copy previous word
        for (j = 0; j < 4; j++) temp[j] = round_keys[i - 4 + j];

        if (i % 16 == 0) {
            uint8_t k = temp[0];
            temp[0] = sbox[temp[1]] ^ rcon[i / 16];
            temp[1] = sbox[temp[2]];
            temp[2] = sbox[temp[3]];
            temp[3] = sbox[k];
        }

        for (j = 0; j < 4; j++) round_keys[i + j] = round_keys[i - 16 + j] ^ temp[j];
    }
}

static void aes_encrypt_block(const uint8_t *in, uint8_t *out, const uint8_t *round_keys) {
    uint8_t state[4][4];
    int r, i, j, c;

    // Copy input to state (column-major)
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            state[j][i] = in[i * 4 + j];

    // AddRoundKey 0
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            state[j][i] ^= round_keys[i * 4 + j];

    for (r = 1; r < 10; r++) {
        // SubBytes
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                state[i][j] = sbox[state[i][j]];

        // ShiftRows
        uint8_t temp;
        // Row 1 (shift 1)
        temp = state[1][0]; state[1][0] = state[1][1]; state[1][1] = state[1][2]; state[1][2] = state[1][3]; state[1][3] = temp;
        // Row 2 (shift 2)
        temp = state[2][0]; state[2][0] = state[2][2]; state[2][2] = temp;
        temp = state[2][1]; state[2][1] = state[2][3]; state[2][3] = temp;
        // Row 3 (shift 3)
        temp = state[3][0]; state[3][0] = state[3][3]; state[3][3] = state[3][2]; state[3][2] = state[3][1]; state[3][1] = temp;

        // MixColumns
        for (c = 0; c < 4; c++) {
            uint8_t col[4];
            for(i=0; i<4; i++) col[i] = state[i][c];

            // Galois Multiplications (x2 is shift left, conditional XOR 0x1B)
            #define X2(x) (((x) << 1) ^ (((x) & 0x80) ? 0x1b : 0x00))
            #define X3(x) (X2(x) ^ (x))

            state[0][c] = X2(col[0]) ^ X3(col[1]) ^ col[2] ^ col[3];
            state[1][c] = col[0] ^ X2(col[1]) ^ X3(col[2]) ^ col[3];
            state[2][c] = col[0] ^ col[1] ^ X2(col[2]) ^ X3(col[3]);
            state[3][c] = X3(col[0]) ^ col[1] ^ col[2] ^ X2(col[3]);
        }

        // AddRoundKey
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                state[j][i] ^= round_keys[r * 16 + i * 4 + j];
    }

    // Final Round
    // SubBytes
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            state[i][j] = sbox[state[i][j]];

    // ShiftRows
    uint8_t t;
    t = state[1][0]; state[1][0] = state[1][1]; state[1][1] = state[1][2]; state[1][2] = state[1][3]; state[1][3] = t;
    t = state[2][0]; state[2][0] = state[2][2]; state[2][2] = t;
    t = state[2][1]; state[2][1] = state[2][3]; state[2][3] = t;
    t = state[3][0]; state[3][0] = state[3][3]; state[3][3] = state[3][2]; state[3][2] = state[3][1]; state[3][1] = t;

    // AddRoundKey (Final)
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            state[j][i] ^= round_keys[160 + i * 4 + j];

    // Output
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            out[i * 4 + j] = state[j][i];
}

/* --- GCM Core Functions --- */

// Multiplication in GF(2^128)
static void gf_mult(const uint8_t *x, const uint8_t *y, uint8_t *z) {
    uint8_t V[16];
    uint8_t Z[16];

    memset(Z, 0, 16);
    memcpy(V, y, 16);

    for (int i = 0; i < 128; i++) {
        // If bit i of x is 1, Z = Z ^ V
        int byte_idx = i / 8;
        int bit_idx = 7 - (i % 8);
        if ((x[byte_idx] >> bit_idx) & 1) {
            for (int k = 0; k < 16; k++) Z[k] ^= V[k];
        }

        // V = V >> 1 (if LSB is 1, XOR with R = 0xE1...)
        // V is treated as a 128-bit block
        uint8_t lsb = V[15] & 0x01;

        // Right shift the whole 16-byte array
        for (int k = 15; k > 0; k--) {
            V[k] = (V[k] >> 1) | ((V[k - 1] & 1) << 7);
        }
        V[0] >>= 1;

        if (lsb) {
            V[0] ^= 0xE1; // R constant for GF(2^128)
        }
    }
    memcpy(z, Z, 16);
}

// Increment 32-bit counter (big endian) inside 16-byte block
static void gcm_inc32(uint8_t *block) {
    for (int i = 15; i >= 12; i--) {
        block[i]++;
        if (block[i] != 0) break;
    }
}

// XOR two blocks
static void xor_block(uint8_t *dst, const uint8_t *src, int len) {
    for (int i = 0; i < len; i++) dst[i] ^= src[i];
}

/* --- Main Decryption Function --- */

/**
 * Decrypts a strict 37-byte buffer using AES128-GCM.
 * Assumes buffer format: [ Ciphertext (21 bytes) | Tag (16 bytes) ]
 * * @param key       16-byte AES key
 * @param iv        12-byte Initialization Vector
 * @param buffer    37-byte input buffer containing ciphertext and tag
 * @param output    Output buffer (must be at least 21 bytes)
 * @return          0 if success (tag verified), -1 if authentication failed
 */
uint8_t ek[176];            // Expanded key
int decrypt_packet_37b(const uint8_t *key, const uint8_t *iv, const uint8_t *buffer, uint8_t *output) {
    uint8_t h[16];              // Hash subkey
    uint8_t j0[16];             // Counter block
    uint8_t s_tag[16];          // Calculator tag
    uint8_t ghash_state[16];    // GHASH accumulator
    uint8_t enc_ctr[16];        // Encrypted counter
    uint8_t len_block[16];      // Length block for GHASH

    const int ct_len = 21;      // 37 total - 16 tag
    const uint8_t *ct = buffer;
    const uint8_t *tag = buffer + ct_len;

    // 1. Prepare Key
    aes_key_expansion(key, ek);

    // 2. Compute H = E(K, 0)
    memset(h, 0, 16);
    aes_encrypt_block(h, h, ek);

    // 3. Prepare J0 (IV || 0...01) - Standard 96-bit IV assumption
    memcpy(j0, iv, 12);
    j0[12] = 0; j0[13] = 0; j0[14] = 0; j0[15] = 1;

    // 4. GCTR Decryption (Ciphertext -> Plaintext)
    // First counter for data is J0 + 1
    uint8_t ctr[16];
    memcpy(ctr, j0, 16);
    gcm_inc32(ctr);

    // Block 1 (First 16 bytes of CT)
    aes_encrypt_block(ctr, enc_ctr, ek);
    xor_block(enc_ctr, ct, 16);
    memcpy(output, enc_ctr, 16); // Store first 16 bytes of PT

    // Block 2 (Remaining 5 bytes)
    gcm_inc32(ctr);
    aes_encrypt_block(ctr, enc_ctr, ek);
    xor_block(enc_ctr, ct + 16, 5);
    memcpy(output + 16, enc_ctr, 5); // Store remaining 5 bytes of PT

    // 5. GHASH Calculation (AAD is empty)
    memset(ghash_state, 0, 16);

    // Process Ciphertext Block 1 (16 bytes)
    xor_block(ghash_state, ct, 16);
    gf_mult(ghash_state, h, ghash_state);

    // Process Ciphertext Block 2 (5 bytes, zero padded)
    uint8_t partial[16];
    memset(partial, 0, 16);
    memcpy(partial, ct + 16, 5);
    xor_block(ghash_state, partial, 16);
    gf_mult(ghash_state, h, ghash_state);

    // Process Length Block (0 bits AAD, 21*8 bits CT)
    memset(len_block, 0, 16);
    // Write 168 (21 * 8) to last 4 bytes (big endian)
    uint32_t bits = ct_len * 8;
    len_block[12] = (bits >> 24) & 0xFF;
    len_block[13] = (bits >> 16) & 0xFF;
    len_block[14] = (bits >> 8)  & 0xFF;
    len_block[15] = (bits)       & 0xFF;

    xor_block(ghash_state, len_block, 16);
    gf_mult(ghash_state, h, ghash_state);

    // 6. Final Tag Calculation (GCTR(K, J0, GHASH))
    aes_encrypt_block(j0, s_tag, ek);
    xor_block(s_tag, ghash_state, 16);

    // 7. Verify Tag (Constant time comparison is better, but this is simple logic)
    int diff = 0;
    for (int i = 0; i < 16; i++) diff |= (s_tag[i] ^ tag[i]);

    if (diff != 0) {
        // Zero out output on failure
        memset(output, 0, ct_len);
        return -1;
    }

    return 0;
}
