/*
 * vatista_matrix.c
 *
 * Created: 2020/09/16 09:47:00
 * Author : lum1narie
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <util/delay_basic.h>

#define CLK_PORT PORTC
#define DOUT_PORT PORTC
#define CS_PORT PORTC

#define CLK_PIN 0
#define DOUT_PIN 2
#define CS_PIN 1

#define get_bit(n, bit) (((n) >> (bit)) & 1)

#define F_CPU 8000000UL

/*
 *
 *   ####
 * # #  # #
 * ###  ###
 *  ######
 * ########
 *   #  #
 *     #
 */
const uint8_t vatista_pattern[] PROGMEM = {
    0b00000000,
    0b00111100,
    0b10100101,
    0b11100111,
    0b01111110,
    0b11111111,
    0b00100100,
    0b00001000
};

inline void send_max7219(uint8_t address, uint8_t data) {
#define TCH_CLOCK 5
    cli(); 
    
    CS_PORT &= ~(1 << CS_PIN);

    // send something for 7~4th bits of address
    for (int i = 7; i >= 4; i--){
        CLK_PORT |= (1 << CLK_PIN);
        _delay_loop_1(TCH_CLOCK);
        CLK_PORT &= ~(1 << CLK_PIN);
    }
    // send 3~0th bits of address
    for (int i = 3; i >= 0 ; i--)
    {
        if(get_bit(address, i)) {
            DOUT_PORT |= (1 << DOUT_PIN);            
        } else {
            DOUT_PORT &= ~(1 << DOUT_PIN);            
        }
        CLK_PORT |= (1 << CLK_PIN);
        _delay_loop_1(TCH_CLOCK);
        CLK_PORT &= ~(1 << CLK_PIN);
    }
    // send 7~0th bits of data
    for (int i = 7; i >= 0 ; i--)
    {
        if(get_bit(data, i)) {
            DOUT_PORT |= (1 << DOUT_PIN);
            } else {
            DOUT_PORT &= ~(1 << DOUT_PIN);
        }
        CLK_PORT |= (1 << CLK_PIN);
        _delay_loop_1(TCH_CLOCK);
        CLK_PORT &= ~(1 << CLK_PIN);
    }
    
    CS_PORT |= (1 << CS_PIN);
    
    sei();
}

inline void init_max7219() {
    // set brightness
    send_max7219(0x0A, 0x02);
    // scan all digit
    send_max7219(0x0B, 0x07);

    // No decode for digits 7~0
    send_max7219(0x09, 0x00);

    // set normal(not test) mode
    send_max7219(0x0F, 0x00);
    
    // boot max7219
    send_max7219(0x0C, 0x01);
}


inline void init_ports() {
    DDRB = 0xFF;
    PORTB = 0x00;
    DDRC = 0xFF;
    PORTC = 0x00;
    DDRD = 0xFF;
    PORTD = 0x00;
    
    CS_PORT |= (1 << CS_PIN);
}

int main(void)
{
    _delay_ms(1500);
    init_ports();
    init_max7219();

    // send vatista data
    for (int i = 0; i <= 7 ; i++)
    {
        uint8_t row = pgm_read_byte(&(vatista_pattern[i]));
        uint8_t reverse_row = 0x00;
        uint8_t address = 0x08 - i;

        for (int i = 0; i <= 7; i++) {
            reverse_row |= (get_bit(row, i) << (7 - i));
        }

        send_max7219(address, reverse_row);
    }
    
    _delay_ms(1000);

    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    while(1){
        sleep_mode();
    }
}

