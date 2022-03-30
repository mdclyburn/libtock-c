#pragma once

#include <stdint.h>

typedef void (*performance_fp_t)(uint8_t, uint32_t);

typedef void (*performance_freeze_fp_t)();

int performance_available();

performance_fp_t* performance_call_fp();

performance_freeze_fp_t* performance_freeze_fp();
