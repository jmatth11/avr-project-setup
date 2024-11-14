#include "i2c.h"
#include <util/delay.h>

#define STATUS_CLOCK_8_BITS (_BV(USISIF)|_BV(USIOIF)|_BV(USIPF)|_BV(USIDC) | \
    (0x0 << USICNT0)) // set USI to shift 8 bits, count 16 clock edges

#define STATUS_CLOCK_1_BIT (_BV(USISIF) | \
    _BV(USIOIF) | \
    _BV(USIPF) | \
    _BV(USIDC) | \
    (0xE << USICNT0))// USI to shift 1 bit, count 2 clock edges

/**
 * Initialize I2C registers and ports.
 */
void i2c_init() {
  // flip the ports to output mode
  i2c_bus |= _BV(i2c_sda);
  i2c_bus |= _BV(i2c_scl);

  // set both pins to HIGH to start and enable pullup.
  i2c_port |= _BV(i2c_sda);
  i2c_port |= _BV(i2c_scl);

  // preload data register with default HIGH
  i2c_data = 0xff;

  // setup for master
  i2c_control = (
    // disable Start condition interrupt
    (0 << USISIE) |
    // disable overflow interrupt
    (0 << USIOIE) |
    // only set WM1 to high for normal 2wire mode
    _BV(USIWM1) |
    // set CS1 and CLK to high to use external clock source
    // with positive edge. Software Clock Strobe (with USITC register)
    _BV(USICS1) |
    _BV(USICLK)
  );

  i2c_status = (
    // clear all flags
    _BV(USISIF) |
    _BV(USIOIF) |
    _BV(USIPF) |
    _BV(USIDC) |
    // reset overflow counter
    (0x0 << USICNT0)
  );
}

/**
 * Send start command.
 */
void i2c_start() {
  // ensure both lines are high
  i2c_port |= _BV(i2c_sda);
  i2c_port |= _BV(i2c_scl);
  // wait till clock pin is high
  while (!(i2c_pin & _BV(i2c_scl)));
  _delay_us(5);

  // pull data line low
  i2c_port &= ~_BV(i2c_sda);
  // wait some time
  _delay_us(5);
  // pull clock line low
  i2c_port &= ~_BV(i2c_scl);

  // release data line to high
  i2c_port |= _BV(i2c_sda);
}

/**
 * Send the stop command.
 */
void i2c_stop() {

  // ensure data line is low
  i2c_port &= ~_BV(i2c_sda);

  // relase clock line to high
  i2c_port |= _BV(i2c_scl);
  // wait for clock pin to read high
  while (!(i2c_pin & _BV(i2c_scl)));
  _delay_us(5);

  // relase data line to high
  i2c_port |= _BV(i2c_sda);
  _delay_us(4);
}

unsigned char transfer(unsigned char mask) {
  i2c_port &= ~_BV(i2c_scl);
  i2c_status = mask;

  do {
    // wait a little bit
    _delay_us(5);
    // toggle clock
    i2c_control |= _BV(USITC);
    // wait for SCL to go high
    while (! (i2c_pin & _BV(i2c_scl)));
    // wait short
    _delay_us(4);
    // toggle clock again
    i2c_control |= _BV(USITC);

  } while (!(i2c_status & _BV(USIOIF)));

  unsigned char temp = i2c_data;
  i2c_data = 0xff;
  return temp;
}

/**
 * Write the given byte.
 *
 * @param[in] data The byte to send.
 * @return The N/ACK byte.
 */
unsigned char i2c_write_byte(unsigned char data) {
  i2c_data = data;
  transfer(STATUS_CLOCK_8_BITS);

  // change data pin to input
  i2c_bus &= ~_BV(i2c_sda);
  unsigned char nack = transfer(STATUS_CLOCK_1_BIT);
  // change back to output
  i2c_bus |= _BV(i2c_sda);
  return nack;
}

/**
 * Read the next byte.
 *
 * @param[in] nack True for reading more, false otherwise.
 * @return The read byte.
 */
unsigned char i2c_read_byte(bool nack) {
  // change data pin to input
  i2c_bus &= ~_BV(i2c_sda);
  unsigned char data = transfer(STATUS_CLOCK_8_BITS);
  // change back to output
  i2c_bus |= _BV(i2c_sda);
  if (nack) {
    // 1 means read another byte
    i2c_data = 1;
  } else {
    // 0 means stop sending
    i2c_data = 0;
  }
  // send nack
  transfer(STATUS_CLOCK_1_BIT);
  return data;
}

/**
 * Write the target's address out.
 *
 * @param[in] address The target address.
 * @param[in] write Flag for Write or Read bit.
 * @return The N/ACK byte.
 */
unsigned char i2c_write_address(unsigned char address, bool write) {
  unsigned char rw_flag = 1;
  if (write) rw_flag = 0;
  // shift address over 1 because Least sig byte is RW flag
  return i2c_write_byte((address << 1) | rw_flag);
}
