#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libtock/tock.h>
#include <libtock/peripherals/gpio.h>
#include <libtock/services/userv.h>

#define OP_RUN      ((uint32_t) 0x01)
#define OP_ADD_DATA ((uint32_t) 0x02)
#define OP_VERIFY   ((uint32_t) 0x03)
#define OP_CLEAR    ((uint32_t) 0x11)

#define ARG_BUFFER_LEN ((uint32_t) 256)

uint8_t arg_buffer_0[ARG_BUFFER_LEN];
uint8_t arg_buffer_1[ARG_BUFFER_LEN];
uint8_t arg_buffer_2[ARG_BUFFER_LEN];

uint8_t res_buffer_0[ARG_BUFFER_LEN];

void allow_buffers(void);
void succeed_or_hang(returncode_t, const char* const);
void usercall(int a_operation_id, int arg1, int arg2, void* data);

// SHA-256
typedef struct {
	uint32_t state[8];
	uint64_t count;
	uint8_t buffer[64];
} SHA256_Ctx;

SHA256_Ctx global_sha256_context;
void sha256_init(SHA256_Ctx* ctx);
void sha256_transform(uint32_t state[8], const uint8_t buffer[64]);
void sha256_update(SHA256_Ctx* ctx, const uint8_t* data, uint32_t len);
void sha256_final(SHA256_Ctx* ctx, uint8_t digest[32]);

/* void test_hash(void); */

/* void test_hash(void) */
/* { */
/* 	libtock_gpio_enable_output(0); */
/* 	volatile uint32_t* const gpio = (uint32_t*) (0x50000300 + 0x0504); */

/* 	*gpio |= (1 << 1); */
/* 	sha256_init(&global_sha256_context); */
/* 	for (uint32_t i = 0; i < 4; i++) */
/* 	{ */
/* 		sha256_update(&global_sha256_context, arg_buffer_0, 64); */
/* 	} */

/* 	sha256_final(&global_sha256_context, res_buffer_0); */
/* 	*gpio ^= (1 << 1); */

/* 	return; */
/* } */

int main(void)
{
	/* test_hash(); */

	// Initialize the cryptographic context on startup
	sha256_init(&global_sha256_context);

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
	int arg1,
	__attribute__((unused)) int arg2,
	__attribute__((unused)) void* data)
{
	const uint32_t operation_id = (uint32_t) a_operation_id;
	printf("[digest-userv] usercall 0x%x\n", a_operation_id);

	// Re-take ownership of argument buffers.
	for (uint8_t i = 0; i < 3; i++)
	{
		succeed_or_hang(
			libtock_userv_unset_arg_buffer_n(i),
			"unsetting read-write allow argument buffers");
	}

	switch (operation_id)
	{
	case OP_ADD_DATA:
	{
		const uint8_t* const input_data = arg_buffer_0;
		const uint32_t input_data_len = (uint32_t)arg1;

		if (input_data_len > ARG_BUFFER_LEN) {
			libtock_userv_usercall_return_error(TOCK_STATUSCODE_INVAL);
			break;
		}

		// Update the context incrementally with incoming buffer segments
		sha256_update(&global_sha256_context, input_data, input_data_len);

		allow_buffers();
		libtock_userv_usercall_return();
		break;
	}
	case OP_RUN:
	{
		uint8_t* const digest_output = res_buffer_0;
		const uint32_t digest_len = 32;

		// Finalize computation and extract the digest hash block
		sha256_final(&global_sha256_context, digest_output);

		// Reset context state immediately for subsequent distinct message workflows
		sha256_init(&global_sha256_context);

		// Provide the OS with the result buffer.
		succeed_or_hang(
			libtock_userv_set_result_buffer_n(
				0,
				digest_output,
				digest_len),
			"setting result buffer");
		allow_buffers();
		libtock_userv_usercall_return();

		break;
	}
	case OP_CLEAR:
		memset(arg_buffer_0, 0x00, ARG_BUFFER_LEN);
		memset(arg_buffer_1, 0x00, ARG_BUFFER_LEN);
		memset(arg_buffer_2, 0x00, ARG_BUFFER_LEN);
		memset(res_buffer_0, 0x00, ARG_BUFFER_LEN);

		allow_buffers();
		libtock_userv_usercall_return();

		break;
	case OP_VERIFY:
	default:
		libtock_userv_usercall_return_error(
			TOCK_STATUSCODE_NOSUPPORT);

		break;
	}

	return;
}

/// Offer the argument buffers for the next usercall.
void allow_buffers(void)
{
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
	}

	return;
}

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define Sigma0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define Sigma1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define sigma0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define sigma1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static const uint32_t K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void sha256_init(SHA256_Ctx* ctx)
{
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
	ctx->count    = 0;
}

void sha256_transform(uint32_t state[8], const uint8_t buffer[64])
{
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
	uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
	uint32_t W[64];

	for (int i = 0; i < 16; i++)
	{
		W[i] = ((uint32_t)buffer[i * 4] << 24) |
		       ((uint32_t)buffer[i * 4 + 1] << 16) |
		       ((uint32_t)buffer[i * 4 + 2] << 8) |
		       ((uint32_t)buffer[i * 4 + 3]);
	}

	for (int i = 16; i < 64; i++)
	{
		W[i] = sigma1(W[i - 2]) + W[i - 7] + sigma0(W[i - 15]) + W[i - 16];
	}

	for (int i = 0; i < 64; i++)
	{
		uint32_t t1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];
		uint32_t t2 = Sigma0(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	state[0] += a; state[1] += b; state[2] += c; state[3] += d;
	state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void sha256_update(SHA256_Ctx* ctx, const uint8_t* data, uint32_t len)
{
	uint32_t buffer_left = (uint32_t)(ctx->count & 0x3F);

	ctx->count += len;

	if (buffer_left + len >= 64)
	{
		memcpy(&ctx->buffer[buffer_left], data, 64 - buffer_left);
		sha256_transform(ctx->state, ctx->buffer);
		data += (64 - buffer_left);
		len  -= (64 - buffer_left);
		buffer_left = 0;

		while (len >= 64)
		{
			sha256_transform(ctx->state, data);
			data += 64;
			len  -= 64;
		}
	}

	if (len > 0)
	{
		memcpy(&ctx->buffer[buffer_left], data, len);
	}
}

void sha256_final(SHA256_Ctx* ctx, uint8_t digest[32])
{
	uint64_t total_bits = ctx->count * 8;
	uint32_t buffer_left = (uint32_t)(ctx->count & 0x3F);

	ctx->buffer[buffer_left++] = 0x80;

	if (buffer_left > 56)
	{
		memset(&ctx->buffer[buffer_left], 0, 64 - buffer_left);
		sha256_transform(ctx->state, ctx->buffer);
		buffer_left = 0;
	}

	memset(&ctx->buffer[buffer_left], 0, 56 - buffer_left);

	for (int i = 0; i < 8; i++)
	{
		ctx->buffer[56 + i] = (uint8_t)(total_bits >> (56 - i * 8));
	}

	sha256_transform(ctx->state, ctx->buffer);

	for (int i = 0; i < 8; i++)
	{
		digest[i * 4]     = (uint8_t)(ctx->state[i] >> 24);
		digest[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
		digest[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
		digest[i * 4 + 3] = (uint8_t)(ctx->state[i]);
	}
}
