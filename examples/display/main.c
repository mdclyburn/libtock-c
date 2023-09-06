#include <stdio.h>

#include <led.h>
#include <timer.h>

#define UPDATE_PERIOD_MS 1000

tock_timer_t display_update_timer;

void update_display(int, int, int, void*);

int main(void)
{
    led_off(0);
    printf("i");
    timer_every(UPDATE_PERIOD_MS, update_display, NULL, &display_update_timer);

    while (true) { yield(); }
}

void update_display(__attribute__ ((unused)) int a1,
                    __attribute__ ((unused)) int a2,
                    __attribute__ ((unused)) int a3,
                    __attribute__ ((unused)) void* a4)
{
    printf("d");
    return;
}
