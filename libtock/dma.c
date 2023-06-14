#include "dma.h"

#define COMMAND_COPY_MEMORY_TO_MEMORY 10
#define COMMAND_STOP 20

int dma_copy(uint32_t* const src_buffer,
             uint32_t* dst_buffer)
{
    syscall_return_t res = command(
        DRIVER_NUM_DMA,
        COMMAND_COPY_MEMORY_TO_MEMORY,
        (uint32_t) src_buffer,
        (uint32_t) dst_buffer);

    return tock_command_return_novalue_to_returncode(res);
}

int dma_stop(void)
{
    syscall_return_t res = command(
        DRIVER_NUM_DMA,
        COMMAND_STOP,
        0,
        0);

    return tock_command_return_novalue_to_returncode(res);
}

int dma_status(uint32_t* status)
{
    syscall_return_t res = command(
        DRIVER_NUM_DMA,
        200,
        0,
        0);
    return tock_command_return_u32_to_returncode(res, status);
}

int dma_power_off(uint8_t block_no)
{
    syscall_return_t res = command(
        DRIVER_NUM_DMA,
        150,
        block_no,
        0);
    return tock_command_return_novalue_to_returncode(res);
}

int dma_power_on(uint8_t block_no)
{
    syscall_return_t res = command(
        DRIVER_NUM_DMA,
        151,
        block_no,
        0);
    return tock_command_return_novalue_to_returncode(res);
}
