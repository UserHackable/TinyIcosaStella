
#include <EEPROM.h>
//#include <avr/interrupt.h>
//#include <avr/power.h>
#include <avr/sleep.h>
//#include <avr/io.h>

// Utility macros
#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

#define LEDCOUNT 20
const static byte pairs [] = {0,1,2,3,4,0,2,4,1,3,0};

void pulse(int led, int pause) {
  led = led % LEDCOUNT;
  if (led < 0) led = LEDCOUNT - led;
  byte pair = led / 2;
  byte odd = led % 2;
  byte pin1 = pairs[pair];
  byte pin2 = pairs[pair + 1];
  digitalWrite(pin1,!odd);
  digitalWrite(pin2,odd);
  pinMode(pin1,OUTPUT);
  pinMode(pin2,OUTPUT);
  delayMicroseconds(pause);
  pinMode(pin1,INPUT);
  pinMode(pin2,INPUT);
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,LOW);
}

void randomBlink(int on, int off) {
  byte pin = random(LEDCOUNT);
  pulse(pin,on);
  delay(off);
}

void rotate(int d, int on, int off) {
  static int pin = 0;
  pulse(pin,on);
  pin += d;
  if (pin < 0) pin = LEDCOUNT - 1;
  if (pin >= LEDCOUNT) pin = 0;
  delay(off);
}

void fade(int pin) {
  for(int on = 1; on < 1000; ++on) {
    int off = 2000 - on;
    pulse(pin,on);
    delayMicroseconds(off);
  }
  for(int on = 1000; on > 0; --on) {
    int off = 2000 - on;
    pulse(pin,on);
    delayMicroseconds(off);
  }
}

void randomFade() {
  byte pin = random(LEDCOUNT);
  fade(pin);
}

void multiFade(int pos) {
  pos += 100;
  for(int p = pos - 50; p < pos + 50; p += 5) {
    int o = (60 - abs(pos - p));
    pulse(p / 5, o * o / 4);
  }
}

void rotateFade() {
  static int c = 50;
  multiFade(c);
  c = (c + 1) % 100;
}

void breathe() {
  static int d   = 5;
  static bool up = true;
  for(int pin = 0; pin < 20; ++pin) pulse(pin, d);
  delayMicroseconds((250 * 20) - (d * 20) + 1);
  if (up) {
    if ( d < 250 ) ++d;
    else up = false;
  } else {
    if ( d > 1 ) --d;
    else up = true;
  }
}

void heartBeat() {
  static int p = 5;
  static bool up = true;
  byte d = (p < 50) ? 5 : (p < 100) ? (p - 50) : (150 - p);
  for(byte pin = 0; pin < 20; ++pin) pulse(pin, d);
  delayMicroseconds((50 * 20) - (d * 20) + 1);
  if (up) {
    if ( p < 150 ) ++p;
    else up = false;
  } else {
    if ( p >   1 ) --p;
    else up = true;
  }  
}

void sleepNow() {
  for(byte pin = 0; pin < 5; ++pin) {
    pinMode(pin,INPUT);
    digitalWrite(pin,LOW);
  }
  adc_disable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Choose our preferred sleep mode:
  sleep_enable();                      // Set sleep enable (SE) bit:
  sleep_mode();                        // Put the device to sleep:
  sleep_disable();                     // Upon waking up, sketch continues from this point.
}

void twinkle()     { randomBlink(3000, 20); }
void spinCounter() { rotate(  1, 3000, 20); }
void spinClock()   { rotate( -1, 3000, 20); }
void allOn()       { rotate(  1,  500,  0); }

typedef void(*ModePtr)();
ModePtr ModePtrs[] = {
  sleepNow, twinkle, spinClock, spinCounter,
  allOn, rotateFade, breathe, heartBeat,
};

const static byte mode_count = sizeof(ModePtrs) / sizeof(ModePtrs[0]);
static byte mode = 0;

void setup() {
  adc_disable();
  // we are using the reset pin to change modes
  if (bit_is_set(MCUSR, EXTRF)) { // reset button
    mode = EEPROM.read(1) % mode_count; // retrieve mode from EEPROM 
    EEPROM.write(1, (mode + 1) % mode_count); // store the next mode into the EEPROM
  } else {
    mode = 0;
    EEPROM.write(1, 1); // store the next mode into the EEPROM
  }
}

void loop() { ModePtrs[mode](); }



