#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <adc.h>
#include <evaluation.h>
#include <led.h>
#include <timer.h>

#define SAMPLE_BUFFER_LEN 128
#define SAMPLING_PERIOD_MS 3000
#define SAMPLING_JITTER_MS ( SAMPLING_PERIOD_MS / 10 )
#define ACTION_LIMIT 1000
#define THRESHOLD_STATE_LEN 5

tock_timer_t audio_sampling_timer;

bool audio_pending = false;
uint16_t samples[SAMPLE_BUFFER_LEN];

void audio_sampling_timer_fired(int, int, int, void*);
void audio_buffer_filled(uint8_t, uint32_t, uint16_t*, void*);

int main(void)
{
    eval_setup();

    eval_check_return_code(
        adc_set_buffer(samples, SAMPLE_BUFFER_LEN),
        "loudness: adc_set_buffer");
    eval_check_return_code(
        adc_set_buffered_sample_callback(&audio_buffer_filled, NULL),
        "loudness: adc_set_buffered_sample_callback");

    const int32_t jitter = ((int32_t) (eval_usprng_next() % (SAMPLING_JITTER_MS / 2))) - SAMPLING_JITTER_MS;
    timer_every(SAMPLING_PERIOD_MS + jitter,
                audio_sampling_timer_fired,
                NULL,
                &audio_sampling_timer);

    uint16_t baseline = 0xFFFF;
    int actions = 0;
    bool threshold_exceeded[THRESHOLD_STATE_LEN];
    uint16_t threshold_exceeded_i = 0;

    for (uint16_t i = 0; i < THRESHOLD_STATE_LEN; i++)
        threshold_exceeded[i] = false;

    while (actions++ < ACTION_LIMIT)
    {

        // Wait until we have collected the audio.
        while (!audio_pending) { yield(); }

        if (baseline == 0xFFFF) // Need to capture a baseline.
        {
            // Assume what we have now is the baseline.
            uint32_t total = 0;
            for (uint32_t i = 0; i < SAMPLE_BUFFER_LEN; i++)
                total += samples[i];
            total /= SAMPLE_BUFFER_LEN;
        }
        else
        {
            // Process the audio data.
            // See if it crosses the level threshold.
            float avg_level = 0.0;
            for (uint32_t i = 0; i < SAMPLE_BUFFER_LEN; i++)
            {
                float sample;
                avg_level += sample / (float) SAMPLE_BUFFER_LEN;
            }

            if (avg_level > 20000.0)
            {
                threshold_exceeded[threshold_exceeded_i++] = true;
                // Check if threshold has been exceeded enough times.
                uint8_t count = 0;
                for (uint16_t i = 0; i < THRESHOLD_STATE_LEN; i++)
                {
                    if (threshold_exceeded[i])
                        count++;
                }

                if (count == THRESHOLD_STATE_LEN)
                {
                    // Notify.

                    // Clear buffer.
                    for (uint16_t i = 0; i < THRESHOLD_STATE_LEN; i++)
                        threshold_exceeded[i] = false;
                }
            }
            else
            {
                threshold_exceeded[threshold_exceeded_i++] = false;
            }
        }

        // Mark the buffer as ready to hold new data.
        audio_pending = false;
    }
}

void audio_sampling_timer_fired(__attribute__ ((unused)) int a1,
                                __attribute__ ((unused)) int a2,
                                __attribute__ ((unused)) int a3,
                                __attribute__ ((unused)) void* a4)
{
    /* printf("sampling timer fired\n"); */
    const uint8_t channel = 0;
    const uint32_t sampling_frequency = 2560;

    if (!audio_pending)
    {
        while (true)
        {
            int rc = adc_buffered_sample(channel, sampling_frequency);
            break;
            /* if (rc == 0) */
                /* break; */

            delay_ms(100);
        }
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
