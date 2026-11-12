#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libtock/services/userv.h>
#include <libtock/tock.h>

#include "ascon.h"

#define ARG_BUFFER_CRYPT_DATA_LEN ((uint32_t)82)
#define ARG_BUFFER_PLAINTEXT_LEN (ARG_BUFFER_CRYPT_DATA_LEN)
#define ARG_BUFFER_CIPHERTEXT_LEN (ARG_BUFFER_PLAINTEXT_LEN)
#define ARG_BUFFER_KEY_LEN ((uint32_t)18)
#define ARG_BUFFER_NONCE_LEN ((uint32_t)18)
#define ARG_BUFFER_AAD_LEN ((uint32_t)34)
#define ARG_BUFFER_TAG_LEN ((uint32_t)10)

const uint32_t argbno_plaintext = 0x00;
uint8_t argb_plaintext[ARG_BUFFER_PLAINTEXT_LEN];

const uint32_t argbno_ciphertext = 0x01;
uint8_t argb_ciphertext[ARG_BUFFER_CIPHERTEXT_LEN];

const uint32_t argbno_key = 0x02;
uint8_t argb_key[ARG_BUFFER_KEY_LEN];

const uint32_t argbno_nonce = 0x03;
uint8_t argb_nonce[ARG_BUFFER_NONCE_LEN];

const uint32_t argbno_aad = 0x04;
uint8_t argb_aad[ARG_BUFFER_AAD_LEN];

const uint32_t argbno_tag = 0x05;
uint8_t argb_tag[ARG_BUFFER_TAG_LEN];

uint8_t resb_cryptout[ARG_BUFFER_CRYPT_DATA_LEN];
uint8_t resb_tag[ARG_BUFFER_TAG_LEN];

#define OP_PADDING_SIZE ((uint32_t)0x01)
#define OP_PADDING_CALC ((uint32_t)0x02)
#define OP_ENCRYPT ((uint32_t)0x10)
#define OP_DECRYPT ((uint32_t)0x20)

#define PADDING_SIZE_BYTES ((uint32_t)8)

void __usercall(int a_operation_id, int arg1, int arg2, void *data);

void offer_userv_arg_buffers(void);
void claim_userv_arg_buffers(void);

returncode_t run_encrypt(void);

int main(void) {
  returncode_t rc;

  offer_userv_arg_buffers();

  // Let the userspace service registry know of this service.
  rc = libtock_userv_register(LIBTOCK_USERV_ROLE_CRYPTO, __usercall);
  if (rc != RETURNCODE_SUCCESS) {
    printf("failed to register userv: %d\n", rc);
    while (true);
  }

  while (true) {
    yield();
  }

  return 0;
}

void
__usercall(
  int a_operation_id, 
  int arg1,
  int arg2,
  __attribute__((unused)) void *data) 
{
  claim_userv_arg_buffers();
  const uint32_t operation_id = (uint32_t)a_operation_id;

  switch (operation_id) {
  case OP_PADDING_SIZE:
    libtock_userv_usercall_return_direct(PADDING_SIZE_BYTES, 0);
    break;

  case OP_PADDING_CALC: {
    const uint32_t pt_len = (uint32_t)arg1;
    const uint32_t padded_len = (padded_len % 8 != 0)
                                    ? padded_len + (pt_len % PADDING_SIZE_BYTES)
                                    : padded_len;

    libtock_userv_usercall_return_direct(padded_len, 0);

    break;
  }
  case OP_ENCRYPT: {
    const uint32_t message_len = arg1;
    const uint32_t aad_len = arg2;

    returncode_t rc = run_encrypt();
    offer_userv_arg_buffers();

    libtock_userv_usercall_return();
    break;
  }
  case OP_DECRYPT:
    libtock_userv_usercall_return_error(RETURNCODE_ENOSUPPORT);
    break;

  default:
    break;
  }

  return;
}

/** Offer or claim argument buffers to/from the kernel.
 */
static void __allow_op_arg_buffers(bool reclaim) {
  const uint8_t* const argbnos[] = {argb_plaintext, argb_ciphertext, argb_aad,
                              argb_key,       argb_nonce,      argb_tag};
  const uint32_t argbnos_len[] = {
      ARG_BUFFER_PLAINTEXT_LEN, ARG_BUFFER_CIPHERTEXT_LEN, ARG_BUFFER_AAD_LEN,
      ARG_BUFFER_KEY_LEN,       ARG_BUFFER_NONCE_LEN,      ARG_BUFFER_TAG_LEN};

  for (uint32_t i = 0; i < sizeof(argbnos) / sizeof(argbnos[0]); i++) {
    const uint32_t buffer_len = reclaim ? 0 : argbnos_len[i];
    returncode_t rc = tock_allow_rw_return_to_returncode(
        allow_readwrite(DRIVER_NUM_USERV, i, ((void *) argbnos[i]), buffer_len));

    if (rc != RETURNCODE_SUCCESS) {
      printf("failed to offer/claim argument buffer %ld\n", argbnos[i]);
      while (true)
        ;
    }
  }
}

void offer_userv_arg_buffers(void) {
  __allow_op_arg_buffers(false);

  return;
}

void claim_userv_arg_buffers(void) { 
  __allow_op_arg_buffers(true);

  return;
}

/**
   Buffers:
   - IN: encrypting key
   - IN: nonce
   - IN: plaintext (length byte + data)
   - IN: additional data (length byte + data)
   - OU: ciphertext (length byte + data)
   - OU: AAD tag
*/
returncode_t run_encrypt(void) {
  uint32_t out_ct_len;
  int32_t err = ascon_encrypt(argb_key, argb_nonce, argb_plaintext,
                              argb_plaintext[ARG_BUFFER_PLAINTEXT_LEN - 1],
                              argb_aad, argb_aad[ARG_BUFFER_AAD_LEN - 1],
                              argb_ciphertext, &out_ct_len);

  if (out_ct_len > UINT8_MAX) {
    return RETURNCODE_ESIZE;
  }
  argb_ciphertext[ARG_BUFFER_CIPHERTEXT_LEN - 1] = (uint8_t)out_ct_len;

  return RETURNCODE_SUCCESS;
}

/**
   Buffers:
   - IN: decrypting key
   - IN: nonce
   - IN: ciphertext (length byte + data)
   - IN: AEAD tag (length byte + data)
   - OU: plaintext (length byte + data)
*/
returncode_t run_decrypt(void) { return RETURNCODE_ENOSUPPORT; }
