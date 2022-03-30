#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <adc.h>
#include <dac.h>
#include <performance.h>
#include <tock.h>

performance_fp_t fp_account = NULL;
performance_freeze_fp_t fp_freeze = NULL;
uint32_t count = 0;

uint32_t nlim = 0;
uint32_t limit[] = { 100, 500, 750, 1500 };

void adc_callback(uint8_t channel, uint16_t value, void* _none)
{
    fp_account(2, 1);
    // 12- to 10-bit.
    dac_set_value(value >> 2);
    count++;

    if (count >= 200)
    {
        count = 0;
        /* nlim = (nlim + 1) % 3; */
        fp_freeze();
    }
}

void configure()
{
    int rc = performance_available();
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("performance not available: %i\n", rc);
        while(true);
    }

    fp_account = performance_call_fp();
    if (fp_account == NULL)
    {
        printf("performance call not available\n");
        while(true);
    }

    fp_freeze = performance_freeze_fp();
    if(fp_freeze == NULL)
    {
        printf("performance freeze not available\n");
        while(true);
    }

    rc = dac_initialize();
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("DAC is not available\n");
        while(true);
    }

    return;
}

int main(void)
{
    // Check for the performance-tracking capsule.
    configure();

    adc_set_continuous_sample_callback(&adc_callback, NULL);
    const int rc = adc_continuous_sample(0, 10000);

    // Hang.
    while (true) {
        yield();
    }
}
