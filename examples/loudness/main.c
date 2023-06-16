#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <adc.h>
#include <led.h>
#include <timer.h>

#define SAMPLE_BUFFER_LEN 512
#define SAMPLING_PERIOD_MS 7500

tock_timer_t audio_sampling_timer;

bool audio_pending;
uint16_t samples[SAMPLE_BUFFER_LEN];

void check_return_code(int, const char* const);

void audio_sampling_timer_fired(int, int, int, void*);
void audio_buffer_filled(uint8_t, uint32_t, uint16_t*, void*);

int main(void)
{
    /* printf("starting loudness\n"); */
    audio_pending = false;
    check_return_code(
        adc_set_buffer(samples, SAMPLE_BUFFER_LEN),
        "adc_set_buffer");
    check_return_code(
        adc_set_buffered_sample_callback(audio_buffer_filled, NULL),
        "adc_set_buffered_sample_callback");
    timer_every(SAMPLING_PERIOD_MS, audio_sampling_timer_fired, NULL, &audio_sampling_timer);

    while (true)
    {
        // Wait until we have collected the audio.
        while (!audio_pending) { yield(); }

        // Process the audio data.
        // See if it crosses the loudness threshold.
        float avg_loudness = 0.0;
        for (uint32_t i = 0; i < SAMPLE_BUFFER_LEN; i++)
        {
            float sample;
            if (samples[i] >= 32768)
            {
                sample = (float) samples[i] - 32768.0;
            }
            else
            {
                sample = 32768.0 - (float) samples[i];
            }

            avg_loudness += sample / (float) SAMPLE_BUFFER_LEN;
        }

        if (avg_loudness > 20000.0)
        {
            led_on(0);
        }
        else
        {
            led_off(0);
        }

        // Mark the buffer as ready to hold new data.
        audio_pending = false;
    }
}

void check_return_code(const int rc, const char* const note)
{
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("loudness: non-zero return code (%s)\n", note);
        while (true) { yield(); }
    }

    return;
}

void audio_sampling_timer_fired(__attribute__ ((unused)) int a1,
                                __attribute__ ((unused)) int a2,
                                __attribute__ ((unused)) int a3,
                                __attribute__ ((unused)) void* a4)
{
    /* printf("sampling timer fired\n"); */
    const uint8_t channel = 0;
    const uint32_t sampling_frequency = 5120;

    if (!audio_pending)
    {
        check_return_code(
            adc_buffered_sample(channel, sampling_frequency),
            "adc_buffered_sample");
    }

    return;
}

void audio_buffer_filled(__attribute__ ((unused)) uint8_t channel_no,
                         __attribute__ ((unused)) uint32_t sample_count,
                         __attribute__ ((unused)) uint16_t* buffer,
                         __attribute__ ((unused)) void* a4)
{
    audio_pending = true;

    return;
}
