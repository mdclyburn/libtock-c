#include "dsp.h"

int dsp_get_stat(DSP_Stat_t stat, uint32_t* const out)
{
    syscall_return_t rval = command(DSP_DRIVER_NUM, 1, (uint32_t) stat, 0);
    return tock_command_return_u32_to_returncode(rval, out);
}
