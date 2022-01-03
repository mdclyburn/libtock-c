#pragma once

#include <stdint.h>

#include "tock.h"

#define DSP_DRIVER_NUM 0x91000

typedef enum {
    CollectDuration = 0,
    ProcessingDuration,
} DSP_Stat_t;

int dsp_get_stat(DSP_Stat_t stat, uint32_t* const out);
