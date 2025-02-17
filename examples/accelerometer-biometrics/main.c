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

#define TEMPERATURE_SAMPLING_PERIOD_MS 2000
#define GSR_SAMPLING_PERIOD_MS (TEMPERATURE_SAMPLING_PERIOD_MS) * 2

#define TEMP_ACTION_LIMIT 9
#define GSR_ACTION_LIMIT 4

int g_t_actions = 0;
int g_gsr_actions = 0;

tock_timer_t g_temperature_timer;
tock_timer_t g_gsr_timer;
tock_timer_t g_acl_timer;

#define GSR_SAMPLES_LEN 10
uint16_t g_gsr_samples[GSR_SAMPLES_LEN];

uint32_t g_rng = 0;

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
void acl_timer_fired(int, int, int, void*);

void gsr_sampling_completed(uint8_t, uint32_t, uint16_t*, void*);
void temperature_reading_completed(int, int, int, void*);

bool is_ready(void);

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

    timer_every(TEMPERATURE_SAMPLING_PERIOD_MS,
                temperature_timer_fired,
                NULL,
                &g_temperature_timer);

    timer_every(GSR_SAMPLING_PERIOD_MS,
                gsr_timer_fired,
                NULL,
                &g_gsr_timer);

	int bytes_rcv;
	rng_sync((uint8_t*) &g_rng, 4, 4, &bytes_rcv);
	g_rng = (g_rng * 37829 + 41) % 999999;
	timer_in(776 + (g_rng % 100),
			 acl_timer_fired,
			 NULL,
			 &g_acl_timer);
	ninedof_avm_subscribe(true);

    uint16_t reading_i = 0;
    g_temperature_status = R_EMPTY;
    g_gsr_status = R_EMPTY;
    while (g_gsr_actions <= GSR_ACTION_LIMIT && g_t_actions <= TEMP_ACTION_LIMIT)
    {
        g_temperature_status = R_EMPTY;
        g_gsr_status = R_EMPTY;

        while (!is_ready()) { yield(); }

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

void acl_timer_fired(__attribute__ ((unused)) int a1,
                     __attribute__ ((unused)) int a2,
                     __attribute__ ((unused)) int a3,
                     __attribute__ ((unused)) void* a4)
{

	g_rng = (g_rng * 37829 + 41) % 999999;
	timer_in(776 + (g_rng % 100),
			 acl_timer_fired,
			 NULL,
			 &g_acl_timer);

	int x, y, z;
	ninedof_read_acceleration_sync(&x, &y, &z);
	for (uint32_t i = 0; i < g_rng % 5000000; i++)
	{
		g_rng = (g_rng * 37829 + 41) % 999999;
	}

    return;
}

#define GSR_SAMPLING_CHANNEL 1
#define GSR_SAMPLING_FREQ (25 * 2 * 2 * 2)

void gsr_timer_fired(__attribute__ ((unused)) int a1,
                     __attribute__ ((unused)) int a2,
                     __attribute__ ((unused)) int a3,
                     __attribute__ ((unused)) void* a4)
{
    if (g_gsr_status != R_EMPTY)
	{
        return;
	}

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

    return;
}
