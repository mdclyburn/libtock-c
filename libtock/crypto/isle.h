#pragma once

#include <stdint.h>

#include "../tock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRIVER_NUM_ISLE ((uint32_t) 0x30999)

#define ISLE_SUBSCRIBE_OUT_MESSAGE_READY ((uint32_t) 0)

#define ISLE_COMMAND_ENCRYPT ((uint32_t) 1)
#define ISLE_COMMAND_DECRYPT ((uint32_t) 2)
#define ISLE_COMMAND_GET_REALM_INFO ((uint32_t) 10)

#define ISLE_IN_BUFFER  ((uint32_t) 0)
#define ISLE_PIV_BUFFER ((uint32_t) 1)
#define ISLE_SRC_BUFFER ((uint32_t) 2)
#define ISLE_OUT_BUFFER ((uint32_t) 0)

#define ISLE_PIV_BUFFER_LEN ((uint32_t) 4)
#define ISLE_SRC_BUFFER_LEN ((uint32_t) 8)

returncode_t
libtock_isle_allow_ro_set_in_buffer(
	const uint8_t* buffer,
	const uint32_t len);

/** Set the partial IV (pIV) buffer.
 *
 * Set the pIV buffer.
 * This buffer should always be 4 bytes in length.
 */
returncode_t
libtock_isle_allow_ro_set_piv_buffer(
	const uint8_t* buffer);

/** Set the source host number buffer.
 *
 * Set the buffer containing the source host's network number.
 * This buffer should always be 8 bytes in length.
 */
returncode_t
libtock_isle_allow_ro_set_srchost_buffer(
	const uint8_t* buffer);

returncode_t
libtock_isle_allow_rw_set_out_buffer(
	const uint8_t* buffer,
	const uint32_t len);

returncode_t
libtock_isle_subscribe_out_message_ready(
	subscribe_upcall callback_fn);

returncode_t
libtock_isle_command_encrypt(
	const uint64_t dst_host_lower);

returncode_t
libtock_isle_command_decrypt(
    const uint64_t src_host_lower);

returncode_t
libtock_isle_command_realm_id(
	const uint32_t realm_idx,
	uint16_t* const realm_id);

returncode_t
libtock_isle_command_host_network_no(
	const uint32_t realm_idx,
	uint64_t* const host_network_no);

#ifdef __cplusplus
}
#endif
