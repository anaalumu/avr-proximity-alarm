#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <math.h>

#define bitSet(reg, n) (reg |= 1 << n)
#define bitRead(reg, n) (reg & (1 << n))
#define bitClear(reg, n) (reg &= ~(1 << n))

#define FOSC 16000000
#define BAUD 9600
#define MYUBRR FOSC / 16 / BAUD - 1

//define PINs
#define TRIG PINB1
#define ECHO PINB0
#define ONOFF_BUTTOM PIND7
#define LED PIND5 // PIND5 is the OCR0B register
#define BUZZER_PIN PIND6 // PIND6 is the OCR0A register
#define VOLUME_BUTTOM PIND3 //for INT1 interruption
#define PHOTORESISTOR PINC1 // analog PIN

// variables for controlling states
volatile bool lightState = false;
volatile bool buzzerState = false;
volatile bool programState = true; // Flag to control program on/off state
volatile uint8_t volumeLevel = 1; // 1: Low, 2: Medium, 3: High

// controlling the deboouncing
volatile bool onoff_buttom_status_old = 1;
volatile bool onoff_buttom_status;

volatile bool volume_buttom_status_old = 1;
volatile bool volume_buttom_status;

// USART functions
void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);
void usart_tx_float(float x, char num_digits_int, char num_digits_decimal);
void txString(const char *pStr);
void send_teleplot(double distance);

//initialization functions
void initial();
void init_TC0();

double measure_distance();
void on_off_buttom();
void PWM_SetDutyCycleBuzzer(uint8_t duty_cycle);
void PWM_SetDutyCycleLed(uint8_t adc_value);
void setBuzzerVolume();
void ADC_init();
uint16_t ADC_Read(uint8_t channel);


int main(void) {
    ADC_init();
    USART_Init(MYUBRR); // Use the defined UBRR
    double unit = 1;
    initial();
    init_TC0();
    sei();

    while (1) {
        on_off_buttom();
        uint16_t adc_value = ADC_Read(PHOTORESISTOR);

        if (programState) { // Only execute program if it's in the ON state
            double distance = unit * measure_distance();

            // Calculate delay for light and buzzer based on distance, ensuring it doesn't exceed 3 seconds
            double delay_seconds = distance / 100.0;
            delay_seconds = (delay_seconds > 3.0) ? 3.0 : delay_seconds;

            // Convert seconds to milliseconds
            unsigned int delay_ms = (unsigned int)(delay_seconds * 1000);

            // Toggle light state
            lightState = !lightState;

            if (lightState) {
              // Turn on light
              PWM_SetDutyCycleLed(adc_value);
            } else {
                // Turn off light
                OCR0B = 0;
            }

            // Toggle buzzer state
            buzzerState = !buzzerState;

            if (buzzerState) {
                // Turn on buzzer
                setBuzzerVolume();
            } else {
                // Turn off buzzer
                PWM_SetDutyCycleBuzzer(0);
            }

            // Delay for the specified interval
            for (unsigned int i = 0; i < delay_ms; i++) {
                _delay_ms(1);
            }
        } else {//buttom is turned off
            PWM_SetDutyCycleBuzzer(0);
            OCR0B = 0;
        }
    }
}

void initial() {
    DDRB |= (1 << TRIG); //sets trig as output
    DDRB &= ~(1 << ECHO); //sets echo as input
    PORTD = PORTD | (1 << ONOFF_BUTTOM);
    PORTD = PORTD | (1 << VOLUME_BUTTOM);
}

double measure_distance() {
    bitSet(PORTB, TRIG);
    _delay_us(10);
    bitClear(PORTB, TRIG);
    _delay_us(10);

    while (!bitRead(PINB, ECHO))
        ;
    uint16_t duration = 0;
    while (bitRead(PINB, ECHO)) {
        duration++;
        _delay_us(10);
    }
    double distance = (duration * 0.343) / 2;
    return distance;
}

void on_off_buttom(){
    onoff_buttom_status = bitRead(PIND, ONOFF_BUTTOM);

    if(onoff_buttom_status != onoff_buttom_status_old){
        _delay_ms(10);
        onoff_buttom_status = bitRead(PIND, ONOFF_BUTTOM);

        if(onoff_buttom_status != onoff_buttom_status_old){
            onoff_buttom_status_old = onoff_buttom_status;

            if(onoff_buttom_status == 0){
                programState = !programState;
                if(programState){
                txString("1");
                txString("\n");
            }else{
                txString("0");
                txString("\n");
            } 
            txString("\n");
            }
        }
    }
}


void init_TC0() {
    DDRD |= (1 << BUZZER_PIN);
    DDRD |= (1 << LED);

    // Set Fast PWM mode with non-inverting mode
    TCCR0A |= (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);

    // Set prescaler to 64
    TCCR0B |= (1 << CS01) | (1 << CS00);


    EICRA |= (1 << ISC11); // Falling edge of INT1 generates an interrupt
    EIMSK |= (1 << INT1);
}

void PWM_SetDutyCycleBuzzer(uint8_t duty_cycle) {
    OCR0A = duty_cycle; // Set the compare value
}

ISR(INT1_vect) {
    volume_buttom_status = bitRead(PIND, VOLUME_BUTTOM);
    _delay_ms(10);

    if(volume_buttom_status == 0){
        if(volumeLevel == 1){
            volumeLevel = 2;
            txString("2");
            txString("\n");
        }else if(volumeLevel == 2){
            volumeLevel = 3;
            txString("3");
            txString("\n");
        }else{
            volumeLevel = 1;
            txString("1");
            txString("\n");
        }
    }       
}

void setBuzzerVolume() {
    switch (volumeLevel) {
        case 1:
            PWM_SetDutyCycleBuzzer(5);
            break;
        case 2:
            PWM_SetDutyCycleBuzzer(10);
            break;
        case 3:
            PWM_SetDutyCycleBuzzer(25);
            break;
        default:
            break;
    }
}

void PWM_SetDutyCycleLed(uint8_t adc_value){
    uint8_t inverted_value = 255 - adc_value*255/739;
    OCR0B = inverted_value;
    usart_tx_float(inverted_value, 3, 0);
    txString(",");
}

// ADC functions
void ADC_init(){
    //configure the ADC reference voltage
    ADMUX |= (1 << REFS0);
    ADMUX &= ~(1 << REFS1);

    ADCSRA = (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);      
    ADCSRA |= (1<<ADSC);
  
    ADCSRA |= ( 1 << ADEN); //INITIALIZE
}

uint16_t ADC_Read(uint8_t channel){
  ADMUX &= 0xF0;
  ADMUX |= channel;

  ADCSRA |= (1 << ADSC); //initialize ADC conversion

  while (ADCSRA & (1 << ADSC)); //wait until the conversion is completed
  uint16_t result = ADC;
  return result;
}

// usart funtions
void USART_Init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

void USART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void txString(const char *pStr) {
    while (*pStr != '\0') {
        USART_Transmit(*pStr);
        pStr++;
    }
}

void usart_tx_float(float x, char num_digits_int, char num_digits_decimal)
{
  char num_elements = num_digits_int + num_digits_decimal + 1 + 1; // one decimal point and one null terminator
  char buffer[num_elements];

  dtostrf(x, num_elements - 1, num_digits_decimal, buffer);
  buffer[num_elements - 1] = '\0';
  txString(buffer);
}

void send_teleplot(double distance) {
    char buffer[7];
    dtostrf(distance, 7, 3, buffer);
    txString(buffer);
    txString(",");
}