#include <stdint.h>
#include <stdio.h>

#include <libtock/tock.h>
#include <libtock/services/userv.h>

#include "ascon.h"

uint8_t arg_buffer[LIBTOCK_USERV_ARG_BUFFER_LEN];

#define OP_PADDING_SIZE ((uint32_t) 0x01)
#define OP_PADDING_CALC ((uint32_t) 0x02)
#define OP_ENCRYPT      ((uint32_t) 0x10)
#define OP_DECRYPT      ((uint32_t) 0x20)

#define PADDING_SIZE_BYTES ((uint32_t) 8)

void __usercall(
	int a_operation_id,
	int arg1,
    int arg2,
	void* data);

returncode_t run_encrypt(void);

int main(void)
{
	// Let the userspace service registry know of this service.
	libtock_userv_register(
		LIBTOCK_USERV_ROLE_CRYPTO,
		__usercall,
		arg_buffer);

	while (true)
	{
		yield();
	}

	return 0;
}

void __usercall(
	int a_operation_id,
	int arg1,
    __attribute__((unused)) int arg2,
	__attribute__((unused)) void* data)
{
	const uint32_t operation_id = (uint32_t) a_operation_id;

	switch (operation_id)
	{
	case OP_PADDING_SIZE:
		libtock_userv_usercall_return_direct(
			PADDING_SIZE_BYTES,
			0);
		break;

	case OP_PADDING_CALC:
	{
		const uint32_t pt_len = (uint32_t) arg1;
		uint32_t padded_len = pt_len;
		if (pt_len % 8 != 0)
		{
			padded_len += pt_len % PADDING_SIZE_BYTES;
		}

		libtock_userv_usercall_return_direct(
			padded_len,
			0);

		break;
	}
	case OP_ENCRYPT:
	{
		returncode_t rc = run_encrypt();
		libtock_userv_usercall_return();
		break;
	}
	case OP_DECRYPT:
		libtock_userv_usercall_return_error(
			RETURNCODE_ENOSUPPORT);
		break;

	default:
		break;
	}

	return;
}

returncode_t run_encrypt(void)
{
	// Un-allow the buffer from the kernel.
	returncode_t rc = tock_allow_rw_return_to_returncode(
		allow_readwrite(
			DRIVER_NUM_USERV,
			LIBTOCK_USERV_ALLOW_RW_ARG_BUFFER,
			(void*) arg_buffer,
			LIBTOCK_USERV_ARG_BUFFER_LEN));

	const uint8_t* const ckey = arg_buffer + 2;
	const uint8_t* const nonce = arg_buffer + (2 + 16 + 2);
	const uint32_t* const pt_len = (uint32_t*) (arg_buffer + 2 + 16 + 2 + 16 + 1);
	const uint8_t* const pt = (arg_buffer + 2 + 16 + 2 + 16 + 5);
	const uint32_t* const aad_len = (uint32_t*) (arg_buffer + 2 + 16 + 2 + 16 + 5 + *pt_len + 1);
	const uint8_t* const aad = (arg_buffer + 2 + 16 + 2 + 16 + 5 + *pt_len + 5);

	printf("pt_len = %d B, aad_len = %d B\n", pt_len, aad_len);

	return RETURNCODE_SUCCESS;
}
