/** biometrics - Periodically sense "biometric" data and encrypt it for eventual sending.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <adc.h>
#include <aes.h>
#include <humidity.h>
#include <temperature.h>
#include <timer.h>

#define SAMPLE_BUFFER_LEN 64
#define SAMPLING_PERIOD_MS 500

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

    while (true)
    {
	    printf("i");

		delay_ms(SAMPLING_PERIOD_MS);

        /* check_return_code( */
		/* 	adc_buffered_sample(0, 2560), */
		/* 	"buffered sample"); */
		adc_buffered_sample(0, 2560);

		delay_ms(SAMPLING_PERIOD_MS);
    }

    return 0;
}

void check_return_code(const int rc, const char* const note)
{
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("oss-adc-sample: non-zero return code (%s)\n", note);
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
