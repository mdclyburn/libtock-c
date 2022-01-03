#include <stdio.h>

#include <dsp.h>
#include <timer.h>
#include <tock.h>

int main(void)
{
    while (true)
    {
        uint32_t processing_ms;
        const int32_t rv = dsp_get_stat(ProcessingDuration, &processing_ms);
        if (rv == RETURNCODE_SUCCESS)
        {
            printf("Processing time: %lu\n", processing_ms);
        }
        else
        {
            printf("Stats busy.\n");
        }

        /* delay_ms(517); */
    }
}
