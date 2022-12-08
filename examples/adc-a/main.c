#include <adc.h>
#include <eacc.h>
#include <stdio.h>
#include <stdint.h>
#include <timer.h>

uint16_t samples[256];
uint16_t samples2[256];

uint16_t count = 0;

tock_timer_t single_sample_timer;

void adc_callback(uint8_t channel, uint16_t len, void* _0)
{
    printf("adc-a: callback done...\n");

    return;
}

void adc_callback2(uint8_t channel, uint16_t sample, void* _0)
{
    if (++count == 64) {
        adc_stop_sampling_channel(channel);
        printf("adc-a: we are done sampling...\n");
    }

    return;
}

void
timer_fired(__attribute__ ((unused)) int _a0,
            __attribute__ ((unused)) int _a1,
            __attribute__ ((unused)) int _a2,
            __attribute__ ((unused)) void* _a3)
{
    const uint64_t accounted = eacc_total_accounted();
    printf("app says accounted: %lld\n", accounted);
    return;
}

int main(void) {
    int rc;

    /* delay_ms(500); */

    /* adc_set_callback(adc_callback, NULL); */
    /* adc_set_buffer(samples, 256); */
    /* adc_buffered_sample(0, 128); */
    /* adc_set_continuous_sample_callback(adc_callback, NULL); */
    /* adc_set_buffer(samples, 256); */
    /* adc_continuous_sample(0, 128); */

    timer_every(3000, timer_fired, NULL, &single_sample_timer);

    delay_ms(100);

    /* int err = adc_sample_buffer_sync(1, 128, samples2, 256); */
    /* if (err != TOCK_STATUSCODE_SUCCESS) { */
    /*     printf ("Channel %d: error(%i) %s \n", 0, err, tock_strrcode(err)); */
    /* } */

    /* printf("adc-a: sampling sync...\n"); */

    /* adc_single_sample(1); */
    printf("adc-a: configuring continuous callback... ");
    rc = adc_set_continuous_sample_callback(adc_callback2, NULL);
    printf("%i\n", rc);

    printf("adc-a: starting continuous sampling... ");
    rc = adc_continuous_sample(1, 2);
    printf("%i\n", rc);

    printf("adc-a: done...\n");

    delay_ms(1000);

    while(true) { yield(); }
}
