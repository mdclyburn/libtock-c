#include <stdint.h>

#include "tock.h"

#define DRIVER_NUM_DMA 0x20007

int dma_copy(uint32_t* const src_buffer,
             uint32_t* dst_buffer);

int dma_stop(void);

int dma_status(uint32_t* status);

int dma_power_on(uint8_t);
int dma_power_off(uint8_t);
