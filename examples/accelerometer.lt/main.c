#include <stdio.h>
#include <stdint.h>

#include <evaluation.h>
#include <led.h>
#include <ninedof.h>
#include <rng.h>
#include <timer.h>

uint32_t my_rng = 0;
tock_timer_t read_accelm_timer;

void read_accelm(int, int, int, void*);uint32_t t_started = 0;
uint32_t t_mark = 0;

void update_activity(void);

void update_activity(void)
{
	uint32_t t_now = (alarm_read() / 16000) - t_started;

	if (t_now < 3130) { // sitting
		t_mark = 3130;
		/* printf("updated 3130\n"); */
		/* delay_ms(500); */
		/* printf("updated 3130\n"); */
		timer_cancel(&read_accelm_timer);
		timer_every(60776 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 3430) { // chores
		t_mark = 3430;
		timer_cancel(&read_accelm_timer);
		timer_every(586 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 3550) { // sitting
		t_mark = 3550;
		timer_cancel(&read_accelm_timer);
		timer_every(60776 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 4470) { // walking
		t_mark = 4470;
		timer_cancel(&read_accelm_timer);
		timer_every(60776 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 6620) { // eating
		t_mark = 6620;
		timer_cancel(&read_accelm_timer);
		timer_every(4281 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 6800) { // walking
		t_mark = 6800;
		timer_cancel(&read_accelm_timer);
		timer_every(776 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 7150) { // chores
		t_mark = 7150;
		timer_cancel(&read_accelm_timer);
		timer_every(476 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 7740) { // walking
		t_mark = 7740;
		timer_cancel(&read_accelm_timer);
		timer_every(776 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 7810) { // chores
		t_mark = 7810;
		timer_cancel(&read_accelm_timer);
		timer_every(476 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 14640) { // sitting
		t_mark = 14640;
		timer_cancel(&read_accelm_timer);
		timer_every(60776 + (my_rng % 100),
					read_accelm,
					NULL,
					&read_accelm_timer);
	} else if (t_now < 18000) { // sleeping
		t_mark = 18000;
		timer_cancel(&read_accelm_timer);
	} else {
		/* printf("no change\n"); */
	}

	return;
}


int main(void)
{
	t_started = alarm_read() / 16000;
	int bytes_rcv;
	rng_sync((uint8_t*) &my_rng, 4, 4, &bytes_rcv);

	/* my_rng = (my_rng * 37829 + 41) % 999999; */

    /* printf("i"); */
    /* timer_in(776 + (my_rng % 100), update_display, NULL, &read_accelm_timer); */
	timer_every(776 + (my_rng % 100),
                read_accelm,
                NULL,
                &read_accelm_timer);

	/* ninedof_avm_subscribe(true); */

    while (true) { yield(); }
}

void read_accelm(__attribute__ ((unused)) int a1,
                    __attribute__ ((unused)) int a2,
                    __attribute__ ((unused)) int a3,
                    __attribute__ ((unused)) void* a4)
{

	my_rng = (my_rng * 37829 + 41) % 999999;
	timer_cancel(&read_accelm_timer);
	update_activity();
	/* timer_in(676 + (my_rng % 100), read_accelm, NULL, &read_accelm_timer); */

	int x, y, z;
	ninedof_read_acceleration_sync(&x, &y, &z);
	for (uint32_t i = 0; i < my_rng % 5000000; i++)
	{
		my_rng = (my_rng * 37829 + 41) % 999999;
	}

    return;
}
