/*
   Demo program for the FHICTBot.
   This bot is equipped with two Parallax Feedback 360° High-Speed Servo's.
*/
#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

const int LdrPin = A0;
const int InterruptPin = 2;
const int HeadlightPin = 8;
const int LedstripPin = 9;
const int LdrSupply = 12;
const int LdrThresholdLower = 500;
const int LdrThresholdHigher = 600;
const int MotionCheckIntervalMillis = 5000 / 2; // Because we use the internal clock of 8 MHz we have to divide the desired interval by two.
volatile bool motion = false;

void setAllPinsToInput()
{
  // Set all pins to input with pullup resistor for minimum power consumption.
  // Pins that are used will receive a subsequent pinMode setting.
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
}

void enableAdc()
{
  bitSet(ADCSRA, ADEN);
}
void disableAdc()
{
  bitClear(ADCSRA, ADEN);
}

void disableAc()
{
  bitSet(ACSR, ACD);
}

void wakeup()
{
  motion = true;
}

void setup()
{
  Serial.begin(2 * 9600); // We need 9600 baud, but because we use the internal clock of 8 MHz we have to set it to 2x9600.
  Serial.println("Hello Skateboard!");
  setAllPinsToInput(); // This does not seem to save additional power ( > 1 μA).
  disableAc();         // This does not seem to save additional power ( > 1 μA).
  wdt_disable();       // This does not seem to save additional power ( > 1 μA).
  pinMode(InterruptPin, INPUT_PULLUP);
  pinMode(HeadlightPin, OUTPUT);
  pinMode(LdrSupply, OUTPUT);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode to power down.
  attachInterrupt(0, wakeup, FALLING); // trigger interrupt when INT0 goes from HIGH to LOW.
  sleep_enable();
}

void loop()
{
  static unsigned long previousMillis = 0;
  digitalWrite(LdrSupply, HIGH);
  int ldrVal = analogRead(LdrPin);
  Serial.println("LDR value: " + String(ldrVal));
  digitalWrite(LdrSupply, LOW);    // Output back to low to save power.
  if (ldrVal > LdrThresholdHigher) // If it is dark with hysteresis.
  {
    digitalWrite(HeadlightPin, HIGH);
  }
  else if (ldrVal < LdrThresholdLower) // If it is light with hysteresis.
  {
    digitalWrite(HeadlightPin, LOW);
  }

  //Serial.println("millis: " + String(millis()) + " motion: " + String(motion));
  if (motion)
  {
    previousMillis = millis();
    motion = false;
  }
  else if (millis() - previousMillis >= MotionCheckIntervalMillis)
  {
    digitalWrite(HeadlightPin, LOW);
    disableAdc();        // This will save appr. 260 μA.
    power_all_disable(); // This does not seem to save additional power.
    sleep_bod_disable(); // BODS (Brown Out Detection Sleep) is active only 3 clock cycles, so sleep_cpu() must follow immediately. This will save appr. 20 μA.
    sleep_cpu();         // Power down! Power drops from appr. 8 mA to appr. 0.1 μA.
    power_all_enable();  // We will need this for the delay function (timer 0).
    enableAdc();         // We need this for the LDR measurement
  }
}
