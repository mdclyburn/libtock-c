#pragma once

#include <stdint.h>

#define DSP_DRIVER_NUM 0x91000

typedef enum {
    CollectDuration = 1,
    ProcessingDuration = 2,
} DSP_Stat_t;

int dsp_available();

int dsp_get_stat(DSP_Stat_t stat, uint32_t* const out);
