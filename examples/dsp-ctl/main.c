#include <stdio.h>

#include <dsp.h>
#include <timer.h>
#include <tock.h>

void check_configuration(void);

int main(void)
{
    check_configuration();

    while (true)
    {
        uint32_t processing_ms;
        const int32_t rv = dsp_get_stat(ProcessingDuration, &processing_ms);

        if (rv == RETURNCODE_SUCCESS)
        {
            printf("Processing time: %lu\r\n", processing_ms);
        }
        else
        {
            printf("Stats unavailable (%i).\r\n", (int)rv);
        }

        delay_ms(1517);
    }
}

void check_configuration(void)
{
    for (;;)
    {
        const int32_t rv = dsp_available();
        if (rv == RETURNCODE_SUCCESS)
        {
            break;
        }
        else
        {
            printf("DSP stats have not been configured.\r\n");
            delay_ms(1517);
        }
    }

    return;
}
