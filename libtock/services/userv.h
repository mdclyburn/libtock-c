#ifndef LIBTOCK_USERV_H
#define LIBTOCK_USERV_H

#include <stdint.h>

#include "../tock.h"

#define DRIVER_NUM_USERV ((uint32_t) 0x10003)

#define LIBTOCK_USERV_COMMAND_REGISTER ((uint32_t) 0x10)
#define LIBTOCK_USERV_COMMAND_RETURN_SUCCESS ((uint32_t) 0x11)
#define LIBTOCK_USERV_COMMAND_RETURN_FAILURE ((uint32_t) 0x12)

#define LIBTOCK_USERV_ALLOW_RW_ARG_BUFFER ((uint32_t) 0x00)

#define LIBTOCK_USERV_SUBSCRIBE_USERCALL ((uint32_t) 0x00)

#define LIBTOCK_USERV_ARG_BUFFER_LEN ((uint32_t) 384)

#define LIBTOCK_USERV_ROLE_CRYPTO ((uint32_t) 0xA0)
#define LIBTOCK_USERV_ROLE_DIGEST ((uint32_t) 0x11)

returncode_t libtock_userv_set_arg_buffer_n(
	uint32_t buffer_no,
	uint8_t* arg_buffer,
	uint32_t buffer_len);

returncode_t libtock_userv_unset_arg_buffer_n(
	uint32_t buffer_no);

returncode_t
libtock_userv_register(
	const uint32_t role_id,
	subscribe_upcall usercall_fn);

returncode_t
libtock_userv_usercall_return(void);

returncode_t
libtock_userv_usercall_return_direct(
	uint32_t val1,
	uint32_t val2);

returncode_t
libtock_userv_usercall_return_error(
	uint32_t errorcode);

#endif
