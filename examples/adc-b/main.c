#include <adc.h>
#include <stdio.h>
#include <stdint.h>
#include <timer.h>

uint16_t samples[256];

int main(void) {
    printf("adc-b: sampling...\n");
    int err = adc_sample_buffer_sync(1, 128, samples, 256);
    if (err != TOCK_STATUSCODE_SUCCESS) {
        printf ("Channel %d: error(%i) %s \n", 2, err, tock_strrcode(err));
    }
    printf("adc-b: done...\n");

    while (true) {  }
}
