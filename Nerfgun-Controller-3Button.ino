#include <Bounce2.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define PINLASER  3  // Output goes to NPN base that controls the laser module
#define PINLIGHT  4  // Output goes to NPN base that controls the LED
#define PINB0     0  // also PCINT0 or INT0
#define PINB1     1  // also PCINT1
#define PINB2     2  // also PCINT2

Bounce btnLaser = Bounce();
Bounce btnLight = Bounce();
Bounce btnFlash = Bounce();

bool effLight = false, effLaser = false;                  // Stores state of LED and Laser
uint8_t effStrobo = 0;                                    // Strobe interval
unsigned long stroboMillis, sleepMillis;                  // Stores start time for strobe effect and sleep counter
bool sleepBool = false;                                   // This indicates if the sleep counter has started

uint8_t geninterval() {           // Returns next strobe interval (random number between 50 and 190ms in 10th steps)
  return 50 + (10 * random(15));
}

void setup() {
  cbi(ADCSRA,ADEN);             // Disable DAC
  pinMode(PINLASER, OUTPUT);
  pinMode(PINLIGHT, OUTPUT);
  btnFlash.attach(PINB0, INPUT_PULLUP);
  btnLaser.attach(PINB1, INPUT_PULLUP);
  btnLight.attach(PINB2, INPUT_PULLUP);
  btnFlash.interval(5);
  btnLaser.interval(5);
  btnLight.interval(5);
  digitalWrite(PINLASER, LOW);
  digitalWrite(PINLIGHT, LOW);
}

void loop() {
  btnLaser.update();
  btnLight.update();
  btnFlash.update();
  
  if(btnLaser.fell()) {
    digitalWrite(PINLASER, !digitalRead(PINLASER));
    effLaser = !effLaser;
    sleepBool = false;
  }

  if(btnLight.fell()) {
    digitalWrite(PINLIGHT, !digitalRead(PINLIGHT));
    effLight = !effLight;
    sleepBool = false;
  }

  if(!btnFlash.read()) {
    if(effLight) {
      digitalWrite(PINLIGHT, LOW);
    } else {
      sleepBool = false;
      if(millis() - stroboMillis > effStrobo) {
        digitalWrite(PINLIGHT, !digitalRead(PINLIGHT));
        effStrobo = geninterval();
        stroboMillis = millis();
      }
    }
  }

  if(btnFlash.rose()) {
    if(effLight) {
      digitalWrite(PINLIGHT, HIGH);
    } else {
      digitalWrite(PINLIGHT, LOW);
      effStrobo = 0;
      stroboMillis = 0;
    }
  }
    
  if(sleepBool) {
    if(millis() - sleepMillis > 200) {
      sleepBool = false;                    // We decided to go to sleep, no need for the counter
      cli();                                // Disable interrupt handling so we can configure everything uninterrupted
      GIMSK = 0b00100000;                   // activate On Pin Change Interrupt PCINT
      PCMSK = 0b00000111;                   // activate PCINT for Pin PB0, PB1 and PB2 (PCINT0, PCINT1 and PCINT2)
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // We power down completely
      sleep_enable();                       // Prepare Sleep mode
      sleep_bod_disable();                  // Disable Brown-out Detector (sleep will be a little bit deeper now)
      sei();                                // Enable interrupt handling, otherwise we won't wake up anymore
      sleep_cpu();                          // And we're gone
      cli();                                // If we arrive at this step, something woke us up, disable interrupt handling so we can clean up uninterrupted
      sleep_disable();                      // Disable Sleep mode
      GIMSK = 0;                            // Since we don't use interrupt handling except for Sleep wakeup, disable PCINT
      PCMSK = 0;                            // We disabled PCINT so we don't need to know what kind of PCINT we should listen to
      sei();                                // Cleanup done, enable interrupt handling again.
    }
  } else {
    if((!effLight) && (!effLaser)) {
      sleepBool = true;
      sleepMillis = millis();
    }
  }
}

ISR(PCINT0_vect) {}   // Interrupt Handling routine. Since we just need PCINT to wake up, this can be empty
