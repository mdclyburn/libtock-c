/** oop-1
 *
 * The "stable" timed application.
 * Uses the ADC.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <adc.h>
#include <aes.h>
#include <alarm.h>
#include <humidity.h>
#include <rng.h>
#include <temperature.h>
#include <timer.h>

#define SAMPLE_BUFFER_LEN 64
#define WAIT_PERIOD_MS 1000
#define ACTION_LIMIT 28

uint16_t samples[SAMPLE_BUFFER_LEN];

void check_return_code(const int rc, const char* const note);

void audio_buffer_filled(uint8_t, uint32_t, uint16_t*, void*);

int main(void)
{
	int nr;
	uint32_t pre_delay;
	rng_sync((uint8_t*) &pre_delay, 4, 4, &nr);
	printf("oop-5: rng: %lu\n", pre_delay % 1000);
	/* printf("oop-2: pre-delay = %ld\n", pre_delay); */
	delay_ms(pre_delay % 1000);

	uint32_t actions = 0;
    while (actions++ < ACTION_LIMIT)
    {
		aes_do_something();
    }

    return 0;
}

void check_return_code(const int rc, const char* const note)
{
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("oop-1: non-zero return code (%s)\n", note);
        while (true) { yield(); }
    }

    return;
}

void audio_buffer_filled(__attribute__ ((unused)) uint8_t channel_no,
                         __attribute__ ((unused)) uint32_t sample_count,
                         __attribute__ ((unused)) uint16_t* buffer,
                         __attribute__ ((unused)) void* a4)
{
    return;
}
