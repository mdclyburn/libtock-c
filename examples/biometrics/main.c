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

void update_activity(void);

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
uint32_t t_started = 0;
uint32_t t_mark = 0;
void update_activity(void)
{
	uint32_t t_now = (alarm_read() / 16000) - t_started;

	if (t_mark < 3130 && t_now < 3130) { // sitting
		t_mark = 3130;
		/* printf("updated 3130\n"); */
		/* delay_ms(500); */
		/* printf("updated 3130\n"); */
		timer_cancel(&g_gsr_timer);
		timer_every(60000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 3430 && t_now < 3430) { // chores
		t_mark = 3550;
		timer_cancel(&g_gsr_timer);
		timer_every(10000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 3550 && t_now < 3550) { // sitting
		t_mark = 4470;
		timer_cancel(&g_gsr_timer);
		timer_every(60000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 4470 && t_now < 4470) { // walking
		t_mark = 6620;
		timer_cancel(&g_gsr_timer);
		timer_every(10000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 6620 && t_now < 6620) { // eating
		t_mark = 6800;
		timer_cancel(&g_gsr_timer);
		timer_every(10000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 6800 && t_now < 6800) { // walking
		t_mark = 7150;
		timer_cancel(&g_gsr_timer);
		timer_every(10000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 7150 && t_now < 7150) { // chores
		t_mark = 7740;
		timer_cancel(&g_gsr_timer);
		timer_every(10000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 7740 && t_now < 7740) { // walking
		t_mark = 7810;
		timer_cancel(&g_gsr_timer);
		timer_every(10000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 7810 && t_now < 7810) { // chores
		t_mark = 14640;
		timer_cancel(&g_gsr_timer);
		timer_every(10000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 14640 && t_now < 14640) { // sitting
		t_mark = 14640;
		timer_cancel(&g_gsr_timer);
		timer_every(60000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);
	} else if (t_mark < 18000 && t_now < 18000) { // sleeping
		t_mark = 2222222;
		timer_cancel(&g_gsr_timer);
		timer_every(10000,
					gsr_timer_fired,
					NULL,
					&g_gsr_timer);

		timer_cancel(&g_temperature_timer);
		timer_every(60000,
					temperature_timer_fired,
					NULL,
					&g_temperature_timer);
	} else {
		/* printf("no change\n"); */
	}

	return;
}

int main(void)
{
	t_started = alarm_read() / 16000;

    /* eval_setup(); */

	int temperature;
	while (1) {
		int rc = temperature_read_sync(&temperature);
		delay_ms(TEMPERATURE_SAMPLING_PERIOD_MS);
	}

    /* eval_check_return_code( */
    /*     temperature_set_callback(temperature_reading_completed, NULL), */
    /*     "biometrics: temperature_set_callback"); */
    /* eval_check_return_code( */
    /*     adc_set_buffer(g_gsr_samples, GSR_SAMPLES_LEN), */
    /*     "biometrics: adc_set_buffer"); */
    /* eval_check_return_code( */
    /*     adc_set_buffered_sample_callback(&gsr_sampling_completed, NULL), */
    /*     "biometrics: adc_set_buffered_sample_callbck"); */

    /* timer_every(TEMPERATURE_SAMPLING_PERIOD_MS, */
    /*             temperature_timer_fired, */
    /*             NULL, */
    /*             &g_temperature_timer); */

    /* timer_every(GSR_SAMPLING_PERIOD_MS, */
    /*             gsr_timer_fired, */
    /*             NULL, */
    /*             &g_gsr_timer); */

    /* uint16_t reading_i = 0; */
    /* g_temperature_status = R_EMPTY; */
    /* g_gsr_status = R_EMPTY; */
    /* while (1) */
    /* { */
    /*     g_temperature_status = R_EMPTY; */
    /*     g_gsr_status = R_EMPTY; */

    /*     while (!is_ready()) { yield(); } */

    /*     // Pack the data for encryption and later sending. */
    /*     g_readings[reading_i].temperature = g_current_readings.temperature; */
    /*     g_readings[reading_i].gsr = g_current_readings.gsr_result; */

    /*     // We encrypt the data once we have reached a certain amount of data. */
    /*     reading_i = (reading_i + 1) % READINGS_BUFFER_LEN; */
    /*     if (reading_i == 0) */
    /*     { */
    /*         /\* printf("encrypting\n"); *\/ */
    /*         aes_do_something(); */
    /*     } */
    /* } */

    /* timer_cancel(&g_temperature_timer); */
    /* timer_cancel(&g_gsr_timer); */

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
    /* if (g_temperature_status != R_EMPTY) */
    /*     return; */

    /* g_temperature_status = R_PENDING; */
    /* eval_check_return_code(temperature_read(), */
                           /* "biometrics: temperature_read"); */
	/* int temp; */
	/* temperature_read_sync(&temp); */

    return;
}

#define GSR_SAMPLING_CHANNEL 1
#define GSR_SAMPLING_FREQ 4

void gsr_timer_fired(__attribute__ ((unused)) int a1,
                     __attribute__ ((unused)) int a2,
                     __attribute__ ((unused)) int a3,
                     __attribute__ ((unused)) void* a4)
{
    if (g_gsr_status != R_EMPTY)
        return;

    while (true)
    {
        int rc = adc_buffered_sample(GSR_SAMPLING_CHANNEL, GSR_SAMPLING_FREQ);
        if (rc == 0)
        {
            g_gsr_status = R_PENDING;
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
	/* update_activity(); */
    g_current_readings.temperature = temperature;
    /* printf("temperature callback\n"); */
    g_temperature_status = R_DONE;

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

    return;
}
