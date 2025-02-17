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

int main(void)
{
	t_started = alarm_read() / 16000;
	int bytes_rcv;
	rng_sync((uint8_t*) &my_rng, 4, 4, &bytes_rcv);

	/* my_rng = (my_rng * 37829 + 41) % 999999; */

    /* printf("i"); */
   timer_in(776 + (my_rng % 100), read_accelm, NULL, &read_accelm_timer);

	/* ninedof_avm_subscribe(true); */

    while (true) { yield(); }
}

void read_accelm(__attribute__ ((unused)) int a1,
                    __attribute__ ((unused)) int a2,
                    __attribute__ ((unused)) int a3,
                    __attribute__ ((unused)) void* a4)
{

	my_rng = (my_rng * 37829 + 41) % 999999;
	timer_in(676 + (alarm_read() % 140), read_accelm, NULL, &read_accelm_timer);

	int x, y, z;
	ninedof_read_acceleration_sync(&x, &y, &z);
	for (uint32_t i = 0; i < my_rng % 5000000; i++)
	{
		my_rng = (my_rng * 37829 + 41) % 999999;
	}

    return;
}
