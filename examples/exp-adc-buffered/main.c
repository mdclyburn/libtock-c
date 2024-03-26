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

#define SAMPLES_LEN 2000
uint16_t samples[SAMPLES_LEN];

void adc_callback(
    uint8_t channel,
    uint32_t no_samples,
    uint16_t* buffer,
    void* _none)
{
    fp_account(2, no_samples);
    count += no_samples;
    printf("received %i samples\n", no_samples);

    if (count >= 200)
    {
        count = 0;
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

    int rc;
    rc = adc_set_buffer(samples, (SAMPLES_LEN / 2));
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("failed to set first buffer (%d)\n", rc);
        while(true);
    }

    rc = adc_set_double_buffer(samples + (SAMPLES_LEN / 2), (SAMPLES_LEN / 2));
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("failed to set double buffer (%d)\n", rc);
        while(true);
    }

    rc = adc_set_buffered_sample_callback(adc_callback, NULL);
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("failed to set the buffered sample callback (%d)\n", rc);
        while(true);
    }

    rc = adc_set_continuous_buffered_sample_callback(adc_callback, NULL);
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("failed to set the continuous buffered sample callback (%d)\n", rc);
        while(true);
    }

    rc = adc_continuous_buffered_sample(0, 1000);
    if (rc != RETURNCODE_SUCCESS)
    {
        printf("failed start sampling (%d)\n", rc);
        while(true);
    }

    // Hang.
    while (true) {
        yield();
    }
}
