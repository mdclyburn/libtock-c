#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <adc.h>
#include <timer.h>

#define SAMPLE_BUFFER_LEN 128
#define SAMPLING_PERIOD_MS 600

tock_timer_t audio_sampling_timer;

uint16_t samples[SAMPLE_BUFFER_LEN];

void check_return_code(int, const char* const);

void audio_sampling_timer_fired(int, int, int, void*);
void audio_buffer_filled(uint8_t, uint32_t, uint16_t*, void*);

int main(void)
{
    while (true)
    {
        check_return_code(
			adc_buffered_sample(0, 2560),
			"buffered sample");
		delay_ms(SAMPLING_PERIOD_MS);
	}
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
