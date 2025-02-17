#include <stdio.h>

#include <led.h>
#include <timer.h>

#define UPDATE_PERIOD_MS 1000
#define ACTION_LIMIT 1000

tock_timer_t display_update_timer;
int actions = 0;

void update_activity(void);

void update_display(int, int, int, void*);
void update_display_every(int, int, int, void*);uint32_t t_started = 0;
uint32_t t_mark = 0;
void update_activity(void)
{
	uint32_t t_now = (alarm_read() / 16000) - t_started;

	if (t_now > 14640) { // sleeping
		timer_cancel(&display_update_timer);
	} else {
		/* printf("no change\n"); */
	}

	return;
}


int main(void)
{
	t_started = alarm_read() / 16000;
    /* led_off(0); */
    /* printf("i"); */
    /* timer_in(UPDATE_PERIOD_MS, update_display, NULL, &display_update_timer); */
	timer_every(UPDATE_PERIOD_MS, update_display_every, NULL, &display_update_timer);

    while (true) { yield(); }
}

void update_display(__attribute__ ((unused)) int a1,
                    __attribute__ ((unused)) int a2,
                    __attribute__ ((unused)) int a3,
                    __attribute__ ((unused)) void* a4)
{
    printf("d");

    actions++;
    if (1)
        timer_in(UPDATE_PERIOD_MS, update_display, NULL, &display_update_timer);

    return;
}

void update_display_every(__attribute__ ((unused)) int a1,
						  __attribute__ ((unused)) int a2,
						  __attribute__ ((unused)) int a3,
						  __attribute__ ((unused)) void* a4)
{
	printf("e");

	/* update_activity(); */
	actions++;
	/* if (1) */
	/* 	timer_cancel(&display_update_timer); */

	return;
}
