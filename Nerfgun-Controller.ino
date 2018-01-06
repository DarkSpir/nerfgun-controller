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
#define PINB1     1  // also PCINT1
#define PINB2     2  // also PCINT2

Bounce btnLaser = Bounce();
Bounce btnLight = Bounce();

uint8_t effLight = 0, effLaser = 0, effStrobo = 0;        // Stores state of LED (0=Off, 1=On, 2=Strobe), Laser (0=Off, 1=On) and Strobe interval
unsigned long stroboMillis, sleepMillis;                  // Stores start time for strobe effect and sleep counter
bool sleepBool = false;                                   // This indicates if the sleep counter has started

uint8_t geninterval() {           // Returns next strobe interval (random number between 50 and 190ms in 10th steps)
  return 50 + (10 * random(15));
}

void setup() {
  cbi(ADCSRA,ADEN);             // Disable DAC
  pinMode(PINLASER, OUTPUT);
  pinMode(PINLIGHT, OUTPUT);
  btnLaser.attach(PINB1, INPUT_PULLUP);
  btnLight.attach(PINB2, INPUT_PULLUP);
  btnLaser.interval(5);
  btnLight.interval(5);
  digitalWrite(PINLASER, LOW);
  digitalWrite(PINLIGHT, LOW);
}

void loop() {
  btnLaser.update();
  btnLight.update();
  
  if(btnLaser.fell()) {
    digitalWrite(PINLASER, !digitalRead(PINLASER));
    effLaser ^= 1;
    sleepBool = false;
  }
  
  if(btnLight.fell()) {
    switch(effLight) {
      case 0:
        digitalWrite(PINLIGHT, HIGH);
        effLight = 1;
        break;

      case 1:
        effLight = 2;
        effStrobo = geninterval();
        stroboMillis = millis();
        break;
        
      case 2:
        digitalWrite(PINLIGHT, LOW);
        effLight = 0;
        break;
    }
  }
  
  if(effLight == 2) {
    if(millis() - stroboMillis > effStrobo) {
      digitalWrite(PINLIGHT, !digitalRead(PINLIGHT));
      effStrobo = geninterval();
      stroboMillis = millis();
    }
  }
  
//  if((effLight == 0) && (effLaser == 0) && (!sleepBool)) {
//    sleepBool = true;
//    sleepMillis = millis();
//  }

  if(sleepBool) {
    if(millis() - sleepMillis > 200) {
      sleepBool = false;                    // We decided to go to sleep, no need for the counter
      cli();                                // Disable interrupt handling so we can configure everything uninterrupted
      GIMSK = 0b00100000;                   // activate On Pin Change Interrupt PCINT
      PCMSK = 0b00000110;                   // activate PCINT for Pin PB1 and PB2 (PCINT1 and PCINT2)
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
    if((effLight == 0) && (effLaser == 0)) {
      sleepBool = true;
      sleepMillis = millis();
    }
  }
}

ISR(PCINT0_vect) {}   // Interrupt Handling routine. Since we just need PCINT to wake up, this can be empty
