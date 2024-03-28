#include <stdio.h>

#include <led.h>
#include <timer.h>

#define UPDATE_PERIOD_MS 1000
#define ACTION_LIMIT 18

tock_timer_t display_update_timer;
int actions = 0;

void update_display(int, int, int, void*);

int main(void)
{
    /* led_off(0); */
    /* printf("i"); */
    timer_in(UPDATE_PERIOD_MS, update_display, NULL, &display_update_timer);

    while (true) { yield(); }
}

void update_display(__attribute__ ((unused)) int a1,
                    __attribute__ ((unused)) int a2,
                    __attribute__ ((unused)) int a3,
                    __attribute__ ((unused)) void* a4)
{
    printf("d");

    actions++;
    if (actions < ACTION_LIMIT)
        timer_in(UPDATE_PERIOD_MS, update_display, NULL, &display_update_timer);

    return;
}
