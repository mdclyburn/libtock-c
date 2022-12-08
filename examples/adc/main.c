#include <adc.h>
#include <stdio.h>
#include <timer.h>

bool time_to_sample = false;
tock_timer_t single_sample_timer;

void
my_sample_callback(uint8_t, uint16_t, void*);

void
my_continuous_callback(uint8_t, uint16_t, void*);

void
timer_fired(int, int, int, void*);

int main(void) {
    // Ask the kernel how many ADC channels are on this board.
    int num_adc;
    int err = adc_channel_count(&num_adc);
    if (err < 0) {
        printf("No ADC on this board.\n");
        return err;
    }

    printf ("ADC Channels: %d\n", num_adc);

    adc_set_single_sample_callback(my_sample_callback, NULL);
    adc_set_continuous_sample_callback(my_continuous_callback, NULL);

    /* delay_ms(500); */
    /* adc_continuous_sample(1, 2400); */
    /* delay_ms(1000); */
    /* adc_continuous_sample(2, 5000); */
    /* adc_continuous_sample(2, 1); */
    /* adc_continuous_sample(1, 2); */

    timer_every(3000, timer_fired, NULL, &single_sample_timer);

    /* while (true) { delay_ms(50); } */

    while (true)
    {
        /* single_sample_done = false; */
        /* adc_single_sample(0); */
        /* yield_for(&single_sample_done); */
        /* delay_ms(1000); */
        yield();
    }
}

void
my_sample_callback(
    uint8_t channel,
    __attribute__ ((unused)) uint16_t sample_value,
    __attribute__ ((unused)) void* param)
{
    /* printf("received single sample from channel no. %i\n", channel); */
    return;
}

void
my_continuous_callback(
    uint8_t channel,
    __attribute__ ((unused)) uint16_t sample_value,
    __attribute__ ((unused)) void* param)
{
    /* printf("received cont. sample from channel no. %i\n", channel); */
    return;
}

void
timer_fired(__attribute__ ((unused)) int _a0,
            __attribute__ ((unused)) int _a1,
            __attribute__ ((unused)) int _a2,
            __attribute__ ((unused)) void* _a3)
{
    adc_single_sample(0);
    return;
}
