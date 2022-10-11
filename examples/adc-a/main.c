#include <adc.h>
#include <stdio.h>
#include <stdint.h>
#include <timer.h>

uint16_t samples[256];
uint16_t samples2[256];

void adc_callback(uint8_t channel, uint16_t len, void* _0)
{
    printf("adc-a: callback done...\n");
}

int main(void) {
    adc_set_callback(adc_callback, NULL);
    adc_set_buffer(samples, 256);
    adc_buffered_sample(0, 128);
    /* adc_set_continuous_sample_callback(adc_callback, NULL); */
    /* adc_set_buffer(samples, 256); */
    /* adc_continuous_sample(0, 128); */

    delay_ms(500);

    /* int err = adc_sample_buffer_sync(1, 128, samples2, 256); */
    /* if (err != TOCK_STATUSCODE_SUCCESS) { */
    /*     printf ("Channel %d: error(%i) %s \n", 0, err, tock_strrcode(err)); */
    /* } */

    printf("adc-a: sampling sync...\n");

    adc_single_sample(1);

    printf("adc-a: done...\n");

    delay_ms(1000);

    while(true) {  }
}
