#include <stdio.h>
#include <stdint.h>

#include <adc.h>
#include <audio.h>
#include <crc.h>
#include <dma.h>
#include <humidity.h>
#include <rfm69.h>
#include <temperature.h>
#include <timer.h>

#define BUFFER_LEN 2048

uint32_t buffer_a[BUFFER_LEN];
uint32_t buffer_b[BUFFER_LEN];
uint16_t sample_buffer[64];
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

void
adc_sampling_completed(uint8_t, uint16_t, void*);
void
adc_buffered_sampling_completed(uint8_t, uint32_t, uint16_t*, void*);

void
throwaway_callback(int, int, int, void*);

int main(void) {
    uint32_t mpr = 738;
    for (uint32_t i = 0; i < BUFFER_LEN; i++) {
        buffer_a[i] = mpr;
        buffer_b[i] = mpr + 1;
        mpr = (mpr * 11) + 38 % 9997;
    }

    delay_ms(500);

    adc_set_buffer(sample_buffer, 5);
    adc_set_buffered_sample_callback(adc_buffered_sampling_completed, NULL);
    adc_set_continuous_sample_callback(adc_sampling_completed, NULL);
    humidity_set_callback(throwaway_callback, NULL);
    temperature_set_callback(throwaway_callback, NULL);
    /* spi_set_master_read_buffer(buffer_spi_r, 256); */
    /* spi_set_master_write_buffer(buffer_spi_w, 256); */
    /* spi_init(); */
    /* spi_set_rate(2000); */

    hail_exp_01();

    /* printf("done\n"); */
    /* delay_ms(100); */
    /* dma_power_off(1); */
    /* dma_power_off(2); */

    while (true) { delay_ms(5000); }
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
    for (uint16_t i = 0; i < 100; i++) // about 250ms to run the loop.
    {
        // ADC
        /* adc_continuous_sample(0, 30); */
        /* timer_in(50, timer_fired, NULL, &adc_timer); */
        /* delay_ms(50); */
        /* delay_ms(40); */
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
        adc_buffered_sample(0, 650);
        /* timer_in(50, timer_fired, NULL, &adc_timer); */
        aes_do_something();
        printf("some characters some characters some characters some characters some characters\n");
        humidity_read();
        temperature_read();

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

void
timer_fired(__attribute__ ((unused)) int _a0,
            __attribute__ ((unused)) int _a1,
            __attribute__ ((unused)) int _a2,
            __attribute__ ((unused)) void* _a3)
{
    adc_stop_sampling();
    return;
}

void
adc_sampling_completed(__attribute__ ((unused)) uint8_t channel_no,
                       __attribute__ ((unused)) uint16_t sample_value,
                       __attribute__ ((unused)) void* parameter)
{
    return;
}

void
adc_buffered_sampling_completed(__attribute__ ((unused)) uint8_t channel_no,
                                __attribute__ ((unused)) uint32_t sample_count,
                                __attribute__ ((unused)) uint16_t* samples,
                                __attribute__ ((unused)) void* parameter)
{
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
