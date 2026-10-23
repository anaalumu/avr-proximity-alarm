#include <avr/io.h>
#define F_CPU 16000000UL

#include "util/delay.h"
#define buzzerP PINB0 // passive buzzer pin
#define buzzerA PINB1 // active buzzer pin 

void playSoundFor100ms(float F, float x);

int main(void) {

  DDRB = 0xFF;

  float note5[] = {523.25, 587.33, 659.26, 698.46, 783.99, 880.00, 987.77};
  float *pNote5 = note5;

  while (1) {
    for (char i = 0; i < sizeof(note5)/sizeof(float); i++) {
        playSoundFor100ms(*(pNote5+i), 0.1);
        _delay_ms(100);
    }
      _delay_ms(500);

    }
  
}

void playSoundFor100ms(float F, float x) {
    // Calculate the period of the note
    float T = 1 / F*1e6;
    int Ton = x*T;
    int Toff = T - Ton;

    /* Calculate the number of cycles needed for the given duration
    int cycles = (int)(x * 1000.0 / (period * 2));*/

    for (int j = 0; j < 2e5/T; j++) {
        
        PORTB = PORTB | (1 << buzzerP);
        for (int i = 0; i < Ton; i++) {
        _delay_us(1); 
    }

    PORTB = PORTB &~ (1 << buzzerP);
        for (int i = 0; i < Toff; i++){
        _delay_us(1); 
    }
}
}


/*void playSoundFor100ms(float F, float x) {
    // Calculate the period of the note
    float period = 1.0 / F;

    // Calculate the number of cycles needed for the given duration
    int cycles = (int)(x * 1000.0 / (period * 2));

    for (int i = 0; i < cycles; i++) {
        // Toggle the buzzer pin to generate sound
        PORTB ^= (1 << buzzerP);
        _delay_ms(period* 1000); // Delay for half of the period
    }
}*/


