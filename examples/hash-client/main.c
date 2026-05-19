#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <libtock/crypto/syscalls/sha_syscalls.h>
#include <libtock-sync/services/alarm.h>

uint8_t g_data[256];
uint8_t g_hash[32];
bool g_next;

char my_str[] = "Hello, world!\0";

void on_digest_upcall(int, int, int, void*);
void succeed_or_hang(returncode_t, const char* const);

int main(void)
{
	libtocksync_alarm_delay_ms(500);

	strncpy((char*) g_data,
			my_str,
			sizeof(g_data));

	if (!libtock_sha_driver_exists())
	{
		printf("Digest driver not available.\n");
	}
	else
	{
		succeed_or_hang(
			libtock_sha_set_upcall(on_digest_upcall, (void*) ((uint32_t) 0)),
			"set done upcall");
		succeed_or_hang(
			libtock_sha_set_add_upcall(on_digest_upcall, (void*) ((uint32_t) 1)),
			"set add upcall");

		succeed_or_hang(
			libtock_sha_set_readwrite_allow_destination_buffer(g_hash, sizeof(g_hash)),
			"set readwrite hash buffer");

		for (uint8_t i = 0; i < 4; i++)
		{
			succeed_or_hang(
				libtock_sha_set_readonly_allow_data_buffer(g_data + (64 * i), 64),
				"set readonly data buffer");
			succeed_or_hang(
				libtock_sha_command_update(),
				"update data");

			while (!g_next) { yield(); }
			g_next = false;

			succeed_or_hang(
				libtock_sha_set_readonly_allow_data_buffer(NULL, 0),
				"set readonly data buffer");
		}
		succeed_or_hang(
			libtock_sha_command_run(),
			"run digest");
		while (!g_next) { yield(); }
		g_next = false;

		printf("Digest: ");
		for (uint8_t i = 0; i < sizeof(g_hash); i++)
			printf("%02x", g_hash[i]);
		printf("\n");
	}

	while (true)
	{
		libtocksync_alarm_delay_ms(250);
	}
}

void
on_digest_upcall(
	int status,
	__attribute__((unused)) int arg2,
	__attribute__((unused)) int arg3,
	void* step)
{
	if ((uint32_t) step == 0) {
		printf("digest addition finished: %d\n", status);
	} else if ((uint32_t) step == 1) {
		printf("digest calculation finished: %d\n", status);
	}

	g_next = true;

	return;
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
