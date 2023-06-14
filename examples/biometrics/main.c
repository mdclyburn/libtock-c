/** biometrics - Periodically sense "biometric" data and encrypt it for eventual sending.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <aes.h>
#include <humidity.h>
#include <temperature.h>
#include <timer.h>

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

int main(void)
{
    humidity_set_callback(humidity_reading_completed, NULL);
    temperature_set_callback(temperature_reading_completed, NULL);

    while (true)
    {
        readings.count = 0;
        timer_in(5000, sensing_timer_fired, NULL, &sensing_timer);

        // Wait until we have all data available.
        while (readings.count < 2) { yield(); }

        // Pack the data for encryption and later sending.
        // We encrypt the data once we have reached a certain amount of data.
        aes_do_something();
    }
}

void check_return_code(const int rc)
{
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("biometrics: non-zero return code\n");
        while (true);
    }

    return;
}

void sensing_timer_fired(__attribute__ ((unused)) int a1,
                         __attribute__ ((unused)) int a2,
                         __attribute__ ((unused)) int a3,
                         __attribute__ ((unused)) void* a4)
{
    check_return_code(temperature_read());
    check_return_code(humidity_read());

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
