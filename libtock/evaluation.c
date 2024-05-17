#include <stdint.h>
#include <stdio.h>

#include "evaluation.h"
#include "led.h"
#include "rng.h"
#include "tock.h"

static uint32_t __lcg_parkmiller(uint32_t* const state);

void eval_setup(void)
{
    // Setup pRNG.
    uint32_t rnd;
    int bytes_rcv;
    eval_check_return_code(
        rng_sync((uint8_t*) &rnd, 4, 4, &bytes_rcv),
        "eval_setup, rng_sync");
    eval_usprng_init(rnd);

    return;
}

void eval_check_return_code(const int rc,
                            const char* const note)
{
    if (rc != RETURNCODE_SUCCESS)
    {
        led_on(0);
        printf("%s: %d\n", note, rc);
        // Spin forever to waste energy, making error state clear.
        while (true) {  }
    }

    return;
}

uint32_t __prng_state = 0xffffffff;

void eval_usprng_init(const uint32_t seed)
{
    __prng_state = seed;

    return;
}

uint32_t eval_usprng_next(void)
{
    return __lcg_parkmiller(&__prng_state);
}

uint32_t __lcg_parkmiller(uint32_t* const state)
{
    const uint32_t N = 0x7fffffff;
    const uint32_t G = 48271u;

    uint32_t div = *state / (N / G);  /* max : 2,147,483,646 / 44,488 = 48,271 */
    uint32_t rem = *state % (N / G);  /* max : 2,147,483,646 % 44,488 = 44,487 */

    uint32_t a = rem * G;        /* max : 44,487 * 48,271 = 2,147,431,977 */
    uint32_t b = div * (N % G);  /* max : 48,271 * 3,399 = 164,073,129 */

    *state = (a > b) ? (a - b) : (a + (N - b));

    return *state;
}
