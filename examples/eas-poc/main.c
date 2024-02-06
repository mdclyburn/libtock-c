#include <stdio.h>
#include <stdint.h>

#include <adc.h>
#include <audio.h>
#include <crc.h>
#include <dma.h>
#include <gpio.h>
#include <humidity.h>
#include <led.h>
#include <rfm69.h>
#include <temperature.h>
#include <timer.h>

#define BUFFER_LEN 2048

uint32_t buffer_a[BUFFER_LEN];
uint32_t buffer_b[BUFFER_LEN];

#define SAMPLE_BUFFER_LEN 64
uint16_t sample_buffer[SAMPLE_BUFFER_LEN];
/* uint32_t buffer_c[BUFFER_LEN]; */
/* uint32_t buffer_d[BUFFER_LEN]; */
/* uint8_t buffer_spi_r[256]; */
/* uint8_t buffer_spi_w[256]; */

tock_timer_t adc_timer;

void timer_fired(int, int, int, void*);

void experiment_dma_together(void);
void experiment_dma_separate(void);

void dma_exp_00(void);
void dma_exp_01(void);
void dma_exp_02(void);

void uart_exp_00(void);

void spi_exp_00(void);

void hail_exp_00(void);
void hail_exp_01(void);
void hail_exp_02(void);
void hail_exp_03(void);
void hail_exp_04(void);
void hail_exp_05(void);
void hail_exp_06(void);
void hail_exp_07(void);
void hail_exp_08(void);
void hail_exp_08b(void);
void hail_exp_09(void);
void hail_exp_10(void);
void hail_exp_11(void);
void hail_exp_12(void);
void hail_exp_13(void);

void model_exp_00(void);
void model_exp_01(void);
void model_exp_02(void);
void model_exp_03(void);

void
adc_sampling_completed(uint8_t, uint16_t, void*);
void
adc_buffered_sampling_completed(uint8_t, uint32_t, uint16_t*, void*);
void
adc_buffered_sampling_completed_ip(uint8_t, uint32_t, uint16_t*, void*);

void
temperature_read_completed_ip(int, int, int, void*);

void
throwaway_callback(int, int, int, void*);

int main(void) {
    uint32_t mpr = 738;
    for (uint32_t i = 0; i < BUFFER_LEN; i++) {
        buffer_a[i] = mpr;
        buffer_b[i] = mpr + 1;
        mpr = (mpr * 11) + 38 % 9997;
    }

    // use UART to get it to turn off after it is done.
    /* delay_ms(50); */
    /* for (uint8_t i = 0; i < 1; i++) */
    /*     printf("jkasldjfklasdjflkasdjflkasdfjl\n"); */

    printf("i");

    adc_set_buffer(sample_buffer, SAMPLE_BUFFER_LEN);
    /* adc_set_single_sample_callback(adc_sampling_completed, NULL); */
    /* adc_set_buffered_sample_callback(adc_buffered_sampling_completed_ip, NULL); */
    adc_set_buffered_sample_callback(adc_buffered_sampling_completed, NULL);
    /* adc_set_continuous_sample_callback(adc_sampling_completed, NULL); */
    humidity_set_callback(throwaway_callback, NULL);
    temperature_set_callback(throwaway_callback, NULL);
    /* temperature_set_callback(temperature_read_completed_ip, NULL); */

    /* printf("i\n"); */
    /* adc_buffered_sample(0, 650); */
    /* aes_do_something(); */
    /* humidity_read(); */
    /* temperature_read(); */

    gpio_enable_output(0);

    delay_ms(450);

    /* spi_set_master_read_buffer(buffer_spi_r, 256); */
    /* spi_set_master_write_buffer(buffer_spi_w, 256); */
    /* spi_init(); */
    /* spi_set_rate(2000); */

    hail_exp_01();
    /* gpio_set(0); */
    /* adc_buffered_sample(0, 650); */
    while (true) { yield(); }

    /* printf("done\n"); */
    /* delay_ms(100); */
    /* dma_power_off(1); */
    /* dma_power_off(2); */

    /* model_exp_03(); */

    while (true) { delay_ms(10000); }
}

void experiment_dma_together(void)
{
    int rc;
    rc = dma_copy(buffer_a, buffer_b);
    rc = adc_start_experiment();

    delay_ms(1000);

    adc_stop_sampling_channel(0);

    dma_power_off(2);

    return;
}

void experiment_dma_separate(void)
{
    int rc;

    for (uint8_t i = 0; i < 1; i++)
    {
        rc = adc_start_experiment();
        delay_ms(200);
        adc_stop_sampling_channel(0);

        delay_ms(100);
        rc = dma_copy(buffer_a, buffer_b);
        delay_ms(300);
    }

    dma_power_off(2);

    return;
}

void dma_exp_00(void)
{
    int rc;
    while (true)
    {
        adc_start_experiment();
        timer_in(50, timer_fired, NULL, &adc_timer);
        delay_ms(100);
        audio_play();
        delay_ms(100);
        rfm69_do_something();
        delay_ms(100);
        printf("send some data over USART 1/DMA1\n");
        delay_ms(100);

        delay_ms(200);
    }

    return;
}

void dma_exp_01(void)
{
    int rc;
    for (uint8_t i = 0; i < 5; i++)
    {
        adc_start_experiment();
        audio_play();
        rfm69_do_something();
        /* printf("send some data over USART 1/DMA1\n"); */
        delay_ms(50);
        adc_stop_sampling_channel(0);

        delay_ms(200);
    }

    return;
}

void dma_exp_02(void)
{
    for (uint8_t i = 0; i < 10; i++)
    {
        rfm69_do_something();
        printf("i am using the dma to send some data over USART 1/DMA 1\n");
        /* uint32_t s; */
        /* dma_status(&s); */
        /* printf("s: %lu\n", s); */
        delay_ms(200);
    }

    return;
}

void uart_exp_00(void)
{
    for (uint8_t i = 0; i < 5; i++)
    {
        for (uint8_t j = 0; j < 1; j++)
        {
            printf("some characters some characters some characters some characters some characters\n");
        }
        delay_ms(200);
    }

    return;
}

void spi_exp_00(void)
{
    rfm69_do_something();
    while (true) { buffer_a[7] += 7; }

    return;
}

// separate
void hail_exp_00(void)
{
    int result;
    for (uint16_t i = 0; i < 1; i++) // about 250ms to run the loop.
    {
        // ADC
        adc_buffered_sample(0, 650);
        delay_ms(50);

        // AES
        aes_do_something();
        /* printf("aes: %d\n", rc); */
        delay_ms(50);

        // UART
        printf("some characters some characters some characters some characters some characters\n");
        delay_ms(50);

        // Humidity
        humidity_read();
        /* printf("humidity: %d\n", result); */
        delay_ms(50);

        // Temperature
        temperature_read();
        /* printf("temp: %d\n", result); */
        delay_ms(50);
    }

    return;
}

// batched
void hail_exp_01(void)
{
    for (uint16_t i = 0; i < 100; i++)
    {
        /* timer_in(50, timer_fired, NULL, &adc_timer); */
        humidity_read();
        temperature_read();
        aes_do_something();
        printf("s");
        adc_buffered_sample(0, 650);

        delay_ms(250);
    }

    return;
}

// adc
void hail_exp_02(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        adc_continuous_sample(0, 30);
        timer_in(50, timer_fired, NULL, &adc_timer);

        delay_ms(200);
    }

    return;
}

// uart
void hail_exp_03(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        printf("some characters some characters some characters some characters some characters\n");

        delay_ms(200);
    }

    return;
}

// adc + uart
void hail_exp_04(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        adc_continuous_sample(0, 30);
        timer_in(50, timer_fired, NULL, &adc_timer);
        printf("some characters some characters some characters some characters some characters\n");

        delay_ms(200);
    }

    return;
}

void hail_exp_05(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        humidity_read();

        delay_ms(200);
    }

    return;
}

void hail_exp_06(void)
{
    /* int rc1, rc2; */
    for (uint8_t i = 0; i < 100; i++)
    {
        humidity_read();
        temperature_read();
        /* printf("rc1 = %d, rc2 = %d\n", rc1, rc2); */

        delay_ms(200);
    }

    return;
}

void
hail_exp_07(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        aes_do_something();
        delay_ms(200);
    }

    return;
}

// ADC + AES, UART + SENSE
void
hail_exp_08(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        adc_buffered_sample(0, 650);
        aes_do_something();
        delay_ms(125);

        printf("some characters some characters some characters some characters some characters\n");
        humidity_read();
        temperature_read();
        delay_ms(125);
    }
}

// ADC + AES, UART + SENSE
void
hail_exp_08b(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        adc_buffered_sample(0, 650);
        aes_do_something();
        delay_ms(125);

        printf("some characters some characters some characters some characters some characters\n");
        humidity_read();
        temperature_read();
        delay_ms(125);
    }
}

// ADC + SENSE, UART + ENCRYPT
void
hail_exp_09(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        aes_do_something();
        printf("some characters some characters some characters some characters some characters\n");
        delay_ms(125);

        humidity_read();
        temperature_read();
        adc_buffered_sample(0, 650);
        /* timer_in(50, timer_fired, NULL, &adc_timer); */
        delay_ms(125);
    }
}

// ADC + UART + ENCRYPT, SENSE
void
hail_exp_10(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        adc_buffered_sample(0, 650);
        /* timer_in(50, timer_fired, NULL, &adc_timer); */
        aes_do_something();
        printf("some characters some characters some characters some characters some characters\n");
        delay_ms(125);

        humidity_read();
        temperature_read();
        delay_ms(125);
    }
}

// ADC + UART + SENSE, ENCRYPT
void
hail_exp_11(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        adc_buffered_sample(0, 650);
        /* timer_in(50, timer_fired, NULL, &adc_timer); */
        humidity_read();
        temperature_read();
        printf("some characters some characters some characters some characters some characters\n");
        delay_ms(125);

        aes_do_something();
        delay_ms(125);
    }
}

// UART + SENSE + ENCRYPT, ADC
void
hail_exp_12(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        aes_do_something();
        printf("some characters some characters some characters some characters some characters\n");
        humidity_read();
        temperature_read();
        delay_ms(125);

        adc_buffered_sample(0, 650);
        delay_ms(125);
    }
}

// ADC + UART, SENSE + ENCRYPT
void
hail_exp_13(void)
{
    for (uint8_t i = 0; i < 100; i++)
    {
        adc_buffered_sample(0, 650);
        printf("some characters some characters some characters some characters some characters\n");
        delay_ms(125);

        aes_do_something();
        humidity_read();
        temperature_read();
        /* timer_in(50, timer_fired, NULL, &adc_timer); */
        delay_ms(125);
    }
}

void
model_exp_00(void)
{
    delay_ms(500);
    adc_buffered_sample(0, 650);

    return;
}

void
model_exp_01(void)
{
    delay_ms(500);
    humidity_read();

    return;
}

void
model_exp_02(void)
{
    for (uint16_t i = 0; i < 5; i++)
    {
        delay_ms(500);
        printf("sjkaskdfjaksdjlfaksjdflkasjdlkfajsljdkfjasldfjasjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj");
    }

    return;
}

void
model_exp_03(void)
{
    delay_ms(500);
    aes_do_something();

    return;
}

// Batched upcalls.
void
hail_exp_ip(void)
{
    for (uint8_t i = 0; i < 50; i++)
    {
        adc_buffered_sample(0, 650);
        /* adc_single_sample(1); */
        /* led_toggle(0); */
        /* printf("some characters some characters some characters some characters some characters\n"); */

        delay_ms(500);

        temperature_read();

        delay_ms(500);
    }
    /* led_toggle(0); */
}

void
timer_fired(__attribute__ ((unused)) int _a0,
            __attribute__ ((unused)) int _a1,
            __attribute__ ((unused)) int _a2,
            __attribute__ ((unused)) void* _a3)
{
    adc_stop_sampling();
    return;
}

uint64_t val = 0;

void
adc_sampling_completed(__attribute__ ((unused)) uint8_t channel_no,
                       __attribute__ ((unused)) uint16_t sample_value,
                       __attribute__ ((unused)) void* parameter)
{
    val += channel_no;
    for (uint32_t i = 0; i < 1000000; i++)
    {
        if (sample_value > 65530) { printf("over\n"); }
        else { val += sample_value; }
    }
    /* temperature_read_completed_ip(1, 0, 0, NULL); */
    printf("adc: %llu\n", val);

    return;
}

void
adc_buffered_sampling_completed(__attribute__ ((unused)) uint8_t channel_no,
                                __attribute__ ((unused)) uint32_t sample_count,
                                __attribute__ ((unused)) uint16_t* samples,
                                __attribute__ ((unused)) void* parameter)
{
    gpio_clear(0);
    return;
}

void
adc_buffered_sampling_completed_ip(__attribute__ ((unused)) uint8_t channel_no,
                                   __attribute__ ((unused)) uint32_t sample_count,
                                   __attribute__ ((unused)) uint16_t* samples,
                                   __attribute__ ((unused)) void* parameter)
{
    /* led_toggle(0); */
    if (parameter != NULL) { return; }
    for (uint32_t i = 0; i < 300000; i++)
    {
        if (sample_count > 65530) { printf("over\n"); }
        else { val += samples[i % sample_count]; }
    }
    printf("adc: %llu\n", val);

    return;
}

uint64_t last_temperature = 0;

void
temperature_read_completed_ip(int temperature,
                              __attribute__ ((unused)) int a2,
                              __attribute__ ((unused)) int a3,
                              __attribute__ ((unused)) void* a4)
{
    last_temperature += temperature;
    for (uint32_t i = 0; i < 1000000; i++)
    {
        last_temperature += temperature + i;
    }
    printf("temp: %llu\n", last_temperature);

    return;
}

void
throwaway_callback(__attribute__ ((unused)) int a1,
                   __attribute__ ((unused)) int a2,
                   __attribute__ ((unused)) int a3,
                   __attribute__ ((unused)) void* a4)
{
    return;
}
