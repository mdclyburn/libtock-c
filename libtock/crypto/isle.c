#include "isle.h"

returncode_t
libtock_isle_allow_ro_set_in_buffer(
	const uint8_t* in_buffer,
	const uint32_t len)
{
	allow_ro_return_t arval = allow_readonly(
		DRIVER_NUM_ISLE,
		ISLE_IN_BUFFER,
		(void*) in_buffer,
		len);

	return tock_allow_ro_return_to_returncode(arval);
}

returncode_t
libtock_isle_allow_ro_set_piv_buffer(
	const uint8_t* buffer)
{
	const uint32_t len = (buffer == NULL ? 0 : ISLE_PIV_BUFFER_LEN);
	allow_ro_return_t arval = allow_readonly(
		DRIVER_NUM_ISLE,
		ISLE_PIV_BUFFER,
		(void*) buffer,
	    len);

	return tock_allow_ro_return_to_returncode(arval);
}

returncode_t
libtock_isle_allow_ro_set_srchost_buffer(
	const uint8_t* buffer)
{
	const uint32_t len = (buffer == NULL ? 0 : ISLE_SRC_BUFFER_LEN);
	allow_ro_return_t arval = allow_readonly(
		DRIVER_NUM_ISLE,
		ISLE_SRC_BUFFER,
		(void*) buffer,
		len);

	return tock_allow_ro_return_to_returncode(arval);
}

returncode_t
libtock_isle_allow_rw_set_out_buffer(
	const uint8_t* out_buffer,
	const uint32_t len)
{
	allow_rw_return_t arval = allow_readwrite(
		DRIVER_NUM_ISLE,
		ISLE_OUT_BUFFER,
		(void*) out_buffer,
		len);

	return tock_allow_rw_return_to_returncode(arval);
}

returncode_t
libtock_isle_subscribe_out_message_ready(
	subscribe_upcall callback_fn)
{
	subscribe_return_t srval = subscribe(
		DRIVER_NUM_ISLE,
		ISLE_SUBSCRIBE_OUT_MESSAGE_READY,
		callback_fn,
		NULL);

	return tock_subscribe_return_to_returncode(srval);
}

returncode_t
libtock_isle_command_encrypt(
	uint64_t dst_host)
{
	syscall_return_t crval = command(
		DRIVER_NUM_ISLE,
		ISLE_COMMAND_ENCRYPT,
	    ((uint32_t) (dst_host & 0xFFFFFFFF)),
		((uint32_t) ((dst_host >> 32) & 0xFFFFFFFF)));

	return tock_command_return_novalue_to_returncode(crval);
}

returncode_t
libtock_isle_command_decrypt(
	const uint64_t src_host)
{
	syscall_return_t crval = command(
		DRIVER_NUM_ISLE,
		ISLE_COMMAND_DECRYPT,
	    ((uint32_t) (src_host & 0xFFFFFFFF)),
		((uint32_t) ((src_host >> 32) & 0xFFFFFFFF)));

	return tock_command_return_novalue_to_returncode(crval);
}

#define ISLE_REALM_INFO_ID_REALM_ID ((uint32_t) 0)
#define ISLE_REALM_INFO_ID_HOST_NETWORK_NO ((uint32_t) 1)

returncode_t
libtock_isle_command_realm_id(
	const uint32_t realm_idx,
	uint16_t* const realm_id)
{
	uint32_t out_realm_id;
	syscall_return_t crval = command(
		DRIVER_NUM_ISLE,
		ISLE_COMMAND_GET_REALM_INFO,
		realm_idx,
		ISLE_REALM_INFO_ID_REALM_ID);

    returncode_t rt = tock_command_return_u32_to_returncode(
		crval,
		&out_realm_id);
	if (rt == RETURNCODE_SUCCESS) {
		*realm_id = (uint16_t) out_realm_id;
	}

	return rt;
}

returncode_t
libtock_isle_command_host_network_no(
	const uint32_t realm_idx,
	uint64_t* const host_network_no)
{
	syscall_return_t crval = command(
		DRIVER_NUM_ISLE,
		ISLE_COMMAND_GET_REALM_INFO,
		realm_idx,
		ISLE_REALM_INFO_ID_HOST_NETWORK_NO);

	return tock_command_return_u64_to_returncode(
		crval,
		host_network_no);
}
