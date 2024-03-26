#include <adc.h>
#include <eacc.h>
#include <stdio.h>
#include <stdint.h>
#include <timer.h>

int ChannelCount;
#define AppCount 3

struct app_info_t
{
    uint8_t app_no;
    uint8_t channel_to_use;
    uint32_t freq;
};

uint16_t count = 0;

bool Run = true;
struct app_info_t Apps[AppCount];
tock_timer_t sampling_timers[AppCount];

void
adc_callback(uint8_t, uint16_t, void*);

void
adc_callback2(uint8_t, uint16_t, void*);

void
app_start_sampling(int, int, int, void*);

uint32_t lcg_parkmiller(uint32_t*);

uint32_t next_random(void);

void
app_start_sampling(__attribute__ ((unused)) int _a0,
                   __attribute__ ((unused)) int _a1,
                   __attribute__ ((unused)) int _a2,
                   void* a3)
{
    struct app_info_t* const app_info = (struct app_info_t*) a3;

    // Stop sampling on whatever channel this app is currently using.
    adc_stop_sampling_channel(app_info->channel_to_use);
    printf("App %d stopped sampling on channel %d.\n",
           app_info->app_no,
           app_info->channel_to_use);

    switch (app_info->app_no)
    {
    case 0:
        // 100 - 2100 S/s
        app_info->freq += 100;
        if (app_info->freq > 10000)
            app_info->freq = 10000;
        break;
    case 1:
        // 100 - 1000 S/s
        app_info->freq = ((next_random() + next_random() + next_random()) % 1150) + 300;
        break;
    case 2:
        // 100 - 1300 S/s
        app_info->freq = ((next_random() + next_random() + next_random()) % 1000) + 300;
        break;
    default:
        printf("unhandled app no. %d\n", app_info->app_no);
        return;
    }

    // Try to start sampling on the designated channel.
    uint8_t next_channel_no = app_info->channel_to_use;
    while (true)
    {
        next_channel_no = (next_channel_no + 1) % ChannelCount;
        const int rc = adc_continuous_sample(next_channel_no,
                                             app_info->freq);

        if (rc == RETURNCODE_SUCCESS)
        {
            app_info->channel_to_use = next_channel_no;
            printf("App %d started sampling channel %u at %lu s/sec.\n",
                   app_info->app_no,
                   next_channel_no,
                   app_info->freq);
            break;
        }
        else
        {
            printf("App %d just realized channel no. %d is busy.\n",
                   app_info->app_no,
                   next_channel_no);
        }
    }

    if (Run)
    {
        timer_in(100,
                 app_start_sampling,
                 (void*) app_info,
                 &sampling_timers[app_info->app_no]);
    }

    return;
}

void adc_callback(__attribute__ ((unused)) uint8_t channel,
                  __attribute__ ((unused)) uint16_t len,
                  __attribute__ ((unused)) void* _0)
{
    return;
}

void adc_callback2(uint8_t channel,
                   __attribute__ ((unused)) uint16_t sample,
                   __attribute__ ((unused)) void* _0)
{
    if (++count == 64) {
        adc_stop_sampling_channel(channel);
        printf("adc-a: we are done sampling...\n");
    }

    return;
}

int main(void) {
    int rc;

    adc_channel_count(&ChannelCount);
    /* printf("There are %d channels available.\n", ChannelCount); */

    // Set up the applications' info.
    /* printf("Setting up application information.\n"); */
    for (uint8_t i = 0; i < 3; i++)
    {
        struct app_info_t* const a = &Apps[i];
        a->app_no = i;
        a->channel_to_use = i % ChannelCount;
        a->freq = 10;
    }

    printf("Setting callback... ");
    rc = adc_set_continuous_sample_callback(adc_callback, NULL);
    printf(" return code %i\n", rc);

    // Set up timers to fire for "applications".
    /* timer_in(1333, app_start_sampling, &Apps[0], &sampling_timers[0]); */
    /* timer_in(3255, app_start_sampling, &Apps[1], &sampling_timers[1]); */
    /* timer_every(5070, app_start_sampling, &Apps[2], &sampling_timers[2]); */

    delay_ms(1000);
    uint16_t level = 0;

    while (1) {
        rc = adc_continuous_sample(0, 300);
        delay_ms(10);
        adc_stop_sampling_channel(0);
        delay_ms(250);
    }

    delay_ms(300 * 1000);
    Run = false;

    // Stop periodic timers.
    /* for (uint8_t i = 0; i < 3; i++) */
    /* { */
    /*     timer_cancel(&sampling_timers[i]); */
    /* } */

    for (uint8_t i = 0; i < ChannelCount; i++)
    {
        adc_stop_sampling_channel(Apps[i].channel_to_use);
    }

    /* const uint64_t e = eacc_total_accounted(); */
    /* printf("Accounted: %lu\n", (uint32_t) e); */

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

uint32_t __random_seed = 0x28d7dda8;

uint32_t next_random(void) {
    return lcg_parkmiller(&__random_seed);
}
