#include "performance.h"
#include "tock.h"

static const DRIVER_NO = 0x00010;

int performance_available()
{
    const syscall_return_t r = command(DRIVER_NO, 0, 0, 0);
    return tock_command_return_novalue_to_returncode(r);
}

performance_fp_t* performance_call_fp()
{
    const syscall_return_t sr = command(DRIVER_NO, 1, 0, 0);

    uint32_t fp_addr = 0;
    const int rc = tock_command_return_u32_to_returncode(sr, &fp_addr);
    if (rc != 0)
    {
        fp_addr = NULL;
    }

    return (performance_fp_t*) fp_addr;
}

performance_freeze_fp_t* performance_freeze_fp()
{
    const syscall_return_t sr = command(DRIVER_NO, 2, 0, 0);

    uint32_t fp_addr = 0;
    const int rc = tock_command_return_u32_to_returncode(sr, &fp_addr);
    if (rc != 0)
    {
        fp_addr = NULL;
    }

    return (performance_freeze_fp_t*) fp_addr;
}
