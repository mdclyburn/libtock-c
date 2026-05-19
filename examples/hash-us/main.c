#include <stdint.h>
#include <stdio.h>

#include <libtock/tock.h>
#include <libtock/services/userv.h>

#define OP_ADD_DATA ((uint32_t) 0x02)
#define OP_RUN      ((uint32_t) 0x01)
#define OP_VERIFY   ((uint32_t) 0x03)

#define ARG_BUFFER_LEN ((uint32_t) 128)

uint8_t arg_buffer_0[ARG_BUFFER_LEN];
uint8_t arg_buffer_1[ARG_BUFFER_LEN];
uint8_t arg_buffer_2[ARG_BUFFER_LEN];

void succeed_or_hang(returncode_t, const char* const);
void usercall(int a_operation_id, int arg1, int arg2, void* data);

int main(void)
{
	// Offer the buffers to the service registry.
	uint8_t* arg_buffers[3] = {
		arg_buffer_0,
		arg_buffer_1,
		arg_buffer_2
	};
	for (uint32_t i = 0; i < 3; i++)
	{
		succeed_or_hang(
			libtock_userv_set_arg_buffer_n(i, arg_buffers[i], ARG_BUFFER_LEN),
			"setting read-write allow argument buffers");
		printf("set buffer %ld\n", i);
	}

	// Let the userspace service registry know of this service.
	succeed_or_hang(
		libtock_userv_register(
			LIBTOCK_USERV_ROLE_DIGEST,
			usercall),
		"registering service");

	while (true) { yield(); }
}

void succeed_or_hang(returncode_t rc, const char* const msg)
{
	if (rc != RETURNCODE_SUCCESS)
	{
		printf("%s: %d\n", msg, rc);
		while (true);
	}

	return;
}

void usercall(
	int a_operation_id,
	__attribute__((unused)) int arg1,
	__attribute__((unused)) int arg2,
	__attribute__((unused)) void* data)
{
	const uint32_t operation_id = (uint32_t) a_operation_id;
	printf("hash-us: usercall 0x%x\n", a_operation_id);

	switch (operation_id)
	{
	case OP_ADD_DATA:
	case OP_RUN:
	case OP_VERIFY:
	default:
		libtock_userv_usercall_return_error(
			RETURNCODE_ENOSUPPORT);

		break;
	}
}
