/** biometrics - Periodically sense temperature and EDA and encrypt it for sending.
 */

#include <stdint.h>
#include <stdio.h>

#include <adc.h>
#include <aes.h>
#include <evaluation.h>
#include <humidity.h>
#include <temperature.h>
#include <timer.h>

#define TEMPERATURE_SAMPLING_PERIOD_MS 2000
#define GSR_SAMPLING_PERIOD_MS (TEMPERATURE_SAMPLING_PERIOD_MS) * 2

#define ACTION_LIMIT 1000

int g_actions = 0;

tock_timer_t g_temperature_timer;
tock_timer_t g_gsr_timer;

#define GSR_SAMPLES_LEN 10
uint16_t g_gsr_samples[GSR_SAMPLES_LEN];

struct
{
    uint8_t count;
    int temperature;
    uint16_t gsr_result;
} g_current_readings;

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

    uint16_t reading_i = 0;
    while (g_actions++ <= ACTION_LIMIT)
    {
        g_current_readings.count = 0;

        // Wait until we have all data available.
        while (g_current_readings.count < 2) { yield(); }

        // Pack the data for encryption and later sending.
        g_readings[reading_i].temperature = g_current_readings.temperature;
        g_readings[reading_i].gsr = g_current_readings.gsr_result;

        // We encrypt the data once we have reached a certain amount of data.
        reading_i = (reading_i + 1) % READINGS_BUFFER_LEN;
        if (reading_i == 0)
        {
            aes_do_something();
        }
    }

    timer_cancel(&g_temperature_timer);
    timer_cancel(&g_gsr_timer);

    while (true) { yield(); }

    return 0;
}

void temperature_timer_fired(__attribute__ ((unused)) int a1,
                         __attribute__ ((unused)) int a2,
                         __attribute__ ((unused)) int a3,
                         __attribute__ ((unused)) void* a4)
{
    eval_check_return_code(temperature_read(),
                           "biometrics: temperature_read");

    return;
}

#define GSR_SAMPLING_CHANNEL 1
#define GSR_SAMPLING_FREQ 4

void gsr_timer_fired(__attribute__ ((unused)) int a1,
                     __attribute__ ((unused)) int a2,
                     __attribute__ ((unused)) int a3,
                     __attribute__ ((unused)) void* a4)
{
    while (true)
    {
        int rc = adc_buffered_sample(GSR_SAMPLING_CHANNEL, GSR_SAMPLING_FREQ);
        if (rc == 0)
        {
            break;
        }

        // Try again soon.
        delay_ms(100);
    }

    return;
}

void temperature_reading_completed(int temperature,
                                   __attribute__ ((unused)) int a2,
                                   __attribute__ ((unused)) int a3,
                                   __attribute__ ((unused)) void* a4)
{
    g_current_readings.temperature = temperature;
    g_current_readings.count++;

    return;
}

void gsr_sampling_completed(__attribute__ ((unused)) uint8_t channel_no,
                            __attribute__ ((unused)) uint32_t sample_count,
                            uint16_t* buffer,
                            __attribute__ ((unused)) void* a4)
{
    // Buffer already filled.
    // Average samples together.
    g_current_readings.gsr_result = 0;
    for (uint8_t i = 0; i < GSR_SAMPLES_LEN; i++)
        g_current_readings.gsr_result += buffer[i];
    g_current_readings.gsr_result /= GSR_SAMPLES_LEN;

    g_current_readings.count++;

    return;
}
