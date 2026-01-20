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
libtock_isle_command_set_address(
	const uint8_t* const address)
{
	syscall_return_t crval;
	returncode_t rc;

	crval = command(
		DRIVER_NUM_ISLE,
		ISLE_COMMAND_SET_ADDRESS_LOWER,
		*((uint32_t*) (address + 0)),
		*((uint32_t*) (address + 4)));
	rc = tock_command_return_novalue_to_returncode(crval);
	if (rc != 0) {
		return rc;
	}

	crval = command(
		DRIVER_NUM_ISLE,
		ISLE_COMMAND_SET_ADDRESS_UPPER,
		*((uint32_t*) (address + 8)),
		*((uint32_t*) (address + 12)));

	return tock_command_return_novalue_to_returncode(crval);
}

returncode_t
libtock_isle_command_encrypt(
	const uint32_t message_len,
	const uint32_t aad_len)
{
	syscall_return_t crval = command(
		DRIVER_NUM_ISLE,
		ISLE_COMMAND_ENCRYPT,
		message_len,
		aad_len);

	return tock_command_return_novalue_to_returncode(crval);
}

returncode_t
libtock_isle_command_decrypt(
	const uint32_t message_len,
	const uint32_t aad_len)
{
	syscall_return_t crval = command(
		DRIVER_NUM_ISLE,
		ISLE_COMMAND_DECRYPT,
		message_len,
		aad_len);

	return tock_command_return_novalue_to_returncode(crval);
}
