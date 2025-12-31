#pragma once

#include <stdint.h>

#include "../../tock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRIVER_NUM_ISLE ((uint32_t) 0x30999)

#define ISLE_IN_BUFFER  ((uint32_t) 0)
#define ISLE_OUT_BUFFER ((uint32_t) 1)

returncode_t
libtock_isle_set_ro_allow_in_buffer(
	const uint8_t* buffer,
	const uint32_t len);

returncode_t
libtock_isle_set_rw_allow_out_buffer(
	const uint8_t* buffer,
	const uint32_t len);

#ifdef __cplusplus
}
#endif
