#include <avr/io.h>
#include <util/delay.h>

#define MS_DELAY 1000

int main (void) {
  /*Set the fifth bit of DDRB to one
  **Set digital pin 13 to output mode */
  DDRB |= _BV(DDB5);

  while(1) {
    /*Set the fifth bit of PORTB to one
    **Toggle pin 13 between HIGH and LOW (on/off) */
    PORTB ^= _BV(PORTB5);

    /*Wait 3000 ms */
    _delay_ms(MS_DELAY);
  }
}
