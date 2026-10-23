# AVR Proximity Alarm 🚨

Project for **Introduction to Embedded Systems (UTS)**: a proximity alarm, similar to a parking sensor, built on an **Arduino Uno (ATmega328P)**. All peripherals are driven directly through the microcontroller's registers, without `digitalWrite`, `analogWrite`, `tone()` or other Arduino functions.

An ultrasonic sensor measures the distance to an object, and an LED and a buzzer blink and beep together. The closer the object, the faster they blink and beep.

## Features

- **Distance measurement** with an HC-SR04 ultrasonic sensor.
- **Distance-based alert rate.** The LED and buzzer toggle every `distance / 100` seconds (1 cm = 10 ms), up to a maximum of 3 s.
- **Ambient-light LED brightness.** A photoresistor is read through the ADC. The darker the room, the brighter the LED, set by PWM duty cycle.
- **3 volume levels.** A push button on external interrupt `INT1` cycles the buzzer PWM duty cycle: low (5), medium (10), high (25).
- **ON/OFF button.** Starts or pauses the whole system, with software debouncing.
- **Serial output** (USART, 9600 baud) with the LED PWM value, the ON/OFF state and the volume level, for monitoring with a serial plotter such as Teleplot.

## How it works

### Distance

A 10 µs pulse is sent on `TRIG`. The program then counts, in steps of 10 µs, how long `ECHO` stays high:

```
distance [cm] = (count × 0.343) / 2
```

Here 0.343 is the distance in cm that sound travels in 10 µs. The result is divided by 2 because the pulse travels to the object and back.

### PWM (Timer0)

Timer0 runs in **Fast PWM, non-inverting mode, prescaler 64** (~976 Hz) and drives two outputs:

| Output | Register | Use |
|--------|----------|-----|
| OC0A (PD6) | `OCR0A` | Buzzer, duty cycle = volume level |
| OC0B (PD5) | `OCR0B` | LED, duty cycle = `255 − ADC × 255 / 739` |

### Main loop

1. Check the ON/OFF button (with debouncing).
2. Read the photoresistor with the ADC.
3. If the system is ON: measure the distance, toggle the LED and the buzzer, and wait the time that matches the distance.
4. If the system is OFF: turn off the LED and the buzzer.

The volume button is handled asynchronously in the `INT1` interrupt service routine (falling edge).

## Hardware

- Arduino Uno (ATmega328P @ 16 MHz)
- HC-SR04 ultrasonic sensor
- Passive buzzer
- LED + current-limiting resistor
- Photoresistor (LDR) + resistor (voltage divider)
- 2 push buttons
- Breadboard and jumper wires

### Wiring

| Component | AVR pin | Arduino pin |
|-----------|---------|-------------|
| HC-SR04 `TRIG` | PB1 | D9 |
| HC-SR04 `ECHO` | PB0 | D8 |
| LED | PD5 (OC0B) | D5 |
| Buzzer | PD6 (OC0A) | D6 |
| Volume button | PD3 (INT1) | D3 |
| ON/OFF button | PD7 | D7 |
| Photoresistor | PC1 (ADC1) | A1 |

Both buttons use the internal pull-up resistors, so each one connects between its pin and **GND** (pressed = `0`).

## Serial output

USART is set to **9600 baud, 8 data bits, 2 stop bits**. The program sends:

- The LED PWM value followed by `,` each time the LED turns on.
- `1` / `0` when the system is switched ON / OFF.
- `1`, `2` or `3` when the volume level changes.

## Build and upload

This project uses [PlatformIO](https://platformio.org/) with the `uno` environment.

From VS Code with the PlatformIO extension, click **Build** (✓) and then **Upload** (→).

Or from the terminal:

```bash
pio run              # build
pio run -t upload    # build and upload to the Arduino
```

## Project structure

```
├── src/main.cpp      # Proximity alarm (main program)
├── platformio.ini    # PlatformIO configuration
├── include/          # Headers (empty)
├── lib/              # Libraries (empty)
└── test/             # Tests (empty)
```
