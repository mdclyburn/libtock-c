#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libtock/services/userv.h>
#include <libtock/tock.h>

#include "ascon.h"

#define ARG_BUFFER_CRYPT_DATA_LEN ((uint32_t)82)
#define ARG_BUFFER_INPUT_LEN (ARG_BUFFER_CRYPT_DATA_LEN)
#define ARG_BUFFER_OUTPUT_LEN (ARG_BUFFER_CRYPT_DATA_LEN)
#define ARG_BUFFER_KEY_LEN ((uint32_t)18)
#define ARG_BUFFER_NONCE_LEN ((uint32_t)18)
#define ARG_BUFFER_AAD_LEN ((uint32_t)34)
#define ARG_BUFFER_TAG_LEN ((uint32_t)18)

uint8_t argb_input[ARG_BUFFER_INPUT_LEN];
uint8_t argb_key[ARG_BUFFER_KEY_LEN];
uint8_t argb_nonce[ARG_BUFFER_NONCE_LEN];
uint8_t argb_aad[ARG_BUFFER_AAD_LEN];
uint8_t argb_tag[ARG_BUFFER_TAG_LEN];

uint8_t resb_output[ARG_BUFFER_OUTPUT_LEN];
uint8_t resb_tag[ARG_BUFFER_TAG_LEN];

#define OP_PADDING_SIZE ((uint32_t)0x01)
#define OP_PADDING_CALC ((uint32_t)0x02)
#define OP_ENCRYPT ((uint32_t)0x10)
#define OP_DECRYPT ((uint32_t)0x20)

#define PADDING_SIZE_BYTES ((uint32_t)8)

void __usercall(int a_operation_id, int arg1, int arg2, void *data);

void offer_userv_arg_buffers(void);
void claim_userv_arg_buffers(void);

returncode_t run_encrypt(
  const uint32_t plaintext_len,
  const uint32_t aad_len,
  uint32_t* const ciphertext_len
);
returncode_t run_decrypt(void);

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
    const uint32_t padded_len = 
      (pt_len % PADDING_SIZE_BYTES != 0) ? pt_len + (pt_len % PADDING_SIZE_BYTES) : pt_len;

    libtock_userv_usercall_return_direct(padded_len, 0);

    break;
  }
  case OP_ENCRYPT: {
    const uint32_t plaintext_len = (uint32_t) arg1;
    const uint32_t aad_len = (uint32_t) arg2;
    uint32_t ciphertext_len;

    returncode_t rc = run_encrypt(plaintext_len, aad_len, &ciphertext_len);

    offer_userv_arg_buffers();

    if (rc == RETURNCODE_SUCCESS)
    {
      libtock_userv_usercall_return_direct(ciphertext_len, ASCON_TAG_SIZE);
    }
    else
    {
      libtock_userv_usercall_return_error((uint32_t) rc);
    }

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
static void
__allow_op_arg_buffers(
  bool reclaim
) 
{
  // The argument/result buffers and their ordering.
  const uint8_t* const argbnos[] = {
    argb_input,
    argb_aad,
    argb_key,
    argb_nonce,
    argb_tag,
  };
  const uint8_t* const resbnos[] = {
    resb_output,
    resb_tag,
  };

  const uint32_t argbnos_len[] = {
      ARG_BUFFER_INPUT_LEN,
      ARG_BUFFER_AAD_LEN,
      ARG_BUFFER_KEY_LEN,
      ARG_BUFFER_NONCE_LEN,
      ARG_BUFFER_TAG_LEN,
    };
  const uint32_t resbnos_len[] = {
    ARG_BUFFER_OUTPUT_LEN,
    ARG_BUFFER_TAG_LEN,
  };

  for (uint32_t i = 0; i < sizeof(argbnos) / sizeof(argbnos[0]); i++)
  {
    const uint32_t buffer_len = reclaim ? 0 : argbnos_len[i];
    returncode_t rc = tock_allow_rw_return_to_returncode(
        allow_readwrite(DRIVER_NUM_USERV, i, ((void *) argbnos[i]), buffer_len));

    if (rc != RETURNCODE_SUCCESS)
    {
      printf("failed to offer/claim argument buffer %ld\n", i);
      while (true);
    }
  }

  for (uint32_t i = 0; i < sizeof(resbnos) / sizeof(resbnos[0]); i++) {
    const uint32_t buffer_len = reclaim ? 0 : resbnos_len[i];
    returncode_t rc = tock_allow_ro_return_to_returncode(
        allow_readonly(DRIVER_NUM_USERV, i, ((void *) resbnos[i]), buffer_len));

    if (rc != RETURNCODE_SUCCESS)
    {
      printf("failed to offer/claim result buffer %ld\n", i);
      while (true);
    }
  }

  return;
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
   - IN: plaintext data
   - IN: additional data
   - OU: ciphertext data
   - OU: AAD tag
*/
returncode_t run_encrypt(
  const uint32_t plaintext_len,
  const uint32_t aad_len,
  uint32_t* const ciphertext_len
)
{
  int32_t err = ascon_encrypt(
    argb_key,
    argb_nonce,
    argb_input,
    plaintext_len,
    argb_aad,
    aad_len,
    resb_output,
    ciphertext_len
  );

  if (err == 0)
  {
    printf("ct len: %lu, tag len: %u\n", *ciphertext_len - ASCON_TAG_SIZE, ASCON_TAG_SIZE);

    // Copy AAD tag to the result buffer.
    memcpy(resb_tag, (uint8_t*) (resb_output + *ciphertext_len), ASCON_TAG_SIZE);

    return RETURNCODE_SUCCESS;
  }
  else
  {
    return RETURNCODE_FAIL;
  }
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
