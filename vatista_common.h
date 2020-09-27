/*
 * vatista_common.h
 *
 * Created: 2020/09/23 22:49:46
 *  Author: lum1narie
 */


#ifndef VATISTA_COMMON_H_
#define VATISTA_COMMON_H_

#include <avr/io.h>

#define ENABLE_POWER_SAVE

#define CLK_PORT PORTC
#define DOUT_PORT PORTC
#define CS_PORT PORTC

#define CLK_PIN PORTC0
#define DOUT_PIN PORTC2
#define CS_PIN PORTC1

#ifdef ENABLE_POWER_SAVE
#define TCH_CLOCK 1
#else
#define TCH_CLOCK 5
#endif

#define get_bit(n, bit) (((n) >> (bit)) & 1)
#define set_bit(n, bit) ((n) |= (uint8_t)(_BV(bit)))
#define clr_bit(n, bit) ((n) &= ~(uint8_t)(_BV(bit)))

#ifdef ENABLE_POWER_SAVE
#define F_CPU 32768UL
#else
#define F_CPU 1048576UL
#endif

#endif /* VATISTA_COMMON_H_ */
