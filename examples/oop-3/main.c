/** oop-3
 *
 * Uses SPI + eInk display.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <adc.h>
#include <aes.h>
#include <humidity.h>
#include <screen.h>
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
	uint32_t refresh_count = 0;
	delay_ms(300);

	while(true)
	{
		check_return_code(
			screen_reset(),
			"reset");
		check_return_code(
			screen_bah(),
			"bah");
		check_return_code(
			screen_update(),
			"update");

		uint8_t refresh_type;
		if (refresh_count++ % 5 == 0)
			refresh_type = SCREEN_REFRESH_FULL;
		else
			refresh_type = SCREEN_REFERSH_PARTIAL;
		check_return_code(
			screen_refresh(refresh_type),
			"refresh");
		check_return_code(
			screen_sleep(),
			"sleep");

		delay_ms(5000);
	}

    return 0;
}

void check_return_code(const int rc, const char* const note)
{
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("oop-2: non-zero return code %d (%s)\n", rc, note);
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
