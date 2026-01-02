#pragma once

#include <stdint.h>

#include "../tock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRIVER_NUM_ISLE ((uint32_t) 0x30999)

#define ISLE_COMMAND_ENCRYPT ((uint32_t) 0)
#define ISLE_COMMAND_SET_ADDRESS_LOWER ((uint32_t) 10)
#define ISLE_COMMAND_SET_ADDRESS_UPPER ((uint32_t) 20)

#define ISLE_IN_BUFFER  ((uint32_t) 0)
#define ISLE_OUT_BUFFER ((uint32_t) 0)

returncode_t
libtock_isle_allow_ro_set_in_buffer(
	const uint8_t* buffer,
	const uint32_t len);

returncode_t
libtock_isle_allow_rw_set_out_buffer(
	const uint8_t* buffer,
	const uint32_t len);


/// Set the node IPv6 address.
returncode_t
libtock_isle_command_set_address(
	const uint8_t* const address);

returncode_t
libtock_isle_command_encrypt(
	const uint32_t message_len);

#ifdef __cplusplus
}
#endif
