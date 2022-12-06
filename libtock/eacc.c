#include "eacc.h"

#include "tock.h"

#define DRIVER_NUM_EACC 0x10001

uint64_t eacc_total_accounted(void)
{
    uint64_t accounted = 0;
    syscall_return_t res = command(DRIVER_NUM_EACC, 1, 0, 0);
    tock_command_return_u64_to_returncode(res, &accounted);

    return accounted;
}
