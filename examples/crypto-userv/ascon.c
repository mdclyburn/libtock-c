#include "ascon.h"

#include <string.h>

/*
 * Ascon-128 Implementation (Ascon v1.2 / NIST SP 800-232)
 *
 * Parameters for Ascon-128:
 * - Key size: 128 bits (16 bytes)
 * - Nonce size: 128 bits (16 bytes)
 * - Tag size: 128 bits (16 bytes)
 * - Rate: 64 bits (8 bytes per block)
 * - a = 12 rounds (initialization and finalization)
 * - b = 6 rounds (associated data and plaintext/ciphertext processing)
 *
 * Optimized for low memory and small code footprint on 32-bit Cortex-M4 (nRF52840).
 * Uses strictly stack/register variables, zero dynamic memory allocation.
 */

typedef struct {
  uint64_t x[5];
} ascon_state_t;

/* Big-endian load and store 64-bit integer */
static inline uint64_t ascon_load64(const uint8_t* bytes) {
  return (((uint64_t)bytes[0]) << 56) |
         (((uint64_t)bytes[1]) << 48) |
         (((uint64_t)bytes[2]) << 40) |
         (((uint64_t)bytes[3]) << 32) |
         (((uint64_t)bytes[4]) << 24) |
         (((uint64_t)bytes[5]) << 16) |
         (((uint64_t)bytes[6]) <<  8) |
         (((uint64_t)bytes[7]) <<  0);
}

static inline void ascon_store64(uint8_t* bytes, uint64_t v) {
  bytes[0] = (uint8_t)(v >> 56);
  bytes[1] = (uint8_t)(v >> 48);
  bytes[2] = (uint8_t)(v >> 40);
  bytes[3] = (uint8_t)(v >> 32);
  bytes[4] = (uint8_t)(v >> 24);
  bytes[5] = (uint8_t)(v >> 16);
  bytes[6] = (uint8_t)(v >>  8);
  bytes[7] = (uint8_t)(v >>  0);
}

/* Load up to 7 bytes into a 64-bit word with 1-padding (0x80) */
static inline uint64_t ascon_load_partial(const uint8_t* bytes, uint32_t len) {
  uint64_t v = 0;
  for (uint32_t i = 0; i < len; ++i) {
    v |= ((uint64_t)bytes[i]) << (56 - 8 * i);
  }
  v |= ((uint64_t)0x80) << (56 - 8 * len);
  return v;
}

/* Store up to 7 bytes from a 64-bit word */
static inline void ascon_store_partial(uint8_t* bytes, uint64_t v, uint32_t len) {
  for (uint32_t i = 0; i < len; ++i) {
    bytes[i] = (uint8_t)(v >> (56 - 8 * i));
  }
}

static inline uint64_t ascon_rotr(uint64_t v, unsigned int n) {
  return (v >> n) | (v << (64 - n));
}

/* Round constants for round r = 12 - rounds .. 11 */
static const uint8_t ASCON_RC[12] = {
  0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5,
  0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b
};

/* Ascon permutation round */
static void ascon_permutation(ascon_state_t* s, int rounds) {
  int start_round = 12 - rounds;

  for (int r = start_round; r < 12; ++r) {
    /* 1. Addition of round constant */
    s->x[2] ^= (uint64_t)ASCON_RC[r];

    /* 2. Substitution layer (5-bit S-box across 64-bit words) */
    s->x[0] ^= s->x[4];
    s->x[4] ^= s->x[3];
    s->x[2] ^= s->x[1];

    uint64_t t0 = s->x[0] ^ (~s->x[1] & s->x[2]);
    uint64_t t1 = s->x[1] ^ (~s->x[2] & s->x[3]);
    uint64_t t2 = s->x[2] ^ (~s->x[3] & s->x[4]);
    uint64_t t3 = s->x[3] ^ (~s->x[4] & s->x[0]);
    uint64_t t4 = s->x[4] ^ (~s->x[0] & s->x[1]);

    t1 ^= t0;
    t0 ^= t4;
    t3 ^= t2;
    t2 = ~t2;

    /* 3. Linear diffusion layer */
    s->x[0] = t0 ^ ascon_rotr(t0, 19) ^ ascon_rotr(t0, 28);
    s->x[1] = t1 ^ ascon_rotr(t1, 61) ^ ascon_rotr(t1, 39);
    s->x[2] = t2 ^ ascon_rotr(t2, 1)  ^ ascon_rotr(t2, 6);
    s->x[3] = t3 ^ ascon_rotr(t3, 10) ^ ascon_rotr(t3, 17);
    s->x[4] = t4 ^ ascon_rotr(t4, 7)  ^ ascon_rotr(t4, 41);
  }
}

int32_t ascon_encrypt(const uint8_t* key,
                      const uint8_t* nonce,
                      const uint8_t* plaintext,
                      uint32_t plaintext_len,
                      const uint8_t* aad,
                      uint32_t aad_len,
                      uint8_t* ciphertext,
                      uint32_t* ciphertext_len) {
  if (!key || !nonce || (!ciphertext && plaintext_len > 0)) {
    return -1;
  }

  ascon_state_t state;
  const uint64_t k0 = ascon_load64(key);
  const uint64_t k1 = ascon_load64(key + 8);
  const uint64_t n0 = ascon_load64(nonce);
  const uint64_t n1 = ascon_load64(nonce + 8);

  /* Initialization: IV || K || N
   * For Ascon-128: IV = 0x80400c0600000000
   * (k=128, r=64, a=12, b=6)
   */
  state.x[0] = 0x80400c0600000000ULL;
  state.x[1] = k0;
  state.x[2] = k1;
  state.x[3] = n0;
  state.x[4] = n1;

  ascon_permutation(&state, 12);
  state.x[3] ^= k0;
  state.x[4] ^= k1;

  /* Associated Data processing */
  if (aad && aad_len > 0) {
    uint32_t remaining_aad = aad_len;
    const uint8_t* aad_ptr = aad;

    while (remaining_aad >= 8) {
      state.x[0] ^= ascon_load64(aad_ptr);
      ascon_permutation(&state, 6);
      aad_ptr += 8;
      remaining_aad -= 8;
    }

    state.x[0] ^= ascon_load_partial(aad_ptr, remaining_aad);
    ascon_permutation(&state, 6);
  }

  /* Domain separation */
  state.x[4] ^= 1;

  /* Plaintext processing */
  uint32_t remaining_pt = plaintext_len;
  const uint8_t* pt_ptr = plaintext;
  uint8_t* ct_ptr = ciphertext;

  while (remaining_pt >= 8) {
    state.x[0] ^= ascon_load64(pt_ptr);
    ascon_store64(ct_ptr, state.x[0]);
    ascon_permutation(&state, 6);
    pt_ptr += 8;
    ct_ptr += 8;
    remaining_pt -= 8;
  }

  if (remaining_pt > 0) {
    state.x[0] ^= ascon_load_partial(pt_ptr, remaining_pt);
    ascon_store_partial(ct_ptr, state.x[0], remaining_pt);
    ct_ptr += remaining_pt;
  } else {
    /* 1-padding when plaintext is multiple of rate (including empty plaintext) */
    state.x[0] ^= 0x8000000000000000ULL;
  }

  /* Finalization */
  state.x[1] ^= k0;
  state.x[2] ^= k1;
  ascon_permutation(&state, 12);
  state.x[3] ^= k0;
  state.x[4] ^= k1;

  /* Write 16-byte authentication tag */
  ascon_store64(ct_ptr, state.x[3]);
  ascon_store64(ct_ptr + 8, state.x[4]);

  if (ciphertext_len) {
    *ciphertext_len = plaintext_len + ASCON_TAG_SIZE;
  }

  return 0;
}

int32_t ascon_decrypt(const uint8_t* key,
                      const uint8_t* nonce,
                      const uint8_t* ciphertext,
                      uint32_t ciphertext_len,
                      const uint8_t* aad,
                      uint32_t aad_len,
                      uint8_t* plaintext,
                      uint32_t* plaintext_len) {
  if (!key || !nonce || !ciphertext || ciphertext_len < ASCON_TAG_SIZE) {
    return -1;
  }

  uint32_t pt_len = ciphertext_len - ASCON_TAG_SIZE;
  if (!plaintext && pt_len > 0) {
    return -1;
  }

  ascon_state_t state;
  const uint64_t k0 = ascon_load64(key);
  const uint64_t k1 = ascon_load64(key + 8);
  const uint64_t n0 = ascon_load64(nonce);
  const uint64_t n1 = ascon_load64(nonce + 8);

  /* Initialization */
  state.x[0] = 0x80400c0600000000ULL;
  state.x[1] = k0;
  state.x[2] = k1;
  state.x[3] = n0;
  state.x[4] = n1;

  ascon_permutation(&state, 12);
  state.x[3] ^= k0;
  state.x[4] ^= k1;

  /* Associated Data processing */
  if (aad && aad_len > 0) {
    uint32_t remaining_aad = aad_len;
    const uint8_t* aad_ptr = aad;

    while (remaining_aad >= 8) {
      state.x[0] ^= ascon_load64(aad_ptr);
      ascon_permutation(&state, 6);
      aad_ptr += 8;
      remaining_aad -= 8;
    }

    state.x[0] ^= ascon_load_partial(aad_ptr, remaining_aad);
    ascon_permutation(&state, 6);
  }

  /* Domain separation */
  state.x[4] ^= 1;

  /* Ciphertext processing */
  uint32_t remaining_ct = pt_len;
  const uint8_t* ct_ptr = ciphertext;
  uint8_t* pt_ptr = plaintext;

  while (remaining_ct >= 8) {
    uint64_t c = ascon_load64(ct_ptr);
    ascon_store64(pt_ptr, state.x[0] ^ c);
    state.x[0] = c;
    ascon_permutation(&state, 6);
    ct_ptr += 8;
    pt_ptr += 8;
    remaining_ct -= 8;
  }

  if (remaining_ct > 0) {
    uint64_t c_partial = 0;
    for (uint32_t i = 0; i < remaining_ct; ++i) {
      c_partial |= ((uint64_t)ct_ptr[i]) << (56 - 8 * i);
    }
    uint64_t p_partial = state.x[0] ^ c_partial;
    ascon_store_partial(pt_ptr, p_partial, remaining_ct);

    /* Update state with ciphertext and 1-padding on plaintext */
    uint64_t pad = ((uint64_t)0x80) << (56 - 8 * remaining_ct);
    uint64_t mask = 0xFFFFFFFFFFFFFFFFULL << (64 - 8 * remaining_ct);
    state.x[0] = (c_partial & mask) | ((p_partial ^ pad) & ~mask);
  } else {
    /* 1-padding when ciphertext is multiple of rate */
    state.x[0] ^= 0x8000000000000000ULL;
  }

  /* Finalization */
  state.x[1] ^= k0;
  state.x[2] ^= k1;
  ascon_permutation(&state, 12);
  state.x[3] ^= k0;
  state.x[4] ^= k1;

  /* Constant-time tag verification */
  const uint8_t* expected_tag = ciphertext + pt_len;
  uint8_t computed_tag[ASCON_TAG_SIZE];
  ascon_store64(computed_tag, state.x[3]);
  ascon_store64(computed_tag + 8, state.x[4]);

  uint8_t diff = 0;
  for (uint32_t i = 0; i < ASCON_TAG_SIZE; ++i) {
    diff |= computed_tag[i] ^ expected_tag[i];
  }

  if (diff != 0) {
    /* Authentication failed: zero plaintext buffer to prevent unauthenticated release */
    if (plaintext && pt_len > 0) {
      memset(plaintext, 0, pt_len);
    }
    return -1;
  }

  if (plaintext_len) {
    *plaintext_len = pt_len;
  }

  return 0;
}