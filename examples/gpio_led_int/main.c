#include <gpio.h>
#include <led.h>
#include <timer.h>
#include <stdio.h>

void toggle_led(uint32_t _r0, uint32_t _r1, uint32_t _r2)
{
    printf("toggling LED\n");
    led_on(2);
    return;
}

int main(void) {
  // Ask the kernel how many LEDs are on this board.
  int num_leds;
  int err = led_count(&num_leds);
  if (err < 0) return err;
  int up = num_leds;
  int lo = num_leds-1;

  gpio_interrupt_callback(toggle_led, NULL);
  gpio_enable_input(0, PullUp);
  gpio_enable_interrupt(0, FallingEdge);

  while(true) {
      // Requires the process to yield to get toggle_led to run after GPIO interrupt.
      /* delay_ms(100); */
  }
}
