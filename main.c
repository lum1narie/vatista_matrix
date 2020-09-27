/*
 * vatista_matrix.c
 *
 * Created: 2020/09/16 09:47:00
 * Author : lum1narie
 */

#include "vatista_common.h"
#include "max7219.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <util/delay.h>


/*
 *
 *   ####
 * # #  # #
 * ###  ###
 *  ######
 * ########
 *   #  #
 *   # #
 */

// clang-format off
const uint8_t vatista_display[] PROGMEM = {
  0b00000000,
  0b00111100,
  0b10100101,
  0b11100111,
  0b01111110,
  0b11111111,
  0b00100100,
  0b00101000
};
// clang-format on

static inline void init_ports() {
  DDRB = 0xFF;
  PORTB = 0x00;
  DDRC = 0xFF;
  PORTC = 0x00;
  DDRD = 0xF3;
  PORTD = 0x0C;

  set_bit(CS_PORT, CS_PIN);
}

static inline void init_settings() {
  cli();
  init_ports();
  _delay_ms(50);
  init_max7219();
  sei();
}

int main(void) {
  init_settings();

  // send vatista data
  send_reversed_pattern_max7219_P(vatista_display);

  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  while (1) {
    sleep_mode();
  }
}
