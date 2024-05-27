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
    check_return_code(
        adc_set_buffer(samples, SAMPLE_BUFFER_LEN),
        "adc_set_buffer");
    check_return_code(
        adc_set_buffered_sample_callback(&audio_buffer_filled, NULL),
        "adc_set_buffered_sample_callback");

	uint32_t actions = 0;
    while (actions++ < ACTION_LIMIT)
    {
		adc_buffered_sample(0, 2560);
		delay_ms(WAIT_PERIOD_MS);
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
