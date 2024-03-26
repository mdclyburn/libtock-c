#include <adc.h>
#include <eacc.h>
#include <stdio.h>
#include <stdint.h>
#include <timer.h>

#define EVALUATION_DURATION_MS (30 * 1000)

int ChannelCount;
#define AppCount 3

struct app_info_t
{
    uint8_t app_no;
    uint8_t channel_to_use;
};

uint16_t count = 0;

bool Run = true;
struct app_info_t Apps[AppCount];
tock_timer_t sampling_timers[AppCount];

#define DATA_POINTS (30 * 4)
uint32_t filled = 1;
uint32_t Timeline[DATA_POINTS];
tock_timer_t recording_timer;

void
adc_callback(uint8_t, uint16_t, void*);

void
app_start_sampling(int, int, int, void*);

void
record_accounting(int, int, int, void*);

uint32_t lcg_parkmiller(uint32_t*);

uint32_t next_random(void);

void
record_accounting(__attribute__ ((unused)) int _a0,
                  __attribute__ ((unused)) int _a1,
                  __attribute__ ((unused)) int _a2,
                  __attribute__ ((unused)) void* a3)
{
    if (filled < DATA_POINTS)
    {
        Timeline[filled++] = eacc_total_accounted();
    }
    else
    {
        Run = false;
    }

    return;
}

void adc_callback(__attribute__ ((unused)) uint8_t channel,
                  __attribute__ ((unused)) uint16_t len,
                  __attribute__ ((unused)) void* _0)
{
    return;
}

int main(void) {
    adc_channel_count(&ChannelCount);

    adc_set_continuous_sample_callback(adc_callback, NULL);

    // Set up timer for periodic recording of accounting total.
    Timeline[0] = 0;
    timer_every(EVALUATION_DURATION_MS / DATA_POINTS,
                record_accounting,
                NULL,
                &recording_timer);

    delay_ms(1000);

    // Start the first usage.
    int rc = adc_continuous_sample(0, 1400);
    adc_continuous_sample(1, 1400);
    delay_ms(30 * 1000);
    adc_stop_sampling_channel(0);
    adc_stop_sampling_channel(1);

    // Show data points, formatted.
    printf("time,accounted\n");
    for (uint32_t i = 1; i < DATA_POINTS; i++)
    {
        const uint32_t time = i * (EVALUATION_DURATION_MS / DATA_POINTS);
        printf("%lu,%lu\n", time, Timeline[i]);
    }

    while(true) { yield(); }
}

uint32_t lcg_parkmiller(uint32_t *state)
{
    const uint32_t N = 0x7fffffff;
    const uint32_t G = 48271u;

    uint32_t div = *state / (N / G);  /* max : 2,147,483,646 / 44,488 = 48,271 */
    uint32_t rem = *state % (N / G);  /* max : 2,147,483,646 % 44,488 = 44,487 */

    uint32_t a = rem * G;        /* max : 44,487 * 48,271 = 2,147,431,977 */
    uint32_t b = div * (N % G);  /* max : 48,271 * 3,399 = 164,073,129 */

    return *state = (a > b) ? (a - b) : (a + (N - b));
}

uint32_t __random_seed = 0x286482;

uint32_t next_random(void) {
    return lcg_parkmiller(&__random_seed);
}
