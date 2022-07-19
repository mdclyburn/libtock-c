#include <stdint.h>
#include <stdio.h>

#include <led.h>
#include <rfm69.h>
#include <timer.h>
#include <tock.h>

#define BUFFER_LENGTH 66

void fast_blink(void);
void configure_radio(void);

uint8_t RXTXBuffer[BUFFER_LENGTH];

int main(void) {
    fast_blink();
    configure_radio();

    uint8_t i = 0;
    while (true) {
        RXTXBuffer[0] = 7;
        RXTXBuffer[1] = 44;
        RXTXBuffer[2] = i++;
        int r = rfm69_transmit();
        if (r != 0) {
            printf("Failed to transmit: %i\n", r);
            // Stay here. No more attempts.
            while (true) { delay_ms(10000); }
        }

        delay_ms(6000);
    }

    /* for (uint8_t i = 0; i < BUFFER_LENGTH; i++) { RXTXBuffer[i] = 0x00; } */
    /* RXTXBuffer[0] = 0x07; */
    /* RXTXBuffer[1] = 0x44; */
    /* rfm69_transmit(); */
    /* delay_ms(3000); */
    /* while (rfm69_dump_registers(1) != 0) { delay_ms(500); } */

    while (true) { delay_ms(3000); }
}

void fast_blink(void) {
    led_on(0);
    delay_ms(50);
    led_off(0);

    return;
}

void configure_radio(void) {
    if (rfm69_exists() != 0) {
        printf("RFM69 driver not available.\n");
    }

    int rc = rfm69_use_variable_length_packets();
    if (rc != 0) {
        printf("RFM69 driver failed to set to variable length packets (%i).\n", rc);
    }

    if (rfm69_use_sync_word(7) != 0) {
        printf("RFM69 driver failed to enable sync word.\n");
    }

    if (rfm69_set_sync_word(0x00ACDCFF, 0x06050403) != 0) {
        printf("RFM69 driver failed to set sync word.\n");
    }

    if (rfm69_configure_address_bcast(15, 255) != 0) {
        printf("RFM69 driver failed to set addressing.\n");
    }

    /* const uint8_t enc_key[16] = { */
    /*     0x01, 0x6f, 0x00, 0x56, 0x45, 0x1e, 0x09, 0x86, */
    /*     0x02, 0x6f, 0x00, 0x56, 0x45, 0x1e, 0x09, 0x86 */
    /* }; */
    /* for (uint8_t i = 0; i < 16; i++) { */
    /*     if (rfm69_set_encryption_key(i, enc_key[i]) != 0) { */
    /*         printf("RFM69 driver failed to set encryption key (%i).\n", i); */
    /*     } */
    /* } */

    if (rfm69_set_tx_rx_buffer(RXTXBuffer, BUFFER_LENGTH) != 0) {
        printf("RFM69 driver failed to set RX/TX buffer.\n");
    }

    return;
}
