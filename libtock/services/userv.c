#include "userv.h"

returncode_t
libtock_userv_set_arg_buffer_n(
	uint32_t buffer_no,
	uint8_t* arg_buffer,
	uint32_t buffer_len)
{
	returncode_t rc =
		tock_allow_rw_return_to_returncode(
			allow_readwrite(
				DRIVER_NUM_USERV,
				buffer_no,
				(void*) arg_buffer,
				buffer_len));

	return rc;
}

returncode_t
libtock_userv_unset_arg_buffer_n(
	uint32_t buffer_no)
{
	returncode_t rc =
		tock_allow_rw_return_to_returncode(
			allow_readwrite(
				DRIVER_NUM_USERV,
				buffer_no,
				NULL,
				0));

	return rc;
}



returncode_t libtock_userv_set_result_buffer_n(
	uint32_t buffer_no,
	uint8_t* res_buffer,
	uint32_t buffer_len)
{
	returncode_t rc =
		tock_allow_ro_return_to_returncode(
			allow_readonly(
				DRIVER_NUM_USERV,
				buffer_no,
				(void*) res_buffer,
				buffer_len));

	return rc;
}

returncode_t libtock_userv_unset_result_buffer_n(
	uint32_t buffer_no)
{
	returncode_t rc =
		tock_allow_ro_return_to_returncode(
			allow_readonly(
				DRIVER_NUM_USERV,
				buffer_no,
			    NULL,
			    0));

	return rc;
}

returncode_t
libtock_userv_register(
	const uint32_t role_id,
	subscribe_upcall usercall_fn)
{
	returncode_t rc;

	// Designate the usercall function.
	rc = tock_subscribe_return_to_returncode(
		subscribe(
			DRIVER_NUM_USERV,
			LIBTOCK_USERV_SUBSCRIBE_USERCALL,
			usercall_fn,
			NULL));
	if (rc != RETURNCODE_SUCCESS)
	{
		return rc;
	}

	// Then register as a userspace service for the given role.
	rc = tock_command_return_novalue_to_returncode(
		command(
			DRIVER_NUM_USERV,
			LIBTOCK_USERV_COMMAND_REGISTER,
			role_id,
			0));

	return rc;
}

returncode_t
libtock_userv_usercall_return(void)
{
	return tock_command_return_novalue_to_returncode(
		command(
			DRIVER_NUM_USERV,
		    LIBTOCK_USERV_COMMAND_RETURN_SUCCESS,
			0,
			0));
}

returncode_t
libtock_userv_usercall_return_direct(
	uint32_t val1,
	uint32_t val2)
{
	return tock_command_return_novalue_to_returncode(
		command(
			DRIVER_NUM_USERV,
		    LIBTOCK_USERV_COMMAND_RETURN_SUCCESS,
			val1,
			val2));
}

returncode_t
libtock_userv_usercall_return_error(
	uint32_t errorcode)
{
	return tock_command_return_novalue_to_returncode(
		command(
			DRIVER_NUM_USERV,
			LIBTOCK_USERV_COMMAND_RETURN_FAILURE,
			errorcode,
			0));
}
