/** biometrics - Periodically sense "biometric" data and encrypt it for eventual sending.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <aes.h>
#include <humidity.h>
#include <temperature.h>
#include <timer.h>

#define SAMPLING_PERIOD_MS 2000
#define ACTION_LIMIT 1000

int actions = 0;
tock_timer_t sensing_timer;

struct
{
    uint8_t count;
    int temperature;
    int humidity;
} readings;

void check_return_code(int);

void sensing_timer_fired(int, int, int, void*);

void humidity_reading_completed(int, int, int, void*);
void temperature_reading_completed(int, int, int, void*);

uint32_t next_random(void);
uint32_t lcg_parkmiller(uint32_t* state);

int main(void)
{
    /* printf("i"); */
    humidity_set_callback(humidity_reading_completed, NULL);
    temperature_set_callback(temperature_reading_completed, NULL);

    /* int count = 0; */
    /* uint32_t sampling_period_ms = 2000 - 375 + (next_random() % 750); */
    timer_every(SAMPLING_PERIOD_MS,
                sensing_timer_fired,
                NULL,
                &sensing_timer);

    /* while (actions <= ACTION_LIMIT) { yield(); } */
    /* while (actions++ <= ACTION_LIMIT) */
    while (true)
    {
        readings.count = 0;

        /* timer_in(SAMPLING_PERIOD_MS - 375 + (next_random() % 750), */
        /*          sensing_timer_fired, */
        /*          NULL, */
        /*          &sensing_timer); */

        // Wait until we have all data available.
        /* printf("waiting for readings...\n"); */
        while (readings.count < 1) { yield(); }

        // Pack the data for encryption and later sending.
        // We encrypt the data once we have reached a certain amount of data.
        /* printf("encrypting\n"); */
        aes_do_something();
    }

    timer_cancel(&sensing_timer);

    while (true) { yield(); }

    return 0;
}

void check_return_code(const int rc)
{
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("biometrics: non-zero return code\n");
        while (true) { yield(); }
    }

    return;
}

void sensing_timer_fired(__attribute__ ((unused)) int a1,
                         __attribute__ ((unused)) int a2,
                         __attribute__ ((unused)) int a3,
                         __attribute__ ((unused)) void* a4)
{
    actions++;
    /* if (temperature_read() != RETURNCODE_SUCCESS) */
    /* { */
    /*     /\* printf("temp read failed\n"); *\/ */
    /* } */

    if (humidity_read() != RETURNCODE_SUCCESS)
    {
        /* printf("hum read failed\n"); */
    }

    return;
}

void humidity_reading_completed(int humidity,
                                __attribute__ ((unused)) int a2,
                                __attribute__ ((unused)) int a3,
                                __attribute__ ((unused)) void* a4)
{
    readings.humidity = humidity;
    readings.count++;

    return;
}

void temperature_reading_completed(int temperature,
                                   __attribute__ ((unused)) int a2,
                                   __attribute__ ((unused)) int a3,
                                   __attribute__ ((unused)) void* a4)
{
    readings.temperature = temperature;
    readings.count++;

    return;
}

/* uint32_t lcg_parkmiller(uint32_t *state) */
/* { */
/*     const uint32_t N = 0x7fffffff; */
/*     const uint32_t G = 48271u; */

/*     uint32_t div = *state / (N / G);  /\* max : 2,147,483,646 / 44,488 = 48,271 *\/ */
/*     uint32_t rem = *state % (N / G);  /\* max : 2,147,483,646 % 44,488 = 44,487 *\/ */

/*     uint32_t a = rem * G;        /\* max : 44,487 * 48,271 = 2,147,431,977 *\/ */
/*     uint32_t b = div * (N % G);  /\* max : 48,271 * 3,399 = 164,073,129 *\/ */

/*     return *state = (a > b) ? (a - b) : (a + (N - b)); */
/* } */

/* uint32_t __random_seed = 0x28d7dda8; */

/* uint32_t next_random(void) { */
/*     return lcg_parkmiller(&__random_seed); */
/* } */
