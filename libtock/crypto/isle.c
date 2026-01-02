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
libtock_isle_command_encrypt(
	const uint32_t message_len)
{
	command_return_t crval = command(
		DRIVER_NUM_ISLE,
		ISLE_COMMAND_ENCRYPT,
		message_len,
		0);

	return tock_command_return_novalue_to_returncode(crval);
}
