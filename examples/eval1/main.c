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
};

uint16_t count = 0;

struct app_info_t Apps[AppCount];
tock_timer_t sampling_timers[AppCount];

void
adc_callback(uint8_t, uint16_t, void*);

void
adc_callback2(uint8_t, uint16_t, void*);

void
app_start_sampling(int, int, int, void*);

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

    uint32_t sampling_freq;
    switch (app_info->app_no)
    {
    case 0:
        sampling_freq = 2100;
        break;
    case 1:
        sampling_freq = 1000;
        break;
    case 2:
        sampling_freq = 1300;
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
                                             sampling_freq);

        if (rc == RETURNCODE_SUCCESS)
        {
            app_info->channel_to_use = next_channel_no;
            printf("App %d started sampling channel %u at %lu s/sec.\n",
                   app_info->app_no,
                   next_channel_no,
                   sampling_freq);
            break;
        }
        else
        {
            printf("App %d just realized channel no. %d is busy.\n",
                   app_info->app_no,
                   next_channel_no);
        }
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
    printf("There are %d channels available.\n", ChannelCount);

    // Set up the applications' info.
    printf("Setting up application information.\n");
    for (uint8_t i = 0; i < 3; i++)
    {
        struct app_info_t* const a = &Apps[i];
        a->app_no = i;
        a->channel_to_use = i % ChannelCount;
    }

    printf("Setting callback. ");
    rc = adc_set_continuous_sample_callback(adc_callback, NULL);
    printf("%i\n", rc);

    // Set up timers to fire for "applications".
    printf("Configuring timers.\n");
    timer_every(3000, app_start_sampling, &Apps[0], &sampling_timers[0]);
    timer_every(5000, app_start_sampling, &Apps[1], &sampling_timers[1]);
    timer_every(7000, app_start_sampling, &Apps[2], &sampling_timers[2]);

    while(true) { yield(); }
}
