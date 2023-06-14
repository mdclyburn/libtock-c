#include "audio.h"

#define COMMAND_PLAY 10

int audio_play(void)
{
    syscall_return_t res = command(
        DRIVER_NUM_AUDIO,
        COMMAND_PLAY,
        0,
        0);

    return tock_command_return_novalue_to_returncode(res);
}
