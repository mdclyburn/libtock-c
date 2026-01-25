#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <libtock-sync/services/alarm.h>
#include <libtock/crypto/aes.h>
#include <libtock/crypto/syscalls/aes_syscalls.h>
#include <libtock/kernel/ipc.h>
#include <libtock/interface/button.h>
#include <libtock/interface/led.h>
#include <libtock/peripherals/gpio.h>
#include <libtock/services/alarm.h>
#include <libtock/tock.h>

#include "aes-gcm.h"

static void test_us_decrypt(void);
static void test_us_decrypt_cb(int, int, int, void*);

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[]) {
	// AES decryption from userspace test.
	test_us_decrypt();
	while (true) { yield(); }

  return 0;
}

uint8_t _g_crypt_key[16];
uint8_t _g_crypt_iv[16];
uint8_t _g_crypt_src[64];
uint8_t _g_crypt_dst[64];

void test_us_decrypt(void)
{
	returncode_t rc;

	/* printf("decrypt start at %ld\n", libtock_unsafe_now()); */

	uint32_t setup_start = libtock_unsafe_now();
	rc = libtock_aes_set_algorithm(LIBTOCK_AES128CBC, true);
	/* printf("rc = %d\n"); */
	rc = libtock_aes_set_readonly_allow_iv_buffer(_g_crypt_iv, 16);
	/* printf("rc = %d\n"); */
	rc = libtock_aes_set_readonly_allow_key_buffer(_g_crypt_key, 16);
	/* printf("rc = %d\n"); */
	rc = libtock_aes_set_readonly_allow_source_buffer(_g_crypt_src, 64);
	/* printf("rc = %d\n"); */
	rc = libtock_aes_set_readwrite_allow_dest_buffer(_g_crypt_dst, 64);
	/* printf("rc = %d\n"); */
	rc = libtock_aes_set_upcall(test_us_decrypt_cb, NULL);
	/* printf("rc = %d\n"); */
	rc = libtock_aes_setup();
	/* printf("rc = %d\n"); */
	printf("setup finish at %ld\n", libtock_unsafe_now() - setup_start);

	rc = libtock_aes_crypt();
	printf("rc = %d\n");

	uint32_t d_start = libtock_unsafe_now();
	for (uint8_t i = 0; i < 64; i++) {
		_g_crypt_dst[i] = _g_crypt_src[i];
	}
	aes128_gcm_decrypt_64bytes(
		_g_crypt_src,
		_g_crypt_dst,
		_g_crypt_key,
		_g_crypt_iv);
	printf("decrypt finish at %ld\n", libtock_unsafe_now() - d_start);

	return;
}

static void test_us_decrypt_cb(int a, int b, int c, void* p) {  }
