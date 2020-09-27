/*
 * vatista_matrix.c
 *
 * Created: 2020/09/16 09:47:00
 * Author : lum1narie
 */

#include "vatista_common.h"
#include "max7219.h"

#include "vatista_common.h"
#include "max7219.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdbool.h>


volatile uint8_t brightness;
static uint8_t EEMEM brightness_eep;
volatile bool was_tmr0_on = false;

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

static inline void add_brightness(int8_t diff) {
  int8_t bness_after = (int8_t)brightness + diff;
  if (bness_after > 0x0F) {
    bness_after = 0x0F;
  } else if (bness_after < 0) {
    bness_after = 0x00;
  }
  brightness = (uint8_t)bness_after;

  // set brightness
  send_max7219(0x0A, brightness);

  // show brightness status
  uint8_t bness_display[8];
  for (int i = 0; i < 8 ; i++) {
    if ((i & 0x3) == 3) {
      bness_display[i] = 0xFF;
    } else if(i <= 3) {
      bness_display[i] = (brightness >= 8 ? (uint8_t)0xFF << ( 15 - brightness) : 0x00);
    } else {
      bness_display[i] = (brightness <= 7 ? (uint8_t)0xFF << (7 - brightness) : 0xFF);
    }
  }
  send_reversed_pattern_max7219(bness_display);
}

static inline void enable_ext_int() {
  EIMSK |= (_BV(INT0) | _BV(INT1));
}

static inline void disable_ext_int() {
  EIMSK &= ~(_BV(INT0) | _BV(INT1));
}

static inline void stop_tmr_1() {
  clr_bit(TIMSK1, OCIE1A);
  TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
  TCNT1 = 0;
}

static inline void start_tmr_1() {
  stop_tmr_1();

  set_bit(TIMSK1, OCIE1A);

  // prescaled tmr0 by 1/1024 (32kHz -> 32Hz)
  TCCR1B |= (_BV(CS10) | _BV(CS12));
  clr_bit(TCCR0B, CS11);
}

static inline void init_ext_int() {
  EICRA &= ~(_BV(ISC00) | _BV(ISC01) | _BV(ISC10) | _BV(ISC11));
}

static inline void init_tmr_1() {
  // set tmr0 for 1.5 sec
#ifdef ENABLE_POWER_SAVE
  OCR1A = 48;
#else
  OCR1A = 1536;
#endif
}


static inline void init_ports() {
  DDRB = 0xFF;
  PORTB = 0x00;
  DDRC = 0xFF;
  PORTC = 0x00;
  DDRD = 0xF3;
  PORTD = 0x0C;

  set_bit(CS_PORT, CS_PIN);
}

static inline void init_clk() {
  CLKPR = _BV(CLKPCE);

  // prescale system clk by 1/256 (8MHz -> 32KHz)
  CLKPR = 0x08;
}

static inline void init_pwr_save() {
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);

  // disable TWI, TMR0/2, USART, ADC
  PRR |= (_BV(PRTWI) | _BV(PRTIM0) | _BV(PRTIM2) | _BV(PRUSART0) | _BV(PRADC));
}

static inline void init_settings() {
  cli();
  init_ports();
  init_ext_int();
  init_tmr_1();
#ifdef ENABLE_POWER_SAVE
  init_pwr_save();
  init_clk();
#endif

  eeprom_busy_wait();
  brightness = eeprom_read_byte(&brightness_eep);

  _delay_ms(50);
  init_max7219();

  // set brightness
  send_max7219(0x0A, brightness);
  sei();
}

ISR(INT0_vect) {
  _delay_ms(20);
  while (get_bit(PIND, PIND2) == 0) {}
  add_brightness(-1);

  _delay_ms(20);
  start_tmr_1();
}

ISR(INT1_vect) {
  _delay_ms(20);
  while (get_bit(PIND, PIND3) == 0) {}
  add_brightness(1);

  _delay_ms(20);
  start_tmr_1();
}

ISR(TIMER1_COMPA_vect) {
  stop_tmr_1();

  eeprom_busy_wait();
  eeprom_update_byte(&brightness_eep, brightness);
  send_reversed_pattern_max7219_P(vatista_display);

  was_tmr0_on = true;
}

int main(void) {
  init_settings();

  // send vatista data
  send_reversed_pattern_max7219_P(vatista_display);

  enable_ext_int();

#ifdef POWER_SAVE_MODE
  sleep_mode();
#endif

  while(1) {
#ifdef POWER_SAVE_MODE
    if (was_tmr0_on) {
      was_tmr0_on = false;
      sleep_mode();
    }
#endif
  }
}
