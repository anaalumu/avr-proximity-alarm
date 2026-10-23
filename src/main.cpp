#include <avr/io.h>

#define F_CPU 16000000UL
#include "util/delay.h"

#define buzzerP PIND7 // passive buzzer pin
#define buzzerA PINC0 // active buzzer pin 

void playSoundFor100ms(float F, float x, int playTime);

int main(void) {

  //DDRB = 0xFF;
  DDRC = 0xFF;
  DDRD = 0xFF;

  float note5[] = {659.26, 659.26, 698.46, 783.99, 783.99, 698.46, 659.26, 587.33, 523.25, 523.25, 587.33, 659.26, 659.26, 587.33, 587.33, 523.25,659.26, 659.26, 698.46, 783.99, 783.99, 698.46, 659.26, 587.33, 523.25, 523.25, 587.33, 659.26, 587.33, 523.25, 523.25};
  float *pNote5 = note5;

  

  while (1) {
    int i = 0;
    while(i < sizeof(note5)/sizeof(float)){
      if(i % 2 == 0){
        playSoundFor100ms(*(pNote5+i), 0.4, 2e5);
      }else{
        playSoundFor100ms(*(pNote5+i), 0.4, 2e5);
      }
        _delay_ms(100);
        i++;
    }
      _delay_ms(500);
    }
    //for (char i = 0; i < sizeof(note5)/sizeof(float); i++) {

}


/*void playSoundFor100ms(float F, float x, int playTime) {
    // Calculate the period of the note
    float T = 1 / F * 1e6;
    int Ton = x * T;
    int Toff = T - Ton;

    // Calculate the number of cycles needed for the given duration
    int cycles = playTime / (Ton + Toff);

    for (int j = 0; j < cycles; j++) {
        PORTB |= (1 << buzzerP); // Turn buzzer on
        for (int i = 0; i < Ton; i++) {
            _delay_us(1); // Wait for Ton microseconds
        }

        PORTB &= ~(1 << buzzerP); // Turn buzzer off
        for (int i = 0; i < Toff; i++) {
            _delay_us(1); // Wait for Toff microseconds
        }
    }
}*/


void playSoundFor100ms(float F, float x, int playTime) {
    // Calculate the period of the note
    float T = 1 / F*1e6;
    int Ton = x*T;
    int Toff = T - Ton;

    for (int j = 0; j < playTime/Ton; j++) {
        PORTD = PORTD | (1 << buzzerP);
        for (int i = 0; i < Ton; i++) {
        _delay_us(1); 
        }

      PORTD = PORTD &~ (1 << buzzerP);
        for (int i = 0; i < Toff; i++){
        _delay_us(1); 
      }
      PORTD = 0;
    }
    for (int j = 0; j < playTime/Ton; j++) {
      PORTC = PORTC | (1 << buzzerA);
        for (int i = 0; i < Ton; i++) {
        _delay_us(2); 
      }

      PORTC = PORTC &~ (1 << buzzerA);
        for (int i = 0; i < Toff; i++){
        _delay_us(2); 
      }
      PORTC = 0;
  }
}











/* PART 1#include <avr/io.h>
#include "util/delay.h"

#define F_CPU 16000000UL
#define buzzerP PINB0 // passive buzzer pin
#define buzzerA PINB1 // active buzzer pin 


int main(void) {

  DDRB = 0xFF;

  volatile float F = 587.33; //Hz
  volatile float T = 1/F*1e6; // 1/F is in second
  volatile float x = 0.5; // duty cycle
  volatile int Ton = x*T; 
  volatile int Toff = T - Ton;

  while (1) {
    PORTB = PORTB | ( 1 << buzzerP );
    for (int i = 0; i < Ton; i++) {
      _delay_us(1);
    }
PORTB = PORTB & ~( 1 << buzzerP );
for (int i = 0; i < Toff; i++) {
  _delay_us(1);
} 

    }
  
}*/