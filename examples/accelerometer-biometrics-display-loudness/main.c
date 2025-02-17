/** biometrics - Periodically sense temperature and EDA and encrypt it for sending.
 */

#include <stdint.h>
#include <stdio.h>

#include <adc.h>
#include <aes.h>
#include <evaluation.h>
#include <humidity.h>
#include <ninedof.h>
#include <temperature.h>
#include <timer.h>

// -----------v

#define SAMPLE_BUFFER_LEN 128
#define AUDIO_ACTION_LIMIT 7
#define THRESHOLD_STATE_LEN 5

tock_timer_t audio_sampling_timer;
tock_timer_t accel_timer;

bool audio_pending = false;
uint16_t samples[SAMPLE_BUFFER_LEN];

void audio_sampling_timer_fired(int, int, int, void*);
void accel_timer_fired(int, int, int, void*);

void audio_buffer_filled(uint8_t, uint32_t, uint16_t*, void*);

// -----------^

#define TEMPERATURE_SAMPLING_PERIOD_MS 2000
#define GSR_SAMPLING_PERIOD_MS (TEMPERATURE_SAMPLING_PERIOD_MS) * 2

#define TEMP_ACTION_LIMIT 9
#define GSR_ACTION_LIMIT 5

int g_t_actions = 0;
int g_gsr_actions = 0;
int g_a_actions = 0;

tock_timer_t g_temperature_timer;
tock_timer_t g_gsr_timer;

#define GSR_SAMPLES_LEN 10
uint16_t g_gsr_samples[128];

enum ReadingStatus {
    R_EMPTY,
    R_PENDING,
    R_DONE
};

struct
{
    int temperature;
    uint16_t gsr_result;
} g_current_readings;

enum ReadingStatus g_temperature_status;
enum ReadingStatus g_gsr_status;

struct reading {
    int temperature;
    int gsr;
};

#define READINGS_BUFFER_LEN 10
struct reading g_readings[READINGS_BUFFER_LEN];

void gsr_timer_fired(int, int, int, void*);
void temperature_timer_fired(int, int, int, void*);

void gsr_sampling_completed(uint8_t, uint32_t, uint16_t*, void*);
void temperature_reading_completed(int, int, int, void*);

bool is_ready(void);

uint32_t g_next_firings[3];
tock_timer_t g_timer;
uint32_t g_waiting_for;
void global_timer_fired(int, int, int, void*);

int main(void)
{
    eval_setup();

    eval_check_return_code(
        temperature_set_callback(temperature_reading_completed, NULL),
        "biometrics: temperature_set_callback");
    eval_check_return_code(
        adc_set_buffer(g_gsr_samples, GSR_SAMPLES_LEN),
        "biometrics: adc_set_buffer");
    eval_check_return_code(
        adc_set_buffered_sample_callback(&gsr_sampling_completed, NULL),
        "biometrics: adc_set_buffered_sample_callbck");

	g_waiting_for = 1000;
	// 0 = temperature
	g_next_firings[0] = 2000;
	g_next_firings[1] = 4000; // gsr
	g_next_firings[2] = 3000; // audio

	timer_every(g_waiting_for, // << because that's when the first event is
			 global_timer_fired,
			 NULL,
			 &g_timer);

	timer_every(826,
				accel_timer_fired,
				NULL,
				&accel_timer);

	// ---------v

    uint16_t baseline = 0xFFFF;
    int actions = 0;
    bool threshold_exceeded[THRESHOLD_STATE_LEN];
    uint16_t threshold_exceeded_i = 0;

    for (uint16_t i = 0; i < THRESHOLD_STATE_LEN; i++)
        threshold_exceeded[i] = false;

	// ---------^

    uint16_t reading_i = 0;
    g_temperature_status = R_EMPTY;
    g_gsr_status = R_EMPTY;
    while (g_gsr_actions <= GSR_ACTION_LIMIT && g_t_actions <= TEMP_ACTION_LIMIT)
    {
        g_temperature_status = R_EMPTY;
        g_gsr_status = R_EMPTY;

        while (!is_ready() && !audio_pending) { yield(); }

		if (is_ready())
		{
			// Pack the data for encryption and later sending.
			g_readings[reading_i].temperature = g_current_readings.temperature;
			g_readings[reading_i].gsr = g_current_readings.gsr_result;

			// We encrypt the data once we have reached a certain amount of data.
			reading_i = (reading_i + 1) % READINGS_BUFFER_LEN;
			if (reading_i == 0)
			{
				/* printf("encrypting\n"); */
				aes_do_something();
			}
		}

		if (audio_pending)
		{
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

    timer_cancel(&g_temperature_timer);
    timer_cancel(&g_gsr_timer);

    while (true) { yield(); }

    return 0;
}

bool is_ready(void)
{
    return (g_temperature_status == R_DONE)
        && (g_gsr_status == R_DONE);
}

#define GSR_SAMPLING_CHANNEL 1
#define GSR_SAMPLING_FREQ (25 * 2 * 2 * 2)

void temperature_timer_fired(__attribute__ ((unused)) int a1,
                         __attribute__ ((unused)) int a2,
                         __attribute__ ((unused)) int a3,
                         __attribute__ ((unused)) void* a4)
{
    if (g_temperature_status != R_EMPTY)
        return;

    g_temperature_status = R_PENDING;
    temperature_read();

    return;
}

// whether the adc was in conflict at the time.
// we check this when the first use finishes to start the next.
// NOTE: we must be able to guarantee that gsr runtime + audio runtime < minimum time between their firings without overlap.
bool adc_in_use = false;
bool adc_in_conflict = false;
uint32_t g_dsp_actions = 0;
#define DSP_ACTION_LIMIT 18

void global_timer_fired(__attribute__ ((unused)) int a1,
						__attribute__ ((unused)) int a2,
						__attribute__ ((unused)) int a3,
						__attribute__ ((unused)) void* a4)
{
	g_dsp_actions++;
	if (g_dsp_actions < DSP_ACTION_LIMIT)
	{
		printf("D");
	}

	g_waiting_for = 1000;

	// subtract waiting time from next firings
	for (uint8_t i = 0; i < 3; i++)
	{
		g_next_firings[i] -= g_waiting_for;
		// run those that are zero, refresh their count
		if (g_next_firings[i]  == 0)
		{
			switch (i)
			{
			case 0: // temp
				// limit actions
				g_t_actions++;
				if (g_t_actions >= TEMP_ACTION_LIMIT)
				{
					g_next_firings[0] = 1;
				}
				else
				{
					g_next_firings[0] = 2000;
				}

				if (g_temperature_status != R_EMPTY)
					return;

				g_temperature_status = R_PENDING;
				temperature_read();
				break;

			case 1: // gsr
				// limit actions
				g_a_actions++;
				if (g_a_actions >= AUDIO_ACTION_LIMIT)
				{
					g_next_firings[1] = 600000;
				}
				else
				{
					g_next_firings[1] = 4000;
				}

				if (adc_in_use)
				{
					adc_in_conflict = true;
				}
				else
				{
					if (g_gsr_status != R_EMPTY)
					{
						return;
					}

					eval_check_return_code(
						adc_set_buffered_sample_callback(&gsr_sampling_completed, NULL),
						"biometrics: adc_set_buffered_sample_callbck");

					adc_in_use = true;
					adc_buffered_sample(GSR_SAMPLING_CHANNEL, GSR_SAMPLING_FREQ);
				}
				break;

			case 2: // audio
				// limit actions
				g_gsr_actions++;
				if (g_gsr_actions >= GSR_ACTION_LIMIT)
				{
					g_next_firings[2] = 600000;
				}
				else
				{
					g_next_firings[2] = 4000;
				}
				if (adc_in_use)
				{
					adc_in_conflict = true;
				}
				else
				{

					const uint8_t channel = 0;
					const uint32_t sampling_frequency = 2560;

					if (!audio_pending)
					{
						adc_in_use = true;
						eval_check_return_code(
							adc_set_buffered_sample_callback(&audio_buffer_filled, NULL),
							"biometrics: adc_set_buffered_sample_callback");

						adc_buffered_sample(channel, sampling_frequency);
					}
				}
				break;
			default:
				break;
			}
		}
	}

	return;
}

void gsr_timer_fired(__attribute__ ((unused)) int a1,
                     __attribute__ ((unused)) int a2,
                     __attribute__ ((unused)) int a3,
                     __attribute__ ((unused)) void* a4)
{
    if (g_gsr_status != R_EMPTY)
	{
        return;
	}

	eval_check_return_code(
		adc_set_buffered_sample_callback(&audio_buffer_filled, NULL),
		"biometrics: adc_set_buffered_sample_callback");
	adc_buffered_sample(GSR_SAMPLING_CHANNEL, GSR_SAMPLING_FREQ);

    return;
}

void temperature_reading_completed(int temperature,
                                   __attribute__ ((unused)) int a2,
                                   __attribute__ ((unused)) int a3,
                                   __attribute__ ((unused)) void* a4)
{
    g_current_readings.temperature = temperature;
    /* printf("temperature callback\n"); */
    g_temperature_status = R_DONE;

	if (++g_t_actions >= TEMP_ACTION_LIMIT)
	{
		timer_cancel(&g_temperature_timer);
	}

    return;
}

void gsr_sampling_completed(__attribute__ ((unused)) uint8_t channel_no,
                            __attribute__ ((unused)) uint32_t sample_count,
                            uint16_t* buffer,
                            __attribute__ ((unused)) void* a4)
{
    /* printf("gsr timer\n"); */
    // Buffer already filled.
    // Average samples together.
    g_current_readings.gsr_result = 0;
    for (uint8_t i = 0; i < GSR_SAMPLES_LEN; i++)
        g_current_readings.gsr_result += buffer[i];
    g_current_readings.gsr_result /= GSR_SAMPLES_LEN;

    /* printf("gsr callback\n"); */
    g_gsr_status = R_DONE;

	if (++g_gsr_actions >= GSR_ACTION_LIMIT)
	{
		timer_cancel(&g_gsr_timer);
	}

	// start audio if it is needed now
	if (adc_in_conflict)
	{
		eval_check_return_code(
			adc_set_buffer(g_gsr_samples, 128),
			"biometrics: adc_set_buffer");
		adc_buffered_sample(0, 2560);
	}
	else
	{
		adc_in_use = false;
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

void accel_timer_fired(__attribute__ ((unused)) int a1,
					   __attribute__ ((unused)) int a2,
					   __attribute__ ((unused)) int a3,
					   __attribute__ ((unused)) void* a4)
{
	int x, y, z;
	ninedof_read_acceleration_sync(&x, &y, &z);

	return;
}

// ---------v

void audio_buffer_filled(__attribute__ ((unused)) uint8_t channel_no,
                         __attribute__ ((unused)) uint32_t sample_count,
                         __attribute__ ((unused)) uint16_t* buffer,
                         __attribute__ ((unused)) void* a4)
{
    audio_pending = true;

	// start audio if it is needed now
	if (adc_in_conflict)
	{
		eval_check_return_code(
			adc_set_buffer(g_gsr_samples, GSR_SAMPLES_LEN),
			"biometrics: adc_set_buffer");
		eval_check_return_code(
			adc_set_buffered_sample_callback(&gsr_sampling_completed, NULL),
			"biometrics: adc_set_buffered_sample_callbck");
		adc_buffered_sample(GSR_SAMPLING_CHANNEL, GSR_SAMPLING_FREQ);
	}
	else
	{
		adc_in_use = false;
	}

    return;
}

// ---------^
