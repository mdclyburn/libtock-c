#include <ninedof.h>
#include <timer.h>

#define RATIO 0.6

tock_timer_t event_timer;

void event_timer_fired(int, int, int, void*);

int main(void)
{
	delay_ms(alarm_read() % 1000);
    timer_in((uint32_t) ((float) 750.0 / (RATIO / 1.0)),
			 event_timer_fired,
			 NULL,
			 &event_timer);

    while (true) { yield(); }
}

void event_timer_fired(__attribute__ ((unused)) int a1,
					   __attribute__ ((unused)) int a2,
					   __attribute__ ((unused)) int a3,
					   __attribute__ ((unused)) void* a4)
{
	int x, y, z;
	float r = 750.0 / (RATIO / 1.0);
    timer_in(((uint32_t) (r - (r / 10.0) + (float) (alarm_read() % (uint32_t) (r / 5.0)))),
			 event_timer_fired,
			 NULL,
			 &event_timer);

	ninedof_read_acceleration_sync(&x, &y, &z);

    return;
}
