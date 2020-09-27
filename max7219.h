/*
* max7219.h
*
* Created: 2020/09/23 22:49:27
*  Author: lum1narie
*/


#ifndef MAX7219_H_
#define MAX7219_H_

#include "vatista_common.h"
#include "max7219.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay_basic.h>
#include <avr/pgmspace.h>


static inline void send_max7219(uint8_t address, uint8_t data);
static inline void init_max7219();
static inline void send_reversed_pattern_max7219(uint8_t data[]);
static inline void send_reversed_pattern_max7219_P(const uint8_t prog_data[] PROGMEM);

static inline void send_max7219(uint8_t address, uint8_t data) {
  cli();

  clr_bit(CS_PORT, CS_PIN);

  // send something for 7~4th bits of address
  for (int i = 7; i >= 4; i--) {
    set_bit(CLK_PORT, CLK_PIN);
    _delay_loop_1(TCH_CLOCK);
    clr_bit(CLK_PORT, CLK_PIN);
  }
  // send 3~0th bits of address
  for (int i = 3; i >= 0 ; i--) {
    if(get_bit(address, i)) {
      set_bit(DOUT_PORT, DOUT_PIN);
    } else {
      clr_bit(DOUT_PORT, DOUT_PIN);
    }
    set_bit(CLK_PORT, CLK_PIN);
    _delay_loop_1(TCH_CLOCK);
    clr_bit(CLK_PORT, CLK_PIN);
  }
  // send 7~0th bits of data
  for (int i = 7; i >= 0 ; i--) {
    if(get_bit(data, i)) {
      set_bit(DOUT_PORT, DOUT_PIN);
    } else {
      clr_bit(DOUT_PORT, DOUT_PIN);
    }
    set_bit(CLK_PORT, CLK_PIN);
    _delay_loop_1(TCH_CLOCK);
    clr_bit(CLK_PORT, CLK_PIN);
  }

  set_bit(CS_PORT, CS_PIN);

  sei();
}

static inline void init_max7219() {
  // scan all digit
  send_max7219(0x0B, 0x07);

  // No decode for digits 7~0
  send_max7219(0x09, 0x00);

  // set normal(not test) mode
  send_max7219(0x0F, 0x00);

  // boot max7219
  send_max7219(0x0C, 0x01);
}

static inline void send_reversed_pattern_max7219(uint8_t data[]) {
  for (int i = 0; i <= 7 ; i++) {
    uint8_t row = data[i];
    uint8_t reverse_row = 0x00;
    uint8_t address = 0x08 - i;

    for (int j = 0; j <= 7; j++) {
      reverse_row |= (get_bit(row, j) << (7 - j));
    }

    send_max7219(address, reverse_row);
  }
}

static inline void send_reversed_pattern_max7219_P(const uint8_t prog_data[] PROGMEM) {
  for (int i = 0; i <= 7 ; i++) {
    uint8_t row = pgm_read_byte(&(prog_data[i]));
    uint8_t reverse_row = 0x00;
    uint8_t address = 0x08 - i;

    for (int j = 0; j <= 7; j++) {
      reverse_row |= (get_bit(row, j) << (7 - j));
    }

    send_max7219(address, reverse_row);
  }
}

#endif /* MAX7219_H_ */
